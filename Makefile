# Makefile for RapidOcrOnnx Go implementation

.PHONY: all clean build-go build-cpp install-deps test

# Default target
all: build-go

# Build Go shared library
build-go:
	@echo "Building Go implementation..."
	go mod tidy
	go build -buildmode=c-shared -o librapidocr.so .
	@echo "Generated librapidocr.so and librapidocr.h"

# Build original C++ version
build-cpp:
	@echo "Building C++ implementation..."
	./build.sh

# Install system dependencies for GoCV
install-deps-ubuntu:
	@echo "Installing OpenCV for Ubuntu/Debian..."
	sudo apt-get update
	sudo apt-get install -y libopencv-dev pkg-config

install-deps-macos:
	@echo "Installing OpenCV for macOS..."
	brew install opencv pkg-config

# Clean build artifacts
clean:
	rm -f librapidocr.so librapidocr.h
	rm -rf build/
	go clean

# Test Go build without OpenCV (syntax check only)
test-syntax:
	@echo "Testing Go syntax..."
	go mod tidy
	go vet ./...
	go fmt ./...

# Run Go tests (if any)
test:
	@echo "Running Go tests..."
	go test -v ./...

# Show Go module dependencies
deps:
	go list -m all

# Update Go dependencies
update-deps:
	go get -u ./...
	go mod tidy

# Help
help:
	@echo "Available targets:"
	@echo "  all              - Build Go implementation (default)"
	@echo "  build-go         - Build Go shared library"
	@echo "  build-cpp        - Build original C++ version"
	@echo "  install-deps-*   - Install system dependencies"
	@echo "  clean           - Clean build artifacts"
	@echo "  test-syntax     - Test Go syntax without building"
	@echo "  test            - Run Go tests"
	@echo "  deps            - Show dependencies"
	@echo "  update-deps     - Update Go dependencies"
	@echo "  help            - Show this help"