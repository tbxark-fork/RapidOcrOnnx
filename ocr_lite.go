package main

import (
	"fmt"
	"log"
	"path/filepath"
	"strings"
	
	"gocv.io/x/gocv"
)

// OcrLite implements the main OCR functionality using Go/GoCV/onnx-gomlx
type OcrLite struct {
	dbNet    *DbNet
	angleNet *AngleNet
	crnnNet  *CrnnNet
}

// NewOcrLite creates a new OcrLite instance
func NewOcrLite() *OcrLite {
	return &OcrLite{
		dbNet:    NewDbNet(),
		angleNet: NewAngleNet(),
		crnnNet:  NewCrnnNet(),
	}
}

// SetNumThread sets the number of threads for all networks
func (o *OcrLite) SetNumThread(numThread int) {
	o.dbNet.SetNumThread(numThread)
	o.angleNet.SetNumThread(numThread)
	o.crnnNet.SetNumThread(numThread)
}

// InitModels initializes all ONNX models
func (o *OcrLite) InitModels(detPath, clsPath, recPath, keysPath string) error {
	// Initialize detection model
	if err := o.dbNet.InitModel(detPath); err != nil {
		return fmt.Errorf("failed to initialize DbNet: %v", err)
	}
	
	// Initialize angle detection model
	if err := o.angleNet.InitModel(clsPath); err != nil {
		return fmt.Errorf("failed to initialize AngleNet: %v", err)
	}
	
	// Initialize recognition model
	if err := o.crnnNet.InitModel(recPath, keysPath); err != nil {
		return fmt.Errorf("failed to initialize CrnnNet: %v", err)
	}
	
	log.Println("All OCR models initialized successfully")
	return nil
}

// Detect performs OCR on an image file
func (o *OcrLite) Detect(imagePath string, padding, maxSideLen int,
	boxScoreThresh, boxThresh, unClipRatio float32, doAngle, mostAngle bool) (OcrResult, error) {
	
	// Load image
	src := gocv.IMRead(imagePath, gocv.IMReadColor)
	if src.Empty() {
		return OcrResult{}, fmt.Errorf("failed to load image: %s", imagePath)
	}
	defer src.Close()
	
	return o.DetectMat(src, filepath.Base(imagePath), padding, maxSideLen,
		boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle)
}

// DetectMat performs OCR on a gocv.Mat image
func (o *OcrLite) DetectMat(src gocv.Mat, imgName string, padding, maxSideLen int,
	boxScoreThresh, boxThresh, unClipRatio float32, doAngle, mostAngle bool) (OcrResult, error) {
	
	startTime := GetCurrentTime()
	
	// Add padding if specified
	paddingSrc := MakePadding(src, padding)
	defer func() {
		if padding > 0 {
			paddingSrc.Close()
		}
	}()
	
	// Calculate scale parameters
	scale := GetScaleParam(paddingSrc, maxSideLen)
	
	// Perform detection and recognition
	result := o.detectInternal(paddingSrc, imgName, scale,
		boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle)
	
	endTime := GetCurrentTime()
	result.DetectTime = endTime - startTime
	
	log.Printf("Total OCR time: %.2f ms", result.DetectTime)
	return result, nil
}

// detectInternal performs the internal OCR processing
func (o *OcrLite) detectInternal(src gocv.Mat, imgName string, scale ScaleParam,
	boxScoreThresh, boxThresh, unClipRatio float32, doAngle, mostAngle bool) OcrResult {
	
	result := OcrResult{
		TextBlocks: []TextBlock{},
		StrRes:     "",
	}
	
	// Step 1: Text Detection with DbNet
	dbStartTime := GetCurrentTime()
	textBoxes := o.dbNet.GetTextBoxes(src, scale, boxScoreThresh, boxThresh, unClipRatio)
	dbEndTime := GetCurrentTime()
	result.DbNetTime = dbEndTime - dbStartTime
	
	if len(textBoxes) == 0 {
		log.Println("No text boxes detected")
		return result
	}
	
	log.Printf("Detected %d text boxes", len(textBoxes))
	
	// Step 2: Extract text regions
	partImgs := o.getPartImages(src, textBoxes)
	defer func() {
		for _, img := range partImgs {
			img.Close()
		}
	}()
	
	// Step 3: Angle Detection
	angles := o.angleNet.GetAngles(partImgs, doAngle, mostAngle)
	
	// Step 4: Apply angle corrections
	correctedImgs := o.applyAngleCorrections(partImgs, angles)
	defer func() {
		for _, img := range correctedImgs {
			img.Close()
		}
	}()
	
	// Step 5: Text Recognition
	textLines := o.crnnNet.GetTextLines(correctedImgs)
	
	// Step 6: Combine results
	result.TextBlocks = o.combineResults(textBoxes, angles, textLines)
	result.StrRes = o.formatResultString(result.TextBlocks)
	
	return result
}

