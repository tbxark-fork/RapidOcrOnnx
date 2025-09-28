package main

import "gocv.io/x/gocv"

// ScaleParam represents image scaling parameters
type ScaleParam struct {
	SrcWidth    int
	SrcHeight   int
	DstWidth    int
	DstHeight   int
	RatioWidth  float32
	RatioHeight float32
}

// TextBox represents a detected text box
type TextBox struct {
	BoxPoint []gocv.Point
	Score    float32
}

// Angle represents text angle detection result
type Angle struct {
	Index int
	Score float32
	Time  float64
}

// TextLine represents recognized text line
type TextLine struct {
	Text       string
	CharScores []float32
	Time       float64
}

// TextBlock represents a complete text block with detection and recognition
type TextBlock struct {
	BoxPoint    []gocv.Point
	BoxScore    float32
	AngleIndex  int
	AngleScore  float32
	AngleTime   float64
	Text        string
	CharScores  []float32
	CrnnTime    float64
	BlockTime   float64
}

// OcrResult represents the complete OCR result
type OcrResult struct {
	DbNetTime   float64
	TextBlocks  []TextBlock
	DetectTime  float64
	StrRes      string
}