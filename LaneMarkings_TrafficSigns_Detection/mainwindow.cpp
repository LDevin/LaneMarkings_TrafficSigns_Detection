#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "LaneDetector.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //设置标题
    setWindowTitle("车道线和交通标志检测--作者：张朝中");
    //加载图片
    QPixmap pixmap("model.jpg");
    ui->label_video->setPixmap(pixmap);
    ui->label_sign1->setPixmap(pixmap);
    ui->label_sign2->setPixmap(pixmap);
    ui->label_sign3->setPixmap(pixmap);
    ui->label_sign4->setPixmap(pixmap);
    //设置自适应
    ui->label_video->setScaledContents(true);
    ui->label_sign1->setScaledContents(true);
    ui->label_sign2->setScaledContents(true);
    ui->label_sign3->setScaledContents(true);
    ui->label_sign4->setScaledContents(true);

    //默认文本
    ui->textEdit_log->setText("☆欢迎来到车道线和交通标志检测☆");
    //设置只读
    ui->textEdit_log->setReadOnly(true);

    //初始化打开视频文件标志
    m_openvideo_flag = 0;
    //初始化打开视频文件路径
    m_fileName = "";
    //初始化暂停检测标志
    m_stop_flag = 0;
    //初始化开始检测标志
    m_start_flag = 0;
    //初始化结束检测标志
    m_end_flag = 0;
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
    //获取文件路径
    auto fileName = QFileDialog::getOpenFileName(nullptr, "open video",
           QDir::currentPath() + "/Videos", "video files(*.avi;*.mp4;*.wmv);;all files(*.*)");

    if (!fileName.isEmpty())
    {
        m_fileName = fileName.toStdString();
        m_openvideo_flag = 1;

        //获取当前时间
        QDateTime current_date_time =QDateTime::currentDateTime();
        m_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
        ui->textEdit_log->append(m_current_date + " [Info] 打开视频文件成功，可以开始检测了...");
        ui->textEdit_log->moveCursor(QTextCursor::End); //将光标移到最后
    }
    else
    {
        m_openvideo_flag = 0;
        //获取当前时间
        QDateTime current_date_time =QDateTime::currentDateTime();
        m_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
        ui->textEdit_log->append(m_current_date + " [Info] 没有选择视频...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}
/*
*开始检测
*/
void MainWindow::on_pushButton_start_clicked()
{
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    m_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");

    //已经在检测，再按开始  则显示在检测中
    if(m_start_flag && m_stop_flag == 0)
    {
        ui->textEdit_log->append(m_current_date + " [Info] 在检测中...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //如果暂停，则恢复
    if(m_stop_flag)
    {
        m_stop_flag = 0;
        ui->textEdit_log->append(m_current_date + " [Info] 恢复检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //打开文件后 才可以开始检测
    if(m_openvideo_flag)
    {
        //开始检测
        m_start_flag = 1;
        if(!runLaneDetection(m_fileName))
        {
            ui->textEdit_log->append(m_current_date + " [Info] 检测完毕...");
            ui->textEdit_log->moveCursor(QTextCursor::End);
            m_start_flag = 0;
        }
        else{
            ui->textEdit_log->append(m_current_date + " [Info] 视频文件打开失败...");
            ui->textEdit_log->moveCursor(QTextCursor::End);
        }
    }
    else{
        ui->textEdit_log->append(m_current_date + " [Info] 请打开视频文件后再开始检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}
/*
*停止检测
*/
void MainWindow::on_pushButton_stop_clicked()
{
    //暂停条件:要已经开始检测
    if(m_start_flag)
    {
        //获取当前时间
        QDateTime current_date_time =QDateTime::currentDateTime();
        m_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
        ui->textEdit_log->append(m_current_date + " [Info] 已经暂停检测了...");
        m_stop_flag = 1;
    }else{
        ui->textEdit_log->append(m_current_date + " [Info] 还没有开始检测了...");
    }
    ui->textEdit_log->moveCursor(QTextCursor::End);
}
/*
*结束检测
*/
void MainWindow::on_pushButton_end_clicked()
{
    m_end_flag = 1;
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    m_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(m_current_date + " [Info] 结束检测...");
    ui->textEdit_log->moveCursor(QTextCursor::End);
    m_start_flag = 0;
    m_stop_flag = 0;
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

    ui->textEdit_log->append(m_current_date + " [Info] 开始检测...");
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

    ui->textEdit_log->append(m_current_date + " [Info] 检测中...");
    ui->textEdit_log->moveCursor(QTextCursor::End);

    // 主算法启动。迭代视频的每一帧
    while (1)
    {

        if (cv::waitKey(50) == 30){ break; }
        if (m_stop_flag){
            continue;
        }
        cap >> m_frame;

        //没有数据则视频已读取完毕 或者 按了结束按钮
        if(!m_frame.data || m_end_flag)
        {
            m_end_flag = 0;
            //清除图片
            ui->label_video->clear();
            //恢复待定
            QPixmap pixmap("model.jpg");
            ui->label_video->setPixmap(pixmap);
            cv::destroyAllWindows();
            return 0;
        }
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

            //将图像显示到label中
            QImage image = QImage((uchar*)(m_frame.data), m_frame.cols, m_frame.rows, QImage::Format_RGB888);
            ui->label_video->setPixmap(QPixmap::fromImage(image));
            ui->label_video->show();
            //暂停一段时间
            cv::waitKey(25);
        }
        else
        {
            flag_plot = -1;
        }
    }
    return flag_plot;
}