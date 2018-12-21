#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "LaneDetector.h"
using namespace cv;
// 高斯模糊
/**
*@功能 将高斯滤波器应用于输入图像以对其进行去噪
*@参数 inputImage是视频的帧
*@return output模糊和去噪的图像
*/
Mat LaneDetector::deNoise(Mat inputImage)
{
    Mat output;

    GaussianBlur(inputImage, output, Size(3, 3), 0, 0);

    return output;
}

// 边缘检测
/**
*@功能 通过过滤图像来检测模糊帧中的所有边缘
*@参数 img_noise是高斯模糊后的帧
*@return output只有边缘以白色表示的二进制图像
*/
Mat LaneDetector::edgeDetector(Mat img_noise)
{
    Mat output;

    // 将图像从RGB转换为灰度化
    cvtColor(img_noise, output, COLOR_RGB2GRAY);

    //大津法
    threshold(output, output, 0, 255, THRESH_BINARY | THRESH_OTSU);

    //canny算子边缘检测
    Canny(output, output, 3, 9, 3);

//    imshow("output", output);
    return output;
}

// MASK边缘图像
/**
*@功能 MASK图像，以便仅检测构成通道一部分的边缘
*@参数 img_edges是前一个函数的边缘图像
*@return output仅表示所需边缘的二进制图像
*/
Mat LaneDetector::mask(Mat img_edges)
{
    Mat output;
    Mat mask = Mat::zeros(img_edges.size(), img_edges.type());

    // 自己调出感兴趣的位置
    Point pts[4] = {
        Point(80, 368),
        Point(280, 210),
        Point(320, 210),
        Point(640, 368)
    };

    // 创建二进制多边形mask
    fillConvexPoly(mask, pts, 4, Scalar(255, 0, 0));

    // 将img_edges和mask相乘得到output
    bitwise_and(img_edges, mask, output);

//    imshow("img_edges",img_edges);
//    imshow("mask",mask);
//    imshow("output",output);

    return output;
}

// 获取HOUGH线
/**
*@功能 获取MASK图像中的所有线段，这些线段将成为通道边界的一部分
*@参数 img_mask是前一个函数的mask二进制图像
*@return line包含图像中所有检测到的线条的矢量
*/
std::vector<Vec4i> LaneDetector::houghLines(Mat img_mask)
{
    std::vector<Vec4i> line;

    // 通过反复试验选择rho和theta
    HoughLinesP(img_mask, line, 1, CV_PI / 180, 20, 20, 30);

    return line;
}

// 分左右线
/**
*@功能 按斜率对所有检测到的Hough线进行排序( 线条分为右侧或左侧)
*@参数 lines是包含所有检测到的Hough线的向量
*@参数 img_edges用于确定图像中心
*@return output包含所有左右线的向量
*/
std::vector<std::vector<Vec4i> > LaneDetector::lineSeparation(std::vector<Vec4i> lines, Mat img_edges)
{
    std::vector<std::vector<Vec4i> > output(2);
    size_t j = 0;
    Point ini;
    Point fini;
    double slope_thresh = 0.3;
    std::vector<double> slopes;
    std::vector<Vec4i> selected_lines;
    std::vector<Vec4i> right_lines, left_lines;

    // 计算所有检测到的线的斜率
    for (auto i : lines)
    {
        ini = Point(i[0], i[1]);
        fini = Point(i[2], i[3]);

        // slope = (y1 - y0)/(x1 - x0)
        double slope = (static_cast<double>(fini.y) - static_cast<double>(ini.y)) / (static_cast<double>(fini.x) - static_cast<double>(ini.x) + 0.00001);

        // 如果斜率太水平，则丢弃该线
        // 否则，保存它们的斜率
        if (std::abs(slope) > slope_thresh)
        {
            slopes.push_back(slope);
            selected_lines.push_back(i);
        }
    }

    // 将线条分成右线和左线
    m_image_center = static_cast<double>((img_edges.cols / 2));
    while (j < selected_lines.size()) {
        ini = Point(selected_lines[j][0], selected_lines[j][1]);
        fini = Point(selected_lines[j][2], selected_lines[j][3]);

        // 将线分类为左侧或右侧的条件
        if (slopes[j] > 0 && fini.x > m_image_center && ini.x > m_image_center)
        {
            right_lines.push_back(selected_lines[j]);
            m_right_flag = true;
        }
        else if (slopes[j] < 0 && fini.x < m_image_center && ini.x < m_image_center)
        {
            left_lines.push_back(selected_lines[j]);
            m_left_flag = true;
        }
        j++;
    }
    //右边线
    output[0] = right_lines;
    //左边线
    output[1] = left_lines;
    return output;
}

