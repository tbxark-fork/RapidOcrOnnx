package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strings"
	
	"gocv.io/x/gocv"
	"github.com/gomlx/onnx-gomlx/onnx"
	"github.com/gomlx/gomlx/backends"
	"github.com/gomlx/gomlx/ml/context"
	"github.com/gomlx/gomlx/graph"
)

// CrnnNet implements text recognition using ONNX model
type CrnnNet struct {
	session    *onnx.Session
	backend    backends.Backend
	numThread  int
	meanValues []float32
	normValues []float32
	dstHeight  int
	keys       []string
}

// NewCrnnNet creates a new CrnnNet instance
func NewCrnnNet() *CrnnNet {
	return &CrnnNet{
		meanValues: []float32{127.5, 127.5, 127.5},
		normValues: []float32{1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5},
		dstHeight:  48,
	}
}

// SetNumThread sets the number of threads for inference
func (c *CrnnNet) SetNumThread(numThread int) {
	c.numThread = numThread
}

// InitModel initializes the ONNX model and loads character keys
func (c *CrnnNet) InitModel(modelPath, keysPath string) error {
	var err error
	
	// Initialize backend
	c.backend = backends.New()
	
	// Load ONNX model
	c.session, err = onnx.LoadFile(c.backend, modelPath)
	if err != nil {
		return fmt.Errorf("failed to load CrnnNet ONNX model: %v", err)
	}
	
	// Load character keys
	err = c.loadKeys(keysPath)
	if err != nil {
		return fmt.Errorf("failed to load keys: %v", err)
	}
	
	log.Printf("CrnnNet model loaded successfully from: %s", modelPath)
	log.Printf("Loaded %d character keys", len(c.keys))
	return nil
}

// loadKeys loads character keys from file
func (c *CrnnNet) loadKeys(keysPath string) error {
	file, err := os.Open(keysPath)
	if err != nil {
		return fmt.Errorf("failed to open keys file: %v", err)
	}
	defer file.Close()
	
	c.keys = []string{"#"} // Start with blank character
	
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line != "" {
			c.keys = append(c.keys, line)
		}
	}
	
	if err := scanner.Err(); err != nil {
		return fmt.Errorf("error reading keys file: %v", err)
	}
	
	c.keys = append(c.keys, " ") // Add space character at the end
	
	return nil
}

// GetTextLines performs text recognition on multiple image parts
func (c *CrnnNet) GetTextLines(partImgs []gocv.Mat) []TextLine {
	textLines := make([]TextLine, len(partImgs))
	
	for i, img := range partImgs {
		startTime := GetCurrentTime()
		textLine := c.getTextLine(img)
		endTime := GetCurrentTime()
		textLine.Time = endTime - startTime
		textLines[i] = textLine
	}
	
	return textLines
}

// getTextLine performs text recognition on a single image
func (c *CrnnNet) getTextLine(src gocv.Mat) TextLine {
	// Resize image maintaining aspect ratio
	scale := float32(c.dstHeight) / float32(src.Rows())
	dstWidth := int(float32(src.Cols()) * scale)
	
	srcResize := gocv.NewMat()
	gocv.Resize(src, &srcResize, gocv.Point{X: dstWidth, Y: c.dstHeight}, 0, 0, gocv.InterpolationLinear)
	defer srcResize.Close()
	
	// Preprocess image
	inputData := SubtractMeanNormalize(srcResize, c.meanValues, c.normValues)
	
	// Run inference
	outputData := c.runInference(inputData, srcResize.Rows(), srcResize.Cols(), srcResize.Channels())
	
	// Decode output to text
	return c.decodeOutput(outputData)
}

// runInference runs the ONNX model inference
func (c *CrnnNet) runInference(inputData []float32, height, width, channels int) [][]float32 {
	// Create context for inference
	ctx := context.New()
	
	// Run inference with onnx-gomlx
	outputs := c.session.Run(ctx, map[string]*graph.Node{
		"input": graph.Const(ctx.Graph(), [][][][]float32{
			c.reshapeInput(inputData, channels, height, width),
		}),
	})
	
	// Extract output data (assuming output shape is [seq_len, batch, num_classes])
	var outputData [][]float32
	for _, outputNode := range outputs {
		// TODO: Implement proper tensor to [][]float32 conversion
		// For now, create placeholder data
		seqLen := width // Sequence length typically matches width after CNN
		numClasses := len(c.keys)
		outputData = make([][]float32, seqLen)
		for i := range outputData {
			outputData[i] = make([]float32, numClasses)
		}
		break
	}
	
	return outputData
}

// reshapeInput reshapes 1D input data to 3D tensor [C][H][W]
func (c *CrnnNet) reshapeInput(data []float32, channels, height, width int) [][][]float32 {
	result := make([][][]float32, channels)
	
	for ch := 0; ch < channels; ch++ {
		result[ch] = make([][]float32, height)
		for h := 0; h < height; h++ {
			result[ch][h] = make([]float32, width)
			for w := 0; w < width; w++ {
				idx := ch*height*width + h*width + w
				if idx < len(data) {
					result[ch][h][w] = data[idx]
				}
			}
		}
	}
	
	return result
}

// decodeOutput decodes model output to text using CTC decoding
func (c *CrnnNet) decodeOutput(outputData [][]float32) TextLine {
	if len(outputData) == 0 || len(c.keys) == 0 {
		return TextLine{Text: "", CharScores: []float32{}, Time: 0.0}
	}
	
	// Simple CTC decoding - find best path
	var text strings.Builder
	var charScores []float32
	
	prevIndex := -1
	for _, timeStep := range outputData {
		// Find character with highest probability
		maxIndex := 0
		maxScore := float32(0.0)
		
		for i, score := range timeStep {
			if score > maxScore {
				maxScore = score
				maxIndex = i
			}
		}
		
		// CTC decoding rules:
		// 1. Remove blanks (index 0)
		// 2. Remove repeated characters
		if maxIndex != 0 && maxIndex != prevIndex && maxIndex < len(c.keys) {
			text.WriteString(c.keys[maxIndex])
			charScores = append(charScores, maxScore)
		}
		
		prevIndex = maxIndex
	}
	
	return TextLine{
		Text:       text.String(),
		CharScores: charScores,
		Time:       0.0, // Will be set by caller
	}
}

// argmax finds the index of the maximum value
func argmax(data []float32) int {
	maxIndex := 0
	maxValue := data[0]
	
	for i, value := range data {
		if value > maxValue {
			maxValue = value
			maxIndex = i
		}
	}
	
	return maxIndex
}

// Close releases resources
func (c *CrnnNet) Close() error {
	if c.session != nil {
		c.session.Close()
	}
	if c.backend != nil {
		c.backend.Close()
	}
	return nil
}