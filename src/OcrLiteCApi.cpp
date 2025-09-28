#include "OcrLiteCApi.h"
#include "OcrLiteImpl.h"
#ifdef OCR_ENABLE_SERVER
#include "OcrServer.h"
#endif
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

#ifdef OCR_ENABLE_SERVER
// Server related implementations
typedef struct {
    OcrServer* server;
} OCR_SERVER_OBJ;

_QM_OCR_API OCR_SERVER_HANDLE
OcrServerInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads) {
    OCR_SERVER_OBJ *pServerObj = new OCR_SERVER_OBJ;
    if (!pServerObj) return nullptr;
    
    try {
        pServerObj->server = new OcrServer(szDetModel, szClsModel, szRecModel, szKeyPath, nThreads);
        return pServerObj;
    } catch (const std::exception& e) {
        delete pServerObj;
        return nullptr;
    }
}

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
) {
    OCR_SERVER_OBJ *pServerObj = (OCR_SERVER_OBJ*)handle;
    if (!pServerObj || !pServerObj->server) return -1;
    
    ServerConfig serverConfig;
    serverConfig.addr = addr ? addr : "0.0.0.0";
    serverConfig.port = port > 0 ? port : 8080;
    serverConfig.threads = threads > 0 ? threads : 1;
    serverConfig.jsonDepth = jsonDepth >= 0 ? jsonDepth : 1;
    serverConfig.padding = padding > 0 ? padding : 50;
    serverConfig.maxSideLen = maxSideLen > 0 ? maxSideLen : 1024;
    serverConfig.boxScoreThresh = boxScoreThresh > 0 ? boxScoreThresh : 0.6f;
    serverConfig.boxThresh = boxThresh > 0 ? boxThresh : 0.3f;
    serverConfig.unClipRatio = unClipRatio > 0 ? unClipRatio : 2.0f;
    serverConfig.doAngle = doAngle;
    serverConfig.mostAngle = mostAngle;
    
    return pServerObj->server->run(serverConfig);
}

_QM_OCR_API void
OcrServerStop(OCR_SERVER_HANDLE handle) {
    OCR_SERVER_OBJ *pServerObj = (OCR_SERVER_OBJ*)handle;
    if (pServerObj && pServerObj->server) {
        pServerObj->server->stop();
    }
}

_QM_OCR_API void
OcrServerDestroy(OCR_SERVER_HANDLE handle) {
    OCR_SERVER_OBJ *pServerObj = (OCR_SERVER_OBJ*)handle;
    if (pServerObj) {
        if (pServerObj->server) {
            delete pServerObj->server;
        }
        delete pServerObj;
    }
}
#endif // OCR_ENABLE_SERVER

};
