#ifndef __OCR_LITE_GO_API_H__
#define __OCR_LITE_GO_API_H__

#ifdef __cplusplus
extern "C" {
#endif

// Initialize OCR models
// Returns 1 on success, 0 on failure
int InitOcr(const char* detPath, const char* clsPath, const char* recPath, const char* keysPath);

// Set number of threads for inference
void SetNumThread(int numThread);

// Perform OCR detection on image file
// Returns result string (caller must free with FreeString)
char* DetectImage(const char* imagePath, int padding, int maxSideLen,
                  float boxScoreThresh, float boxThresh, float unClipRatio,
                  int doAngle, int mostAngle);

// Free string allocated by Go
void FreeString(char* str);

// Clean up OCR resources
void CloseOcr();

#ifdef __cplusplus
}
#endif

#endif // __OCR_LITE_GO_API_H__