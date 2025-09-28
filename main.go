package main

import "C"
import (
	"fmt"
	"log"
	"unsafe"
)

// Global OCR instance
var globalOcr *OcrLite

//export InitOcr
func InitOcr(detPath, clsPath, recPath, keysPath *C.char) C.int {
	// Convert C strings to Go strings
	detPathGo := C.GoString(detPath)
	clsPathGo := C.GoString(clsPath)
	recPathGo := C.GoString(recPath)
	keysPathGo := C.GoString(keysPath)
	
	// Initialize OCR
	globalOcr = NewOcrLite()
	err := globalOcr.InitModels(detPathGo, clsPathGo, recPathGo, keysPathGo)
	if err != nil {
		log.Printf("Failed to initialize OCR models: %v", err)
		return 0
	}
	
	log.Println("OCR models initialized successfully via C API")
	return 1
}

//export SetNumThread
func SetNumThread(numThread C.int) {
	if globalOcr != nil {
		globalOcr.SetNumThread(int(numThread))
	}
}

//export DetectImage
func DetectImage(imagePath *C.char, padding, maxSideLen C.int,
	boxScoreThresh, boxThresh, unClipRatio C.float,
	doAngle, mostAngle C.int) *C.char {
	
	if globalOcr == nil {
		return C.CString("ERROR: OCR not initialized")
	}
	
	// Convert parameters
	imagePathGo := C.GoString(imagePath)
	
	// Perform OCR detection
	result, err := globalOcr.Detect(
		imagePathGo,
		int(padding), int(maxSideLen),
		float32(boxScoreThresh), float32(boxThresh), float32(unClipRatio),
		int(doAngle) != 0, int(mostAngle) != 0,
	)
	
	if err != nil {
		return C.CString(fmt.Sprintf("ERROR: %v", err))
	}
	
	// Return result as C string
	return C.CString(result.StrRes)
}

//export FreeString
func FreeString(str *C.char) {
	C.free(unsafe.Pointer(str))
}

//export CloseOcr
func CloseOcr() {
	if globalOcr != nil {
		globalOcr.Close()
		globalOcr = nil
	}
	log.Println("OCR resources released")
}

// Go main function for testing
func main() {
	log.Println("RapidOcrOnnx Go implementation initialized")
	
	// Example usage (for testing)
	// This would normally be called through C API
	/*
	ocr := NewOcrLite()
	defer ocr.Close()
	
	err := ocr.InitModels("det.onnx", "cls.onnx", "rec.onnx", "keys.txt")
	if err != nil {
		log.Fatalf("Failed to initialize models: %v", err)
	}
	
	result, err := ocr.Detect("test.jpg", 10, 1024, 0.6, 0.3, 2.0, true, true)
	if err != nil {
		log.Fatalf("Failed to detect: %v", err)
	}
	
	log.Printf("OCR Result: %s", result.StrRes)
	*/
}