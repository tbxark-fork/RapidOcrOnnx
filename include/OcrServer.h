#ifndef __OCR_SERVER_H__
#define __OCR_SERVER_H__

#include <string>
#include <memory>

struct ServerConfig {
    std::string addr = "0.0.0.0";
    int port = 8080;
    int threads = 1;
    int jsonDepth = 1;
    
    // OCR parameters with default values
    int32_t padding = 50;
    int32_t maxSideLen = 1024;
    float boxScoreThresh = 0.6f;
    float boxThresh = 0.3f;
    float unClipRatio = 2.0f;
    int32_t doAngle = 1;
    int32_t mostAngle = 1;
};

class OcrServer {
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    
public:
    OcrServer(const std::string& detModel, 
              const std::string& clsModel, 
              const std::string& recModel, 
              const std::string& keyPath,
              int nThreads);
    ~OcrServer();
    
    // Start server (blocking call)
    int run(const ServerConfig& config);
    
    // Stop server
    void stop();
};

#endif // __OCR_SERVER_H__
