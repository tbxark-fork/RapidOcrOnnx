#ifdef __cplusplus
#ifndef __OCR_LITE_C_API_H__
#define __OCR_LITE_C_API_H__
#include "stdint.h"
extern "C"
{

#ifdef WIN32
#ifdef __CLIB__
#define _QM_OCR_API __declspec(dllexport)
#else
#define _QM_OCR_API __declspec(dllimport)
#endif
#else
#define _QM_OCR_API
#endif

typedef void *OCR_HANDLE;
typedef char OCR_BOOL;

#ifndef NULL
#define NULL 0
#endif
#define TRUE 1
#define FALSE 0

typedef struct __ocr_param {
    int32_t padding;
    int32_t maxSideLen;
    float boxScoreThresh;
    float boxThresh;
    float unClipRatio;
    int32_t doAngle; // 1 means do
    int32_t mostAngle; // 1 means true
} OCR_PARAM;

typedef struct {
    uint8_t *text;
} TEXT_BLOCK;

typedef struct {
    TEXT_BLOCK *textBlocks;
    uint64_t textBlocksLength;
} OCR_RESULT;

/*
By default, nThreads should be the number of threads
*/
_QM_OCR_API OCR_HANDLE
OcrInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads);

_QM_OCR_API OCR_BOOL
OcrDetect(OCR_HANDLE handle, const char *imgPath, const char *imgName, OCR_PARAM *pParam, OCR_RESULT *ocrResult);

_QM_OCR_API OCR_BOOL
OcrFreeResult(OCR_RESULT *result);

_QM_OCR_API int OcrGetLen(OCR_HANDLE handle);

_QM_OCR_API void OcrDestroy(OCR_HANDLE handle);

};
#endif //__OCR_LITE_C_API_H__
#endif //__cplusplus
