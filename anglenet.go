package main

import (
	"fmt"
	"log"
	
	"gocv.io/x/gocv"
	"github.com/gomlx/onnx-gomlx/onnx"
	"github.com/gomlx/gomlx/backends"
	"github.com/gomlx/gomlx/ml/context"
	"github.com/gomlx/gomlx/graph"
)

// AngleNet implements text angle detection using ONNX model
type AngleNet struct {
	session    *onnx.Session
	backend    backends.Backend
	numThread  int
	meanValues []float32
	normValues []float32
	dstWidth   int
	dstHeight  int
}

// NewAngleNet creates a new AngleNet instance
func NewAngleNet() *AngleNet {
	return &AngleNet{
		meanValues: []float32{127.5, 127.5, 127.5},
		normValues: []float32{1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5},
		dstWidth:   192,
		dstHeight:  48,
	}
}

// SetNumThread sets the number of threads for inference
func (a *AngleNet) SetNumThread(numThread int) {
	a.numThread = numThread
}

// InitModel initializes the ONNX model
func (a *AngleNet) InitModel(modelPath string) error {
	var err error
	
	// Initialize backend
	a.backend = backends.New()
	
	// Load ONNX model
	a.session, err = onnx.LoadFile(a.backend, modelPath)
	if err != nil {
		return fmt.Errorf("failed to load AngleNet ONNX model: %v", err)
	}
	
	log.Printf("AngleNet model loaded successfully from: %s", modelPath)
	return nil
}

// GetAngles detects angles for multiple image parts
func (a *AngleNet) GetAngles(partImgs []gocv.Mat, doAngle, mostAngle bool) []Angle {
	size := len(partImgs)
	angles := make([]Angle, size)
	
	if doAngle {
		for i, img := range partImgs {
			startTime := GetCurrentTime()
			
			// Resize to target size
			angleImg := gocv.NewMat()
			gocv.Resize(img, &angleImg, gocv.Point{X: a.dstWidth, Y: a.dstHeight}, 0, 0, gocv.InterpolationLinear)
			
			angle := a.getAngle(angleImg)
			endTime := GetCurrentTime()
			angle.Time = endTime - startTime
			
			angles[i] = angle
			angleImg.Close()
		}
	} else {
		// No angle detection - set all to default values
		for i := 0; i < size; i++ {
			angles[i] = Angle{Index: -1, Score: 0.0, Time: 0.0}
		}
	}
	
	// Most possible angle processing
	if doAngle && mostAngle {
		a.processMostAngle(angles)
	}
	
	return angles
}

// getAngle detects angle for a single image
func (a *AngleNet) getAngle(src gocv.Mat) Angle {
	// Preprocess image
	inputData := SubtractMeanNormalize(src, a.meanValues, a.normValues)
	
	// Run inference
	outputData := a.runInference(inputData, src.Rows(), src.Cols(), src.Channels())
	
	// Convert output to angle
	return a.scoreToAngle(outputData)
}

// runInference runs the ONNX model inference
func (a *AngleNet) runInference(inputData []float32, height, width, channels int) []float32 {
	// Create context for inference
	ctx := context.New()
	
	// Run inference with onnx-gomlx
	outputs := a.session.Run(ctx, map[string]*graph.Node{
		"input": graph.Const(ctx.Graph(), [][][][]float32{
			a.reshapeInput(inputData, channels, height, width),
		}),
	})
	
	// Extract output data (assuming single output)
	var outputData []float32
	for _, outputNode := range outputs {
		// TODO: Implement proper tensor to []float32 conversion
		outputData = make([]float32, 2) // AngleNet typically outputs 2 classes (0°, 180°)
		break
	}
	
	return outputData
}

// reshapeInput reshapes 1D input data to 3D tensor [C][H][W]
func (a *AngleNet) reshapeInput(data []float32, channels, height, width int) [][][]float32 {
	result := make([][][]float32, channels)
	
	for c := 0; c < channels; c++ {
		result[c] = make([][]float32, height)
		for h := 0; h < height; h++ {
			result[c][h] = make([]float32, width)
			for w := 0; w < width; w++ {
				idx := c*height*width + h*width + w
				if idx < len(data) {
					result[c][h][w] = data[idx]
				}
			}
		}
	}
	
	return result
}

// scoreToAngle converts model output scores to angle result
func (a *AngleNet) scoreToAngle(outputData []float32) Angle {
	maxIndex := 0
	maxScore := float32(0.0)
	
	for i, score := range outputData {
		if score > maxScore {
			maxScore = score
			maxIndex = i
		}
	}
	
	return Angle{
		Index: maxIndex,
		Score: maxScore,
		Time:  0.0, // Time will be set by caller
	}
}

// processMostAngle processes angles to find the most common angle
func (a *AngleNet) processMostAngle(angles []Angle) {
	if len(angles) == 0 {
		return
	}
	
	// Count angle indices
	angleCount := make(map[int]int)
	for _, angle := range angles {
		if angle.Index >= 0 {
			angleCount[angle.Index]++
		}
	}
	
	// Find most common angle
	mostCommonIndex := -1
	maxCount := 0
	for index, count := range angleCount {
		if count > maxCount {
			maxCount = count
			mostCommonIndex = index
		}
	}
	
	// Apply most common angle to all (if found)
	if mostCommonIndex >= 0 && maxCount > len(angles)/2 {
		for i := range angles {
			if angles[i].Index >= 0 {
				angles[i].Index = mostCommonIndex
			}
		}
		log.Printf("Applied most common angle: %d (count: %d/%d)", mostCommonIndex, maxCount, len(angles))
	}
}

// Close releases resources
func (a *AngleNet) Close() error {
	if a.session != nil {
		a.session.Close()
	}
	if a.backend != nil {
		a.backend.Close()
	}
	return nil
}