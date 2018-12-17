#ifndef LANEDETECTOR_H
#define LANEDETECTOR_H
class LaneDetector
{
private:
    double img_size;            // 图像大小
    double img_center;          // 图像中心位置
    bool left_flag = false;     // 左边线标志
    bool right_flag = false;    // 右边线标志
    cv::Point right_b;          // 车道边界的两个线方程的成员
    double right_m;             // y = m*x + b
    cv::Point left_b;
    double left_m;

public:
    cv::Mat deNoise(cv::Mat inputImage);        // Apply Gaussian blurring to the input Image
    cv::Mat edgeDetector(cv::Mat img_noise);    // Filter the image to obtain only edges
    cv::Mat mask(cv::Mat img_edges);            // Mask the edges image to only care about ROI
    std::vector<cv::Vec4i> houghLines(cv::Mat img_mask);  // Detect Hough lines in masked edges image
    std::vector<std::vector<cv::Vec4i> > lineSeparation(std::vector<cv::Vec4i> lines, cv::Mat img_edges);  // Sprt detected lines by their slope into right and left lines
    std::vector<cv::Point> regression(std::vector<std::vector<cv::Vec4i> > left_right_lines, cv::Mat inputImage);  // Get only one line for each side of the lane
    std::string predictTurn();  // Determine if the lane is turning or not by calculating the position of the vanishing point
    int plotLane(cv::Mat inputImage, std::vector<cv::Point> lane, std::string turn);  // Plot the resultant lane and turn prediction in the frame.
};
#endif // LANEDETECTOR_H
