#ifndef __OCR_LITE_IMPL_H__
#define __OCR_LITE_IMPL_H__

#include "opencv2/core.hpp"
#include "OcrStruct.h"
#include "DbNet.h"
#include "AngleNet.h"
#include "CrnnNet.h"

class OcrLiteImpl {
public:
    OcrLiteImpl();

    ~OcrLiteImpl();

    void setNumThread(int numOfThread);



    bool initModels(const std::string &detPath, const std::string &clsPath,
                    const std::string &recPath, const std::string &keysPath);

    OcrResult detect(const char *path, const char *imgName,
                     int padding, int maxSideLen,
                     float boxScoreThresh, float boxThresh, float unClipRatio, bool doAngle, bool mostAngle);

private:
    DbNet dbNet;
    AngleNet angleNet;
    CrnnNet crnnNet;
    std::vector<cv::Mat> getPartImages(cv::Mat &src, std::vector<TextBox> &textBoxes);

    OcrResult detect(const char *path, const char *imgName,
                     cv::Mat &src, cv::Rect &originRect, ScaleParam &scale,
                     float boxScoreThresh = 0.6f, float boxThresh = 0.3f,
                     float unClipRatio = 2.0f, bool doAngle = true, bool mostAngle = true);
};

#endif //__OCR_LITE_IMPL_H__
