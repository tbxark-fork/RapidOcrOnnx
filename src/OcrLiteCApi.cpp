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
    int32_t mostAngle,
	int32_t jsonDepth
) {

    OCR_OBJ *pOcrObj = (OCR_OBJ *)handle;
    if (!pOcrObj) return nullptr;

    if (padding == 0) padding = 50;
    if (maxSideLen == 0) maxSideLen = 1024;
    if (boxScoreThresh == 0) boxScoreThresh = 0.6f;
    if (boxThresh == 0) boxThresh = 0.3f;
    if (unClipRatio == 0) unClipRatio = 2.0f;
    if (doAngle == 0) doAngle = 1;
    if (mostAngle == 0) mostAngle = 1;

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
    if (jsonDepth > 0) {
		json jBlocks = json::array();
        for (const auto &tb : result.textBlocks) {
            json jBlock;
            jBlock["text"] = tb.text;
            if (jsonDepth > 1) {
                // box points as [[x,y], ...]
                json pts = json::array();
                for (const auto &pt : tb.boxPoint) {
                    pts.push_back({pt.x, pt.y});
                }
                jBlock["boxPoint"] = pts;
                if (jsonDepth > 2) {
                    jBlock["boxScore"] = tb.boxScore;
                    jBlock["angleIndex"] = tb.angleIndex;
                    jBlock["angleScore"] = tb.angleScore;
                    jBlock["angleTime"] = tb.angleTime;
                    jBlock["charScores"] = tb.charScores;
                    jBlock["crnnTime"] = tb.crnnTime;
                    jBlock["blockTime"] = tb.blockTime;
                }
            }
            jBlocks.push_back(jBlock);
        }
        j["textBlocks"] = jBlocks;
	}
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
