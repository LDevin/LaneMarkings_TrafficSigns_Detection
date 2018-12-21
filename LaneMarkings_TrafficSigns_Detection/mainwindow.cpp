#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "LaneDetector.h"
using namespace cv;
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //设置标题
    setWindowTitle("车道线和交通标志检测--作者：张朝中");

    //设置自适应
    ui->label_video->setScaledContents(true);
    ui->label_sign1->setScaledContents(true);
    ui->label_sign2->setScaledContents(true);
    ui->label_sign3->setScaledContents(true);
    ui->label_sign4->setScaledContents(true);
    //加载图片
    QPixmap pixmap("model.jpg");
    ui->label_video->setPixmap(pixmap);
    ui->label_sign1->setPixmap(pixmap);
    ui->label_sign2->setPixmap(pixmap);
    ui->label_sign3->setPixmap(pixmap);
    ui->label_sign4->setPixmap(pixmap);

    //默认文本
    ui->textEdit_log->setText("☆欢迎来到车道线和交通标志检测☆");
    //设置只读
    ui->textEdit_log->setReadOnly(true);

    //为窗口,按钮添加图标
    QIcon icon;
    icon.addFile(tr("resource/logo.png"));
    setWindowIcon(icon);
    icon.addFile(tr("resource/open.ico"));
    ui->pushButton_open->setIcon(icon);
    ui->pushButton_open->setIconSize(QSize(36,36));
    icon.addFile(tr("resource/start.ico"));
    ui->pushButton_start->setIcon(icon);
    ui->pushButton_start->setIconSize(QSize(36,36));
    icon.addFile(tr("resource/stop.ico"));
    ui->pushButton_stop->setIcon(icon);
    ui->pushButton_stop->setIconSize(QSize(36,36));
    icon.addFile(tr("resource/end.ico"));
    ui->pushButton_end->setIcon(icon);
    ui->pushButton_end->setIconSize(QSize(36,36));

    ui->checkBox_lane->setIconSize(QSize(48,48));
    ui->checkBox_sign->setIconSize(QSize(48,48));

    //设置checkBox初始状态
    ui->checkBox_lane->setChecked(true);
    ui->checkBox_sign->setChecked(true);

    //初始化打开视频文件路径
    m_fileName = "";
    //初始化打开视频文件标志
    m_openvideo_flag = false;
    //初始化暂停检测标志
    m_stop_flag = false;
    //初始化开始检测标志
    m_start_flag = false;
    //初始化结束检测标志
    m_end_flag = false;
    //初始化车道线检测标志
    m_lane_flag = true;
    //初始化交通标志检测标志
    m_sign_flag = true;

}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
*打开视频文件
*/
void MainWindow::on_pushButton_open_clicked()
{
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    if(m_openvideo_flag)
    {
        ui->textEdit_log->append(current_date + " [Info] 请结束当前视频检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }
    //获取文件路径
    auto fileName = QFileDialog::getOpenFileName(nullptr, "open video",
           QDir::currentPath() + "/Videos", "video files(*.avi;*.mp4;*.wmv);;all files(*.*)");

    current_date_time =QDateTime::currentDateTime();
    current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    if (!fileName.isEmpty())
    {
        m_fileName = fileName.toStdString();
        m_openvideo_flag = 1;

        ui->textEdit_log->append(current_date + " [Info] 打开视频文件成功，可以开始检测了...");
        ui->textEdit_log->moveCursor(QTextCursor::End); //将光标移到最后
    }
    else
    {
        m_openvideo_flag = 0;
        ui->textEdit_log->append(current_date + " [Info] 没有选择视频...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}
/*
*开始检测
*/
void MainWindow::on_pushButton_start_clicked()
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");

    //已经在检测，再按开始  则显示在检测中
    if(m_start_flag && m_stop_flag == false)
    {

        ui->textEdit_log->append(current_date + " [Info] 在检测中...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //如果暂停，则恢复
    if(m_stop_flag)
    {
        m_stop_flag = false;
        ui->textEdit_log->append(current_date + " [Info] 恢复检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //打开文件后 才可以开始检测
    if(m_openvideo_flag)
    {
        //开始检测
        m_start_flag = true;
        if(!runLaneDetection(m_fileName))
        {
            current_date_time =QDateTime::currentDateTime();
            current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
            ui->textEdit_log->append(current_date + " [Info] 检测完毕...");
            if(m_sign_flag)
                ui->textEdit_log->append(current_date + " [Info] 检测到"+QString::number(m_sign_count)+"个交通标志");
            ui->textEdit_log->moveCursor(QTextCursor::End);
            m_end_flag = false;
            m_start_flag = false;
            m_stop_flag = false;
            m_openvideo_flag = false;
        }
        else{
            current_date_time =QDateTime::currentDateTime();
            current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
            ui->textEdit_log->append(current_date + " [Info] 视频文件打开失败...");
            ui->textEdit_log->moveCursor(QTextCursor::End);
        }
    }
    else{
        ui->textEdit_log->append(current_date + " [Info] 请打开视频文件后再开始检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}
/*
*停止检测
*/
void MainWindow::on_pushButton_stop_clicked()
{
    //暂停条件:要已经开始检测
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    if(m_start_flag)
    {
        ui->textEdit_log->append(current_date + " [Info] 已经暂停检测了...");
        m_stop_flag = true;
    }else{
        ui->textEdit_log->append(current_date + " [Info] 还没有开始检测了...");
    }
    ui->textEdit_log->moveCursor(QTextCursor::End);
}
/*
*手动结束检测
*/
void MainWindow::on_pushButton_end_clicked()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    if(m_start_flag == false)
    {
        ui->textEdit_log->append(current_date + " [Info] 还没开始检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }
    m_end_flag = true;
    m_start_flag = false;
    m_stop_flag = false;
    m_openvideo_flag = false;
    ui->textEdit_log->append(current_date + " [Info] 结束检测...");
    ui->textEdit_log->moveCursor(QTextCursor::End);
}


/**
*@功能 功能主要运行车道检测的主要算法输出video，绘制检测到的车道。
*@参数 fileName视频的路径
*/
int MainWindow::runLaneDetection(std::string fileName)
{

    // 输入参数是视频的路径
    cv::VideoCapture cap(fileName);
    if (!cap.isOpened())
        return -1;

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 开始检测...");
    ui->textEdit_log->moveCursor(QTextCursor::End);

    //opencv窗口
    cv::namedWindow("Lane");
    // 显示最终输出图像
    HWND hWnd = (HWND)cvGetWindowHandle("Lane");
    HWND hRawWnd = ::GetParent(hWnd); //获得父窗口句柄
    ShowWindow(hWnd, 0);     //0：的时候表示隐藏子窗口
    ShowWindow(hRawWnd, 0);  //0：的时候表示隐藏父窗口

    LaneDetector lanedetector;  // 创建类对象

    cv::Mat img_denoise;
    cv::Mat img_edges;
    cv::Mat img_mask;
    std::vector<cv::Vec4i> lines;
    std::vector<std::vector<cv::Vec4i> > left_right_lines;
    std::vector<cv::Point> lane;
    std::string turn;
    int flag_plot = -1;
    m_sign_count = 0;
    int last_count = 0;
    bool isZero = false;
    bool first = true;
    m_label_flag[1] = false;  //label是否加载了交通标志图片
    m_label_flag[2] = false;
    m_label_flag[3] = false;
    m_label_flag[4] = false;

    current_date_time =QDateTime::currentDateTime();
    current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 检测中...");
    ui->textEdit_log->moveCursor(QTextCursor::End);

    // 主算法启动。迭代视频的每一帧
    while (1)
    {

        if (cv::waitKey(25) == 30){ break; }
        if (m_stop_flag){
            continue;
        }
        cap >> m_frame;

        //没有数据则视频已读取完毕 或者 按了结束按钮
        if(!m_frame.data || m_end_flag)
        {
            //清除图片
            ui->label_video->clear();
            //恢复待定
            QPixmap pixmap("model.jpg");
            ui->label_video->setPixmap(pixmap);
            cv::destroyAllWindows();
            return 0;
        }
        cv::resize(m_frame, m_frame, cv::Size(640,368), 0, 0);


        /*********下面是：基于RGB的交通标志检测*********/
        if(m_sign_flag)
        {
            //色彩分割
            int width = m_frame.cols;//图像宽度
            int height = m_frame.rows;//图像高度
            double B = 0.0, G = 0.0, R = 0.0;
            Mat matRgb = Mat::zeros(m_frame.size(), CV_8UC1);
            int x, y; //循环
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x++)
                {
                    // 获取BGR值
                    B = m_frame.at<Vec3b>(y, x)[0];
                    G = m_frame.at<Vec3b>(y, x)[1];
                    R = m_frame.at<Vec3b>(y, x)[2];

                    //红色
                    if (R - G > 50 && R - B > 50 )   // && B<50
                    {
                        matRgb.at<uchar>(y, x) = 255;
                        continue;
                    }
//                    //绿色
//                    if (G - R > 50 && G -B > 30)    //&& R < 100
//                    {
//                        matRgb.at<uchar>(y, x) = 255;
//                        continue;
//                    }
                    //蓝色
                    if (B - G > 50 && B - R > 50)   //&& G < 100
                    {
                        matRgb.at<uchar>(y, x) = 255;
                        continue;
                    }
//                    //黄色
//                    if (G - B > 50 && R - B > 50)
//                    {
//                        matRgb.at<uchar>(y, x) = 255;
//                        continue;
//                    }
                }
            }

            // 中值滤波
            medianBlur(matRgb, matRgb, 3);
            medianBlur(matRgb, matRgb, 5);
            //imshow("medianBlur", matRgb);

            Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 1 + 1, 2 * 1 + 1), Point(1, 1));
            Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(2 * 3 + 1, 2 * 3 + 1), Point(3, 3));
            erode(matRgb, matRgb, element);//腐蚀
            //imshow("erode", matRgb);
            dilate(matRgb, matRgb, element1);//膨胀
            //imshow("dilate", matRgb);
            fillHole(matRgb, matRgb);//填充
            //imshow("fillHole", matRgb);

            Mat matRgbCopy;
            matRgb.copyTo(matRgbCopy);

            //找轮廓
            vector<vector<Point>>contours; //轮廓
            vector<Vec4i> hierarchy;//分层
            findContours(matRgb, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));//寻找轮廓
            vector<vector<Point>> contours_poly(contours.size());  //近似后的轮廓点集
            vector<Rect> boundRect(contours.size());  //包围点集的最小矩形vector
            vector<int> index;   //保存找到的交通标志的下标

            //画轮廓
            for (int i = 0; i < contours.size(); i++)
            {
                approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true); //对多边形曲线做适当近似，contours_poly[i]是输出的近似点集
                boundRect[i] = boundingRect(Mat(contours_poly[i])); //计算并返回包围轮廓点集的最小矩形
            }

            int cur_count =0;
            for (int i = 0; i < contours.size(); i++)
            {
                Rect rect = boundRect[i];
                //首先进行一定的限制，筛选出区域

                //若轮廓矩形内部还包含着矩形，则将被包含的小矩形取消
                bool inside = false;
                for (int j = 0; j < contours.size(); j++)
                {
                    Rect t = boundRect[j];
                    if (rect == t)
                        continue;
                    else if (isInside(rect, t))
                    {
                        inside = true;
                        break;
                    }
                }
                if (inside)
                    continue;

                //高宽比限制
                float ratio = (float)rect.width / (float)rect.height;
                //轮廓面积限制
                //float Area = (float)rect.width * (float)rect.height;
                float dConArea = (float)contourArea(contours[i]);
                //float dConLen = (float)arcLength(contours[i], 1);
                if (dConArea < 600)
                    continue;
                if(ratio > 3)
                    continue;
                //画轮廓识别框
                Scalar color = (0, 0, 255);//蓝色线画轮廓
                rectangle(m_frame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
                cur_count++;
                index.push_back(i);
            }

            //检测到了
            if(cur_count)
            {
                //第一次检测到，直接赋值给sign_count
                if(first)
                {
                    for(int i=0; i<index.size();i++)
                    {
//                        Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                        Mat roi = m_frame.clone();
                        cv::cvtColor(roi, roi, CV_BGR2RGB);//颜色空间转换
                        QImage image = QImage((uchar*)(roi.data), roi.cols, roi.rows, QImage::Format_RGB888);
                        selectLabelShow(image);
                    }
                    m_sign_count = cur_count;
                    first =false;
                    isZero = false;
                }
                else{
                    if(isZero == true)       // 如果没有检测到后，再一次检测到
                    {
                        m_sign_count += cur_count;
                        isZero = false;

                        for(int i=0; i<index.size();i++)
                        {
//                            Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                            Mat roi = m_frame.clone();
                            cv::cvtColor(roi, roi, CV_BGR2RGB);//颜色空间转换
                            QImage image = QImage((uchar*)(roi.data), roi.cols, roi.rows, QImage::Format_RGB888);
                            selectLabelShow(image);
                        }
                    }else if(cur_count > last_count)  //由检测1个，持续当前检测变成2时，则增加了1个
                    {
                        m_sign_count += (cur_count-last_count);

                        for(int i=0; i<index.size();i++)
                        {
//                            Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                            Mat roi = m_frame.clone();
                            cv::cvtColor(roi, roi, CV_BGR2RGB);//颜色空间转换
                            QImage image = QImage((uchar*)(roi.data), roi.cols, roi.rows, QImage::Format_RGB888);
                            selectLabelShow(image);
                        }
                    }
                }
                last_count = cur_count;
            }
            else{
                isZero = true;
            }
        }
        /*********上面是：基于RGB的交通标志检测*********/


        /*********下面是：基于Hough和消失点,拟合直线的车道线检测*********/
        if(m_lane_flag)
        {
            // 使用高斯滤波器对图像进行去噪
            img_denoise = lanedetector.deNoise(m_frame);
            // 检测图像中的边缘
            img_edges = lanedetector.edgeDetector(img_denoise);

            // 掩盖图像，以便我们只获得ROI
            img_mask = lanedetector.mask(img_edges);

            // 在裁剪后的图像中获取Hough线
            lines = lanedetector.houghLines(img_mask);

    //        //直接画出不做处理的hough线   虚线段无法连成一条直线     改进 拟合直线
    //        for( size_t i = 0; i < lines.size(); i++ )
    //        {
    //            cv::Vec4i l = lines[i];
    //            cv::line( m_frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0,255,255), 4, CV_AA);
    //        }

            //检测车道线不为空
            if (!lines.empty())
            {

                // 将线分成左右两行
                left_right_lines = lanedetector.lineSeparation(lines, img_edges);

    //            //测试左右线       成功
    //            for(int j=0; j<2; j++)
    //            {
    //                for( size_t i = 0; i < left_right_lines[j].size(); i++ )
    //                {
    //                    cv::Vec4i l = left_right_lines[j][i];
    //                    cv::line( m_frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0,255,255), 4, CV_AA);
    //                }
    //            }

                // 应用回归以仅为通道的每一侧获得一条线
                lane = lanedetector.regression(left_right_lines, m_frame);

                // 通过确定线的消失点来预测转弯
                turn = lanedetector.predictTurn();

                // 绘制车道检测
                flag_plot = lanedetector.plotLane(m_frame, lane, turn);
            }
            else
            {
                flag_plot = -1;
            }
        }
        /*********上面是：基于Hough和消失点,拟合直线的车道线检测*********/


        //将图像显示到label中
        cv::cvtColor(m_frame, m_frame, CV_BGR2RGB);//颜色空间转换
        QImage image = QImage((uchar*)(m_frame.data), m_frame.cols, m_frame.rows, QImage::Format_RGB888);
        ui->label_video->setPixmap(QPixmap::fromImage(image));
        ui->label_video->show();
    }
    return flag_plot;
}

void MainWindow::on_checkBox_lane_clicked()
{
    QString flag;
    if(ui->checkBox_lane->isChecked())
    {
        flag = "打开";
        m_lane_flag = true;
    }
    else
    {
        flag = "关闭";
        m_lane_flag = false;
    }
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 车道线检测..." + flag);
    ui->textEdit_log->moveCursor(QTextCursor::End);
}

void MainWindow::on_checkBox_sign_clicked()
{
    QString flag;
    if(ui->checkBox_sign->isChecked())
    {
        flag = "打开";
        m_sign_flag = true;
    }
    else
    {
        flag = "关闭";
        m_sign_flag = false;
    }
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 交通标志检测..." + flag);
    ui->textEdit_log->moveCursor(QTextCursor::End);
}


void MainWindow::fillHole(const Mat srcBw, Mat &dstBw)
{
    Size m_Size = srcBw.size();
    Mat Temp = Mat::zeros(m_Size.height + 2, m_Size.width + 2, srcBw.type());
    srcBw.copyTo(Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)));

    cv::floodFill(Temp, Point(0, 0), Scalar(255));

    Mat cutImg;
    Temp(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)).copyTo(cutImg);

    dstBw = srcBw | (~cutImg);
}
//判断rect1与rect2是否有交集
bool MainWindow::isInside(Rect rect1, Rect rect2)
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
void MainWindow::selectLabelShow(QImage image)
{
    if(!m_label_flag[1])
    {
        ui->label_sign1->setPixmap(QPixmap::fromImage(image));
        m_label_flag[1] = true;
    }
    else if(!m_label_flag[2])
    {
        ui->label_sign2->setPixmap(QPixmap::fromImage(image));
        m_label_flag[2] = true;
    }
    else if(!m_label_flag[3])
    {
        ui->label_sign3->setPixmap(QPixmap::fromImage(image));
        m_label_flag[3] = true;
    }
    else if(!m_label_flag[4])
    {
        ui->label_sign4->setPixmap(QPixmap::fromImage(image));
        m_label_flag[4] = true;
    }
    //如果全部已加载了图片就重新赋值
    if(m_label_flag[1] && m_label_flag[2] && m_label_flag[3] && m_label_flag[4])
    {
        m_label_flag[1] = false;
        m_label_flag[2] = false;
        m_label_flag[3] = false;
        m_label_flag[4] = false;
    }
}
