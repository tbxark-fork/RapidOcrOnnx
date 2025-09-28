#include "OcrServer.h"
#include "OcrLiteImpl.h"
#include "crow_all.h"
#include "json.hpp"
#include <fstream>
#include <cstdio>
#include <thread>
#include <cstring>

// Platform-specific includes
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

using json = nlohmann::json;

struct OcrServer::Impl {
    OcrLiteImpl ocrEngine;
    std::unique_ptr<crow::SimpleApp> app;
    std::thread serverThread;
    bool running = false;
    
    Impl(const std::string& detModel, 
         const std::string& clsModel, 
         const std::string& recModel, 
         const std::string& keyPath,
         int nThreads) {
        ocrEngine.setNumThread(nThreads);
        ocrEngine.initModels(detModel, clsModel, recModel, keyPath);
        app = std::make_unique<crow::SimpleApp>();
    }
    
    void setupRoutes(const ServerConfig& config) {
        // Health check endpoint
        CROW_ROUTE((*app), "/healthz").methods("GET"_method)
        ([](const crow::request&) {
            json response;
            response["status"] = "ok";
            return crow::response(200, response.dump());
        });
        
        // OCR endpoint
        CROW_ROUTE((*app), "/ocr").methods("POST"_method)
        ([this, config](const crow::request& req) {
            try {
                // Create temporary file
                std::string tmpName;
                
#ifdef _WIN32
                // Windows implementation
                char tmpPath[MAX_PATH];
                char tmpDir[MAX_PATH];
                if (GetTempPathA(MAX_PATH, tmpDir) == 0) {
                    json errorResponse;
                    errorResponse["error"] = "Failed to get temp directory";
                    return crow::response(500, errorResponse.dump());
                }
                if (GetTempFileNameA(tmpDir, "ocr", 0, tmpPath) == 0) {
                    json errorResponse;
                    errorResponse["error"] = "Failed to create temporary file";
                    return crow::response(500, errorResponse.dump());
                }
                std::ofstream tmpFile(tmpPath, std::ios::binary);
                if (!tmpFile.is_open()) {
                    json errorResponse;
                    errorResponse["error"] = "Failed to open temporary file";
                    return crow::response(500, errorResponse.dump());
                }
                tmpFile.write(req.body.c_str(), req.body.length());
                tmpFile.close();
                tmpName = tmpPath;
#else
                // Unix implementation
                char tmpTemplate[] = "/tmp/ocr_XXXXXX";
                int fd = mkstemp(tmpTemplate);
                if (fd == -1) {
                    json errorResponse;
                    errorResponse["error"] = "Failed to create temporary file";
                    return crow::response(500, errorResponse.dump());
                }
                
                // Write request body to temporary file
                if (write(fd, req.body.c_str(), req.body.length()) < 0) {
                    close(fd);
                    unlink(tmpTemplate);
                    json errorResponse;
                    errorResponse["error"] = "Failed to write temporary file";
                    return crow::response(500, errorResponse.dump());
                }
                close(fd);
                tmpName = tmpTemplate;
#endif
                
                // Parse query parameters with defaults from config
                int32_t padding = config.padding;
                int32_t maxSideLen = config.maxSideLen;
                float boxScoreThresh = config.boxScoreThresh;
                float boxThresh = config.boxThresh;
                float unClipRatio = config.unClipRatio;
                int32_t doAngle = config.doAngle;
                int32_t mostAngle = config.mostAngle;
                int32_t jsonDepth = config.jsonDepth;
                
                // Parse query parameters
                auto urlParams = crow::query_string(req.url_params);
                
                char* paddingParam = urlParams.get("padding");
                if (paddingParam != nullptr && strlen(paddingParam) > 0) {
                    padding = std::stoi(paddingParam);
                }
                char* maxSideLenParam = urlParams.get("maxSideLen");
                if (maxSideLenParam != nullptr && strlen(maxSideLenParam) > 0) {
                    maxSideLen = std::stoi(maxSideLenParam);
                }
                char* boxScoreThreshParam = urlParams.get("boxScoreThresh");
                if (boxScoreThreshParam != nullptr && strlen(boxScoreThreshParam) > 0) {
                    boxScoreThresh = std::stof(boxScoreThreshParam);
                }
                char* boxThreshParam = urlParams.get("boxThresh");
                if (boxThreshParam != nullptr && strlen(boxThreshParam) > 0) {
                    boxThresh = std::stof(boxThreshParam);
                }
                char* unClipRatioParam = urlParams.get("unClipRatio");
                if (unClipRatioParam != nullptr && strlen(unClipRatioParam) > 0) {
                    unClipRatio = std::stof(unClipRatioParam);
                }
                char* doAngleParam = urlParams.get("doAngle");
                if (doAngleParam != nullptr && strlen(doAngleParam) > 0) {
                    doAngle = std::stoi(doAngleParam);
                }
                char* mostAngleParam = urlParams.get("mostAngle");
                if (mostAngleParam != nullptr && strlen(mostAngleParam) > 0) {
                    mostAngle = std::stoi(mostAngleParam);
                }
                char* jsonDepthParam = urlParams.get("jsonDepth");
                if (jsonDepthParam != nullptr && strlen(jsonDepthParam) > 0) {
                    jsonDepth = std::stoi(jsonDepthParam);
                }
                
                // Perform OCR
                OcrResult result = ocrEngine.detect(
                    tmpName.c_str(),
                    "uploaded_image",
                    padding,
                    maxSideLen,
                    boxScoreThresh,
                    boxThresh,
                    unClipRatio,
                    doAngle != 0,
                    mostAngle != 0
                );
                
                // Clean up temporary file
#ifdef _WIN32
                DeleteFileA(tmpName.c_str());
#else
                unlink(tmpName.c_str());
#endif
                
                // Build JSON response
                json response;
                response["dbNetTime"] = result.dbNetTime;
                response["detectTime"] = result.detectTime;
                response["strRes"] = result.strRes;
                
                if (jsonDepth > 0) {
                    json jBlocks = json::array();
                    for (const auto& tb : result.textBlocks) {
                        json jBlock;
                        jBlock["text"] = tb.text;
                        if (jsonDepth > 1) {
                            // box points as [[x,y], ...]
                            json pts = json::array();
                            for (const auto& pt : tb.boxPoint) {
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
                    response["textBlocks"] = jBlocks;
                }
                
                crow::response res(200, response.dump());
                res.add_header("Content-Type", "application/json");
                return res;
                
            } catch (const std::exception& e) {
                json errorResponse;
                errorResponse["error"] = e.what();
                return crow::response(500, errorResponse.dump());
            }
        });
    }
};

OcrServer::OcrServer(const std::string& detModel, 
                     const std::string& clsModel, 
                     const std::string& recModel, 
                     const std::string& keyPath,
                     int nThreads) 
    : pImpl(std::make_unique<Impl>(detModel, clsModel, recModel, keyPath, nThreads)) {
}

OcrServer::~OcrServer() {
    stop();
}

int OcrServer::run(const ServerConfig& config) {
    if (pImpl->running) {
        return -1; // Already running
    }
    
    pImpl->setupRoutes(config);
    pImpl->running = true;
    
    // Configure and start server
    pImpl->app->port(config.port).multithreaded().run();
    
    return 0;
}

void OcrServer::stop() {
    if (pImpl->running) {
        pImpl->app->stop();
        pImpl->running = false;
    }
}