// 回归左右线
/**
*@功能 回归采用所有分类线段的初始点和最终点，并使用最小二乘法从它们中拟合新线。
*@参数 left_right_lines是lineSeparation函数的输出
*@参数 inputImage用于获取行数
*@return output包含两个车道边界线的初始点和最终点
*/
std::vector<Point> LaneDetector::regression(std::vector<std::vector<Vec4i> > left_right_lines, Mat inputImage)
{
    std::vector<Point> output(4);
    Point ini;
    Point fini;
    Point ini2;
    Point fini2;
    Vec4d right_line;
    Vec4d left_line;
    std::vector<Point> right_pts;
    std::vector<Point> left_pts;

    // 如果检测到右线，则使用线的所有初始点和最终点拟合线
    if (m_right_flag == true)
    {
        for (auto i : left_right_lines[0])
        {
            ini = Point(i[0], i[1]);
            fini = Point(i[2], i[3]);

            right_pts.push_back(ini);
            right_pts.push_back(fini);
        }

        if (right_pts.size() > 0)
        {
            //  直线拟合函数(组成右边线)
            fitLine(right_pts, right_line, CV_DIST_L2, 0, 0.01, 0.01);
            m_right_m = right_line[1] / right_line[0];
            m_right_b = Point(right_line[2], right_line[3]);
        }
    }


    // 如果检测到左线，则使用线的所有初始点和最终点拟合一条线
    if (m_left_flag == true)
    {
        for (auto j : left_right_lines[1])
        {
            ini2 = Point(j[0], j[1]);
            fini2 = Point(j[2], j[3]);

            left_pts.push_back(ini2);
            left_pts.push_back(fini2);
        }

        if (left_pts.size() > 0)
        {
            //  直线拟合函数(组成左边线)
            fitLine(left_pts, left_line, CV_DIST_L2, 0, 0.01, 0.01);
            m_left_m = left_line[1] / left_line[0];
            m_left_b = Point(left_line[2], left_line[3]);
        }
    }

    // 获得了一个斜率和偏移点，应用线方程来获得线点
    int ini_y = inputImage.rows;
    //与刚兴趣的区域相同
    int fin_y = 210;

    double right_ini_x = ((ini_y - m_right_b.y) / m_right_m) + m_right_b.x;
    double right_fin_x = ((fin_y - m_right_b.y) / m_right_m) + m_right_b.x;

    double left_ini_x = ((ini_y - m_left_b.y) / m_left_m) + m_left_b.x;
    double left_fin_x = ((fin_y - m_left_b.y) / m_left_m) + m_left_b.x;

    output[0] = Point(right_ini_x, ini_y);
    output[1] = Point(right_fin_x, fin_y);
    output[2] = Point(left_ini_x, ini_y);
    output[3] = Point(left_fin_x, fin_y);

    return output;
}

// 转向预测
/**
*@功能 预测车道是左转，右转还是直线
*@功能 通过消失点相对于图像中心的位置来完成
*@return output字符串，表示左转或右转或直线
*/
std::string LaneDetector::predictTurn()
{
    std::string output;
    double vanish_x;
    double thr_vp = 10;

    // 消失点是两条车道边界线相交的点  m1x+b1-y1 = m2x+b2-y2
    vanish_x = static_cast<double>(((m_right_m*m_right_b.x) - (m_left_m*m_left_b.x) - m_right_b.y + m_left_b.y) / (m_right_m - m_left_m));

    // 消失点位置决定了道路转弯的位置
    if (vanish_x < (m_image_center - thr_vp))
        output = "Left Turn";
    else if (vanish_x >(m_image_center + thr_vp))
        output = "Right Turn";
    else if (vanish_x >= (m_image_center - thr_vp) && vanish_x <= (m_image_center + thr_vp))
        output = "Straight";

    return output;
}

// 绘图结果
/**
*@功能 此功能绘制车道的两侧，转弯预测消息和覆盖车道边界内区域的透明多边形
*@参数 inputImage是原始捕获的帧
*@参数 lane是包含两条线信息的向量
*@参数 turn是包含转弯信息的输出字符串
*@return 该函数返回0
*/
void LaneDetector::plotLane(Mat inputImage, std::vector<Point> lane, std::string turn)
{
    std::vector<Point> poly_points;
    Mat output;

    // 创建透明多边形以更好地显示通道
    inputImage.copyTo(output);
    poly_points.push_back(lane[2]);
    poly_points.push_back(lane[0]);
    poly_points.push_back(lane[1]);
    poly_points.push_back(lane[3]);
    fillConvexPoly(output, poly_points, Scalar(0, 0, 255), CV_AA, 0);
    addWeighted(output, 0.3, inputImage, 1.0 - 0.3, 0, inputImage);

    // 绘制车道边界的两条线
    line(inputImage, lane[0], lane[1], Scalar(0, 255, 255), 5, CV_AA);
    line(inputImage, lane[2], lane[3], Scalar(0, 255, 255), 5, CV_AA);

    // 绘制转弯信息
    putText(inputImage, turn, Point(50, 90), FONT_HERSHEY_COMPLEX_SMALL, 3, cvScalar(0, 255, 0), 1, CV_AA);
}
