package main

import (
	"fmt"
	"os"
)

// Example usage of the Go OCR implementation
func exampleUsage() {
	fmt.Println("=== RapidOcrOnnx Go Implementation Example ===")
	
	// Create OCR instance (using minimal version for demo)
	ocr := NewOcrLiteMinimal()
	defer ocr.Close()
	
	fmt.Println("✅ OCR instance created successfully")
	
	// Model paths that would be used in production
	modelPaths := struct {
		det  string
		cls  string 
		rec  string
		keys string
	}{
		det:  "models/ch_PP-OCRv3_det_infer.onnx",
		cls:  "models/ch_ppocr_mobile_v2.0_cls_infer.onnx", 
		rec:  "models/ch_PP-OCRv3_rec_infer.onnx",
		keys: "models/ppocr_keys_v1.txt",
	}
	
	fmt.Printf("\nModel paths for production use:\n")
	fmt.Printf("  Detection: %s\n", modelPaths.det)
	fmt.Printf("  Angle:     %s\n", modelPaths.cls)
	fmt.Printf("  Recognition: %s\n", modelPaths.rec)
	fmt.Printf("  Keys:      %s\n", modelPaths.keys)
	
	// Demonstrate data structures
	result := OcrResult{
		DbNetTime:  45.2,
		DetectTime: 123.7,
		StrRes:     "Hello World 你好世界",
		TextBlocks: []TextBlock{
			{
				BoxPoint:   []Point{{X: 10, Y: 20}, {X: 200, Y: 20}, {X: 200, Y: 60}, {X: 10, Y: 60}},
				BoxScore:   0.95,
				Text:       "Hello World",
				CharScores: []float32{0.9, 0.95, 0.92, 0.88, 0.94},
				AngleIndex: 0,
			},
			{
				BoxPoint:   []Point{{X: 15, Y: 70}, {X: 180, Y: 70}, {X: 180, Y: 110}, {X: 15, Y: 110}},
				BoxScore:   0.89,
				Text:       "你好世界", 
				CharScores: []float32{0.87, 0.91, 0.86, 0.93},
				AngleIndex: 0,
			},
		},
	}
	
	fmt.Printf("\nDemo OCR Result:\n")
	fmt.Printf("  Detected text: %s\n", result.StrRes)
	fmt.Printf("  Detection time: %.2f ms\n", result.DetectTime) 
	fmt.Printf("  Text blocks: %d\n", len(result.TextBlocks))
	
	for i, block := range result.TextBlocks {
		fmt.Printf("  Block %d: '%s' (score: %.3f, angle: %d)\n", 
			i+1, block.Text, block.BoxScore, block.AngleIndex)
		fmt.Printf("    Box: [(%d,%d), (%d,%d), (%d,%d), (%d,%d)]\n",
			block.BoxPoint[0].X, block.BoxPoint[0].Y,
			block.BoxPoint[1].X, block.BoxPoint[1].Y, 
			block.BoxPoint[2].X, block.BoxPoint[2].Y,
			block.BoxPoint[3].X, block.BoxPoint[3].Y)
	}
	
	fmt.Println("\n✅ Go implementation successfully demonstrates OCR data structures")
}

// Demonstration of C API bindings  
func exampleCAPI() {
	fmt.Println("\n=== C API Bindings Example ===")
	fmt.Println("The Go implementation can be compiled as a C shared library:")
	fmt.Println()
	fmt.Println("Build command:")
	fmt.Println("  go build -buildmode=c-shared -o librapidocr.so .")
	fmt.Println()
	fmt.Println("C usage example:")
	fmt.Println(`  #include "librapidocr.h"
  
  int main() {
      // Initialize models
      InitOcr("det.onnx", "cls.onnx", "rec.onnx", "keys.txt");
      
      // Set threads
      SetNumThread(4);
      
      // Perform OCR
      char* result = DetectImage("image.jpg", 10, 1024, 0.6f, 0.3f, 2.0f, 1, 1);
      printf("Result: %s\n", result);
      
      // Cleanup
      FreeString(result);
      CloseOcr();
      return 0;
  }`)
}

func init() {
	// Check if this is being run as a demo
	if len(os.Args) > 1 && os.Args[1] == "demo" {
		exampleUsage()
		exampleCAPI()
		os.Exit(0)
	}
}