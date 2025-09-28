#include "OcrServer.h"
#include "cxxopts.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    cxxopts::Options options("ocr-server", "RapidOCR ONNX Server - Text recognition HTTP API server");
    
    options.add_options()
        ("h,help", "Print usage")
        ("det", "Detection model path", cxxopts::value<std::string>()->default_value("models/ch_PP-OCRv4_det_server_infer.onnx"))
        ("cls", "Classification model path", cxxopts::value<std::string>()->default_value("models/ch_ppocr_mobile_v2.0_cls_infer.onnx"))
        ("rec", "Recognition model path", cxxopts::value<std::string>()->default_value("models/ch_PP-OCRv4_rec_server_infer.onnx"))
        ("keys", "Keys file path", cxxopts::value<std::string>()->default_value("keys.txt"))
        ("threads", "Number of threads", cxxopts::value<int>()->default_value("4"))
        ("port", "Server port", cxxopts::value<int>()->default_value("8080"))
        ("addr", "Server address", cxxopts::value<std::string>()->default_value("0.0.0.0"))
        ("json-depth", "JSON response depth", cxxopts::value<int>()->default_value("2"))
        ("padding", "Image padding", cxxopts::value<int>()->default_value("50"))
        ("max-side-len", "Maximum side length", cxxopts::value<int>()->default_value("1024"))
        ("box-score-thresh", "Box score threshold", cxxopts::value<float>()->default_value("0.6"))
        ("box-thresh", "Box threshold", cxxopts::value<float>()->default_value("0.3"))
        ("unclip-ratio", "Unclip ratio", cxxopts::value<float>()->default_value("2.0"))
        ("do-angle", "Enable angle detection", cxxopts::value<bool>()->default_value("true"))
        ("most-angle", "Use most angle", cxxopts::value<bool>()->default_value("true"));
    
    try {
        auto result = options.parse(argc, argv);
        
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }
        
        // Extract model paths
        std::string detModel = result["det"].as<std::string>();
        std::string clsModel = result["cls"].as<std::string>();
        std::string recModel = result["rec"].as<std::string>();
        std::string keyPath = result["keys"].as<std::string>();
        int nThreads = result["threads"].as<int>();
        
        // Create server configuration
        ServerConfig config;
        config.addr = result["addr"].as<std::string>();
        config.port = result["port"].as<int>();
        config.threads = nThreads;
        config.jsonDepth = result["json-depth"].as<int>();
        config.padding = result["padding"].as<int>();
        config.maxSideLen = result["max-side-len"].as<int>();
        config.boxScoreThresh = result["box-score-thresh"].as<float>();
        config.boxThresh = result["box-thresh"].as<float>();
        config.unClipRatio = result["unclip-ratio"].as<float>();
        config.doAngle = result["do-angle"].as<bool>() ? 1 : 0;
        config.mostAngle = result["most-angle"].as<bool>() ? 1 : 0;
        
        // Print configuration
        std::cout << "=== RapidOCR Server Configuration ===" << std::endl;
        std::cout << "Detection Model: " << detModel << std::endl;
        std::cout << "Classification Model: " << clsModel << std::endl;
        std::cout << "Recognition Model: " << recModel << std::endl;
        std::cout << "Keys File: " << keyPath << std::endl;
        std::cout << "Threads: " << nThreads << std::endl;
        std::cout << "Address: " << config.addr << std::endl;
        std::cout << "Port: " << config.port << std::endl;
        std::cout << "JSON Depth: " << config.jsonDepth << std::endl;
        std::cout << "====================================" << std::endl;
        
        // Initialize and start server
        std::cout << "Initializing OCR server..." << std::endl;
        OcrServer server(detModel, clsModel, recModel, keyPath, nThreads);
        
        std::cout << "Starting server on " << config.addr << ":" << config.port << std::endl;
        int result_code = server.run(config);
        
        if (result_code != 0) {
            std::cerr << "Server failed to start with code: " << result_code << std::endl;
            return result_code;
        }
        
    } catch (const cxxopts::exceptions::parsing& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}