#ifdef __CLIB__

#include "OcrLiteCApi.h"
#include "OcrLiteImpl.h"
#include "json.hpp"

using json = nlohmann::json;

extern "C" {

typedef struct {
    OcrLiteImpl OcrObj;
} OCR_OBJ;

_QM_OCR_API OCR_HANDLE
OcrInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads) {
    OCR_OBJ *pOcrObj = new OCR_OBJ;
    if (!pOcrObj) return nullptr;
    pOcrObj->OcrObj.setNumThread(nThreads);
    pOcrObj->OcrObj.initModels(szDetModel, szClsModel, szRecModel, szKeyPath);
    return pOcrObj;
}

// Return a newly allocated JSON string; caller must FreeString
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
    int32_t mostAngle) {

    OCR_OBJ *pOcrObj = (OCR_OBJ *)handle;
    if (!pOcrObj) return nullptr;

    OcrResult result = pOcrObj->OcrObj.detect(
        imgPath,
        imgName,
        padding,
        maxSideLen,
        boxScoreThresh,
        boxThresh,
        unClipRatio,
        doAngle != 0,
        mostAngle != 0);

    // Build JSON
    json j;
    j["dbNetTime"] = result.dbNetTime;
    j["detectTime"] = result.detectTime;
    j["strRes"] = result.strRes;

    json jBlocks = json::array();
    for (const auto &tb : result.textBlocks) {
        json jBlock;
        // box points as [[x,y], ...]
        json pts = json::array();
        for (const auto &pt : tb.boxPoint) {
            pts.push_back({pt.x, pt.y});
        }
        jBlock["boxPoint"] = pts;
        jBlock["boxScore"] = tb.boxScore;
        jBlock["angleIndex"] = tb.angleIndex;
        jBlock["angleScore"] = tb.angleScore;
        jBlock["angleTime"] = tb.angleTime;
        jBlock["text"] = tb.text;
        jBlock["charScores"] = tb.charScores;
        jBlock["crnnTime"] = tb.crnnTime;
        jBlock["blockTime"] = tb.blockTime;
        jBlocks.push_back(jBlock);
    }
    j["textBlocks"] = jBlocks;

    std::string s = j.dump();
    // Allocate C string for return; caller frees with FreeString
    char *ret = (char *)malloc(s.size() + 1);
    if (!ret) return nullptr;
    memcpy(ret, s.c_str(), s.size() + 1);
    return ret;
}

_QM_OCR_API void OcrDestroy(OCR_HANDLE handle) {
    OCR_OBJ *pOcrObj = (OCR_OBJ *)handle;
    if (pOcrObj) delete pOcrObj;
}

_QM_OCR_API void FreeString(OCR_STRING str) {
    if (str) free(str);
}

};
#endif
