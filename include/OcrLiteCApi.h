#ifdef __cplusplus
#ifndef __OCR_LITE_C_API_H__
#define __OCR_LITE_C_API_H__
#include "stdint.h"
extern "C"
{

#ifdef WIN32
#  ifdef OCRLIB_EXPORTS
#    define _QM_OCR_API __declspec(dllexport)
#  else
#    define _QM_OCR_API __declspec(dllimport)
#  endif
#else
#define _QM_OCR_API
#endif

typedef void *OCR_HANDLE;
typedef char OCR_BOOL;
typedef char *OCR_STRING;

#ifndef NULL
#define NULL 0
#endif
#define TRUE 1
#define FALSE 0

#define VERSION "1.2.3"

/*
By default, nThreads should be the number of threads
*/
_QM_OCR_API OCR_HANDLE
OcrInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads);

_QM_OCR_API OCR_STRING
OcrDetect(
    OCR_HANDLE handle,
    const char *imgPath,
    const char *imgName,
    int32_t padding,
    int32_t maxSideLen,
    float boxScoreThresh,
    float boxThresh,
    float unClipRatio,
    int32_t doAngle,
    int32_t mostAngle,
	int32_t jsonDepth
);

_QM_OCR_API void OcrDestroy(OCR_HANDLE handle);

_QM_OCR_API void FreeString(OCR_STRING str);

#ifdef OCR_ENABLE_SERVER
// Server related APIs
typedef void *OCR_SERVER_HANDLE;

_QM_OCR_API OCR_SERVER_HANDLE
OcrServerInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads);

_QM_OCR_API int
OcrServerRun(
    OCR_SERVER_HANDLE handle,
    const char* addr,
    int port,
    int threads,
    int jsonDepth,
    int32_t padding,
    int32_t maxSideLen,
    float boxScoreThresh,
    float boxThresh,
    float unClipRatio,
    int32_t doAngle,
    int32_t mostAngle
);

_QM_OCR_API void
OcrServerStop(OCR_SERVER_HANDLE handle);

_QM_OCR_API void
OcrServerDestroy(OCR_SERVER_HANDLE handle);
#endif // OCR_ENABLE_SERVER

};
#endif //__OCR_LITE_C_API_H__
#endif //__cplusplus
