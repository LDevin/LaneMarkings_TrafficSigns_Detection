#ifndef LANEDETECTOR_H
#define LANEDETECTOR_H
class LaneDetector
{
private:
    double m_image_size;            // 图像大小
    double m_image_center;          // 图像中心位置
    bool m_left_flag = false;     // 左边线标志
    bool m_right_flag = false;    // 右边线标志
    cv::Point m_right_b;          // 车道边界的两个线方程的成员
    double m_right_m;             // y = m*x + b
    cv::Point m_left_b;
    double m_left_m;

public:
    cv::Mat deNoise(cv::Mat inputImage);        // 将高斯滤波器应用于输入图像以对其进行去噪
    cv::Mat edgeDetector(cv::Mat img_noise);    // 通过过滤图像来检测模糊帧中的所有边缘
    cv::Mat mask(cv::Mat img_edges);            // MASK图像，以便仅检测构成通道一部分的边缘
    std::vector<cv::Vec4i> houghLines(cv::Mat img_mask);  // 获取MASK图像中的所有线段，这些线段将成为通道边界的一部分
    std::vector<std::vector<cv::Vec4i> > lineSeparation(std::vector<cv::Vec4i> lines, cv::Mat img_edges);  // 按斜率对所有检测到的Hough线进行排序( 线条分为右侧或左侧)
    std::vector<cv::Point> regression(std::vector<std::vector<cv::Vec4i> > left_right_lines, cv::Mat inputImage);  // 回归采用所有分类线段的初始点和最终点，并使用最小二乘法从它们中拟合新线。
    std::string predictTurn();  // 预测车道是左转，右转还是直线
    void plotLane(cv::Mat inputImage, std::vector<cv::Point> lane, std::string turn);  // 此功能绘制车道的两侧，转弯预测消息和覆盖车道边界内区域的透明多边形
};
#endif // LANEDETECTOR_H
