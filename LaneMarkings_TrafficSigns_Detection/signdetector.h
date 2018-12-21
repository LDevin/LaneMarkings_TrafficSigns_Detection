#ifndef SIGNDETECTOR_H
#define SIGNDETECTOR_H


class SignDetector
{
public:
    SignDetector();
    cv::Mat colorSegmentation(cv::Mat src);             //色彩分割
    cv::Mat deNoise(cv::Mat src);                       //中值滤波降噪
    cv::Mat morphologyProcess(cv::Mat src);             //形态学处理
    std::vector<cv::Rect> myfindContours(cv::Mat src,std::vector<std::vector<cv::Point>> &contours);  //找轮廓
    void fillHole(const cv::Mat srcBw, cv::Mat &dstBw); //二值图像的孔洞填充
    bool isInside(cv::Rect rect1, cv::Rect rect2);      //判断rect1与rect2是否有交集
};

#endif // SIGNDETECTOR_H