// getPartImages extracts image parts based on detected text boxes
func (o *OcrLite) getPartImages(src gocv.Mat, textBoxes []TextBox) []gocv.Mat {
	partImgs := make([]gocv.Mat, len(textBoxes))
	
	for i, textBox := range textBoxes {
		// For simplicity, use bounding rectangle
		// In full implementation, would use rotated rectangle cropping
		rect := gocv.BoundingRect(textBox.BoxPoint)
		
		// Ensure rectangle is within image bounds
		rect = o.clipRect(rect, src.Cols(), src.Rows())
		
		// Extract region
		roi := src.Region(rect)
		partImgs[i] = roi.Clone()
		roi.Close()
	}
	
	return partImgs
}

// clipRect clips rectangle to image bounds
func (o *OcrLite) clipRect(rect gocv.Rect, width, height int) gocv.Rect {
	if rect.X < 0 {
		rect.Width += rect.X
		rect.X = 0
	}
	if rect.Y < 0 {
		rect.Height += rect.Y
		rect.Y = 0
	}
	if rect.X+rect.Width > width {
		rect.Width = width - rect.X
	}
	if rect.Y+rect.Height > height {
		rect.Height = height - rect.Y
	}
	
	if rect.Width <= 0 {
		rect.Width = 1
	}
	if rect.Height <= 0 {
		rect.Height = 1
	}
	
	return rect
}

// applyAngleCorrections rotates images based on detected angles
func (o *OcrLite) applyAngleCorrections(partImgs []gocv.Mat, angles []Angle) []gocv.Mat {
	correctedImgs := make([]gocv.Mat, len(partImgs))
	
	for i, img := range partImgs {
		if i < len(angles) && angles[i].Index == 1 {
			// Rotate 180 degrees
			correctedImgs[i] = o.rotateImage180(img)
		} else {
			// No rotation needed
			correctedImgs[i] = img.Clone()
		}
	}
	
	return correctedImgs
}

// rotateImage180 rotates image 180 degrees
func (o *OcrLite) rotateImage180(src gocv.Mat) gocv.Mat {
	dst := gocv.NewMat()
	gocv.Rotate(src, &dst, gocv.Rotate180Clockwise)
	return dst
}

// combineResults combines detection, angle, and recognition results
func (o *OcrLite) combineResults(textBoxes []TextBox, angles []Angle, textLines []TextLine) []TextBlock {
	blocks := make([]TextBlock, len(textBoxes))
	
	for i, textBox := range textBoxes {
		block := TextBlock{
			BoxPoint:   textBox.BoxPoint,
			BoxScore:   textBox.Score,
			AngleIndex: -1,
			AngleScore: 0.0,
			AngleTime:  0.0,
			Text:       "",
			CharScores: []float32{},
			CrnnTime:   0.0,
			BlockTime:  0.0,
		}
		
		// Add angle information
		if i < len(angles) {
			block.AngleIndex = angles[i].Index
			block.AngleScore = angles[i].Score
			block.AngleTime = angles[i].Time
		}
		
		// Add text recognition information
		if i < len(textLines) {
			block.Text = textLines[i].Text
			block.CharScores = textLines[i].CharScores
			block.CrnnTime = textLines[i].Time
		}
		
		blocks[i] = block
	}
	
	return blocks
}

// formatResultString formats the OCR result as a string
func (o *OcrLite) formatResultString(textBlocks []TextBlock) string {
	var result strings.Builder
	
	for i, block := range textBlocks {
		if i > 0 {
			result.WriteString("\n")
		}
		result.WriteString(block.Text)
	}
	
	return result.String()
}

// Close releases all resources
func (o *OcrLite) Close() error {
	if o.dbNet != nil {
		o.dbNet.Close()
	}
	if o.angleNet != nil {
		o.angleNet.Close()
	}
	if o.crnnNet != nil {
		o.crnnNet.Close()
	}
	return nil
}