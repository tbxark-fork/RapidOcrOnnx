package main

import (
	"fmt"
	"time"
	
	"gocv.io/x/gocv"
	"github.com/gomlx/onnx-gomlx/onnx"
	"github.com/gomlx/gomlx/backends"
	"github.com/gomlx/gomlx/ml/context"
)

// OcrUtils contains utility functions for OCR processing
type OcrUtils struct{}

// GetCurrentTime returns current time in milliseconds
func GetCurrentTime() float64 {
	return float64(time.Now().UnixNano()) / 1000000.0
}

// SubtractMeanNormalize preprocesses image for neural network input
func SubtractMeanNormalize(src gocv.Mat, meanValues, normValues []float32) []float32 {
	// Convert Mat to float32 array and apply mean subtraction and normalization
	rows := src.Rows()
	cols := src.Cols()
	channels := src.Channels()
	
	inputData := make([]float32, rows*cols*channels)
	
	// Get pixel data from Mat
	data := src.ToBytes()
	
	idx := 0
	for r := 0; r < rows; r++ {
		for c := 0; c < cols; c++ {
			for ch := 0; ch < channels; ch++ {
				pixelIdx := (r*cols+c)*channels + ch
				if pixelIdx < len(data) {
					pixelValue := float32(data[pixelIdx])
					// Apply mean subtraction and normalization
					normalizedValue := (pixelValue - meanValues[ch]) * normValues[ch]
					inputData[idx] = normalizedValue
					idx++
				}
			}
		}
	}
	
	return inputData
}

// GetScaleParam calculates scaling parameters for image preprocessing
func GetScaleParam(src gocv.Mat, targetSize int) ScaleParam {
	srcWidth := src.Cols()
	srcHeight := src.Rows()
	
	var scale float32
	if srcWidth > srcHeight {
		scale = float32(targetSize) / float32(srcWidth)
	} else {
		scale = float32(targetSize) / float32(srcHeight)
	}
	
	dstWidth := int(float32(srcWidth) * scale)
	dstHeight := int(float32(srcHeight) * scale)
	
	return ScaleParam{
		SrcWidth:    srcWidth,
		SrcHeight:   srcHeight,
		DstWidth:    dstWidth,
		DstHeight:   dstHeight,
		RatioWidth:  float32(dstWidth) / float32(srcWidth),
		RatioHeight: float32(dstHeight) / float32(srcHeight),
	}
}

// MakePadding adds padding to image
func MakePadding(src gocv.Mat, padding int) gocv.Mat {
	if padding <= 0 {
		return src
	}
	
	padded := gocv.NewMat()
	white := gocv.NewScalar(255, 255, 255, 0)
	gocv.CopyMakeBorder(src, &padded, padding, padding, padding, padding, gocv.BorderConstant, white)
	
	return padded
}

// ClampFloat32 clamps a float32 value between min and max
func ClampFloat32(x, min, max float32) float32 {
	if x > max {
		return max
	}
	if x < min {
		return min
	}
	return x
}