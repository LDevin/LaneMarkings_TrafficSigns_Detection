#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "signdetector.h"
using namespace cv;
using namespace std;

SignDetector::SignDetector()
{

}


/**
*@功能 色彩分割
*@参数 src源图像
*@return matRgb色彩分割的图像
*/
Mat SignDetector::colorSegmentation(Mat src)
{
    int width = src.cols;//图像宽度
    int height = src.rows;//图像高度
    double B = 0.0, G = 0.0, R = 0.0;
    //获取matRgb
    Mat matRgb = Mat::zeros(src.size(), CV_8UC1);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // 获取BGR值
            B = src.at<Vec3b>(y, x)[0];
            G = src.at<Vec3b>(y, x)[1];
            R = src.at<Vec3b>(y, x)[2];

            //红色
            if (R - G > 50 && R - B > 50 )   // && B<50
            {
                matRgb.at<uchar>(y, x) = 255;
                continue;
            }
            //绿色
            if (G - R > 50 && G -B > 30)    //&& R < 100
            {
                matRgb.at<uchar>(y, x) = 255;
                continue;
            }
            //蓝色
            if (B - G > 50 && B - R > 50)   //&& G < 100
            {
                matRgb.at<uchar>(y, x) = 255;
                continue;
            }
//            //黄色
//            if (G - B > 50 && R - B > 50)
//            {
//                matRgb.at<uchar>(y, x) = 255;
//                continue;
//            }
        }
    }
    return matRgb;
}


/**
*@功能 中值滤波降噪
*@参数 src源图像
*@return src中值滤波降噪后的图像
*/
Mat SignDetector::deNoise(Mat src)
{
    //中值滤波降噪
    medianBlur(src, src, 3);
    medianBlur(src, src, 5);
    //imshow("medianBlur", matRgb);
    return src;
}


/**
*@功能 形态学处理
*@参数 src源图像
*@return src形态学处理后的图像
*/
Mat SignDetector::morphologyProcess(Mat src)
{
    //element,element1生成形态学的核
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 1 + 1, 2 * 1 + 1), Point(1, 1));
    Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(2 * 3 + 1, 2 * 3 + 1), Point(3, 3));
    erode(src, src, element);             //腐蚀
    //imshow("erode", src);
    dilate(src, src, element1);           //膨胀
    //imshow("dilate", src);
    fillHole(src, src);      //填充
    //imshow("fillHole", src);

    return src;
}



/**
*@功能 寻找轮廓
*@参数 src源图像
*@参数 contours轮廓点集的最小矩形
* @return boundRect包围
*/
vector<Rect> SignDetector::myfindContours(Mat src, vector<vector<Point>> &contours)
{
    vector<Vec4i> hierarchy;        //分层
    findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//寻找轮廓
    vector<vector<Point>> contours_poly(contours.size());  //近似后的轮廓点集
    vector<Rect> boundRect(contours.size());  //包围点集的最小矩形vector

    //获取包围轮廓的最小矩形
    for (int i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true); //对多边形曲线做适当近似，contours_poly[i]是输出的近似点集
        boundRect[i] = boundingRect(Mat(contours_poly[i])); //计算并返回包围轮廓点集的最小矩形
    }
    return boundRect;
}

/**
*@功能 二值图像的孔洞填充
*@参数 srcBw源图像
*@参数 dstBw目标图像
*/
void SignDetector::fillHole(const Mat srcBw, Mat &dstBw)
{
    Size m_Size = srcBw.size();
    Mat Temp = Mat::zeros(m_Size.height + 2, m_Size.width + 2, srcBw.type());
    srcBw.copyTo(Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)));

    cv::floodFill(Temp, Point(0, 0), Scalar(255));

    Mat cutImg;//裁剪延展的图像
    Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)).copyTo(cutImg);

    dstBw = srcBw | (~cutImg);
}


/**
*@功能 判断rect1与rect2是否有交集
*@参数 rect1检测到的区域1
*@参数 rect2检测到的区域2
*/
bool SignDetector::isInside(Rect rect1, Rect rect2)
{
    Rect t = rect1&rect2;
    if (rect1.area() > rect2.area())
    {
        return false;
    }
    else
    {
        if (t.area() != 0)
            return true;
    }
}
