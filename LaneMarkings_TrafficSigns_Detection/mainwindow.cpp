#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "lanedetector.h"
#include "signdetector.h"
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
    m_video_filename = "";
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

    //初始化图片列表
    m_imagelist = new QListWidget;
    //定义QListWidget对象
    m_imagelist->resize(480,340);
    //设置QListWidget的显示模式
    m_imagelist->setViewMode(QListView::IconMode);
    //设置QListWidget中单元项的图片大小
    m_imagelist->setIconSize(QSize(100,100));
    //设置QListWidget中单元项的间距
    m_imagelist->setSpacing(10);
    //设置自动适应布局调整（Adjust适应，Fixed不适应），默认不适应
    m_imagelist->setResizeMode(QListWidget::Adjust);
    //设置不能移动
    m_imagelist->setMovement(QListWidget::Static);
}

MainWindow::~MainWindow()
{
    delete ui;
}


/**
*@功能 打开视频文件-点击事件
*
*/
void MainWindow::on_pushButton_open_clicked()
{
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");

    //文件是否已打开并且是否已经开始，若是则已在进行检测
    if(m_openvideo_flag && m_start_flag)
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
        //文件打开成功
        m_openvideo_flag = true;

        m_video_filename = fileName.toStdString();
        ui->textEdit_log->append(current_date + " [Info] 打开视频文件成功，可以开始检测了...");
        ui->textEdit_log->moveCursor(QTextCursor::End); //将光标移到最后
    }
    else
    {
        //文件打开失败
        m_openvideo_flag = false;

        ui->textEdit_log->append(current_date + " [Info] 没有选择视频...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}


/**
*@功能 开始检测-点击事件
*
*/
void MainWindow::on_pushButton_start_clicked()
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");

    //已经在检测，再按开始 ，则显示在检测中
    if(m_start_flag && m_stop_flag == false)
    {
        ui->textEdit_log->append(current_date + " [Info] 在检测中...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //如果暂停，则恢复
    if(m_stop_flag)
    {
        //暂停标志置为false
        m_stop_flag = false;

        ui->textEdit_log->append(current_date + " [Info] 恢复检测...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
        return;
    }

    //打开文件后 才可以开始检测
    if(m_openvideo_flag)
    {
        //开始检测
        if(!runDetection(m_video_filename))
        {
            current_date_time =QDateTime::currentDateTime();
            current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
            ui->textEdit_log->append(current_date + " [Info] 检测完毕...");
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


/**
*@功能 停止检测--点击事件
*
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


/**
*@功能 手动结束检测--点击事件
*
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
int MainWindow::runDetection(std::string fileName)
{
    //开始检测标志置为true
    m_start_flag = true;

    // fileName是视频的路径
    cv::VideoCapture cap(fileName);
    if (!cap.isOpened())
        return -1;

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 开始检测...");
    ui->textEdit_log->moveCursor(QTextCursor::End);

    //opencv窗口
    cv::namedWindow("Lane");
    // 隐藏opencv窗口
    HWND hWnd = (HWND)cvGetWindowHandle("Lane");
    HWND hRawWnd = ::GetParent(hWnd); //获得父窗口句柄
    ShowWindow(hWnd, 0);     //0：的时候表示隐藏子窗口
    ShowWindow(hRawWnd, 0);  //0：的时候表示隐藏父窗口

    /*****创建LaneDetector类对象*****/
    LaneDetector lanedetector;
    /**车道线的相关初始化**/
    cv::Mat img_denoise;            //高斯模糊降噪用的
    cv::Mat img_edges;              //边缘检测后得到的
    cv::Mat img_mask;               //mask遮罩
    std::vector<cv::Vec4i> lines;   //Hough检测到的线
    std::vector<std::vector<cv::Vec4i> > left_right_lines;  //左右车道线
    std::vector<cv::Point> lane;    //车道线的点
    std::string turn;           //转向信息

    /*****创建SignDetector类对象*****/
    SignDetector signdetector;
    /**交通标志的相关初始化**/
    m_sign_count = 0;           //初始化检测到的交通标志的总个数
    int last_count = 0;         //上一次检测到交通标志的个数
    int cur_count = 0;          //当前检测到交通标志的个
    bool isZero = false;        //是否没有检测到交通标志
    bool first = true;          //是否第一次检测到交通标志
    m_label_flag[1] = false;    //label是否加载了交通标志图片
    m_label_flag[2] = false;
    m_label_flag[3] = false;
    m_label_flag[4] = false;
    m_num = 1;                  //图片号

    current_date_time =QDateTime::currentDateTime();
    current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    ui->textEdit_log->append(current_date + " [Info] 检测中...");
    ui->textEdit_log->moveCursor(QTextCursor::End);

    // 主算法启动。迭代视频的每一帧
    while (1)
    {
        //暂停25ms
        if (cv::waitKey(25) == 30){ break; }

        //暂停检测
        if (m_stop_flag){
            continue;
        }

        //读取一帧
        cap >> m_frame;

        //没有数据则视频已读取完毕 或者 按了结束按钮
        if(!m_frame.data || m_end_flag)
        {
            //清除图片
            ui->label_video->clear();
            ui->label_sign1->clear();
            ui->label_sign2->clear();
            ui->label_sign3->clear();
            ui->label_sign4->clear();
            //恢复待定
            QPixmap pixmap("model.jpg");
            ui->label_video->setPixmap(pixmap);
            ui->label_sign1->setPixmap(pixmap);
            ui->label_sign2->setPixmap(pixmap);
            ui->label_sign3->setPixmap(pixmap);
            ui->label_sign4->setPixmap(pixmap);
            //销毁所opencv窗口
            cv::destroyAllWindows();
            return 0;
        }

        //重设大小,有利于后续的位置(选择遮罩的位置)
        cv::resize(m_frame, m_frame, cv::Size(640,368), 0, 0);


        /*********下面是：基于RGB的交通标志检测*********/
        //交通标志检测标志为true，则开始检测
        if(m_sign_flag)
        {
            //色彩分割
            Mat matRgb = signdetector.colorSegmentation(m_frame);

            // 中值滤波
            matRgb = signdetector.deNoise(matRgb);

            //形态学处理
            matRgb = signdetector.morphologyProcess(matRgb);

            //找轮廓
            vector<vector<Point>> contours;  //轮廓
            vector<Rect> boundRect = signdetector.myfindContours(matRgb,contours);

            //保存找到的交通标志的下标
            vector<int> index;
            //记住这个变量要重新置0，否则会加上之前的个数，出错
            cur_count =0;

            //首先进行一定的限制，筛选出轮廓
            for (int i = 0; i < contours.size(); i++)
            {
                Rect rect = boundRect[i];

                //若轮廓矩形内部还包含着矩形，则将被包含的小矩形取消
                bool inside = false;
                for (int j = 0; j < contours.size(); j++)
                {
                    Rect t = boundRect[j];
                    if (rect == t)
                        continue;
                    else if (signdetector.isInside(rect, t))
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

                //画一个，当前个数加1
                cur_count++;

                //保存相应轮廓的下标
                index.push_back(i);
            }

            //cur_count不为0，则有检测到
            if(cur_count)
            {
                //(1)第一次检测到，直接将cur_count赋值给sign_count
                if(first)
                {
                    m_sign_count = cur_count;
                    //将第一次检测标志置为false
                    first =false;
                    //将没有检测到标志置false
                    isZero = false;

                    //将检测到图像显示到label,并保存
                    for(int i=0; i< cur_count;i++)
                    {
                        //Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                        showSaveImage();
                    }
                }
                else{//非第一次检测

                    if(isZero == true)//(2)如果没有检测到后，再一次检测到,则直接加上cur_count
                    {
                        m_sign_count += cur_count;
                        //将没有检测到标志置false
                        isZero = false;

                        //将检测到图像显示到label,并保存
                        for(int i=0; i< cur_count;i++)
                        {
                            //Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                            showSaveImage();
                        }
                    }else if(cur_count > last_count)  //(3)由检测1个，持续当前检测变成2时，则增加了1个
                    {
                        m_sign_count += (cur_count-last_count);

                        //将检测到图像显示到label,并保存
                        for(int i=0; i<(cur_count-last_count);i++)
                        {
                            //Mat roi = m_frame(Rect(boundRect[index[i]].x,boundRect[index[i]].y,boundRect[index[i]].width,boundRect[index[i]].height));
                            showSaveImage();
                        }
                    }
                }
                //将当前的个数赋值给上一次
                last_count = cur_count;
            }
            else{
                //将没有检测到标志置true
                isZero = true;
            }
        }
        /*********上面是：基于RGB的交通标志检测*********/


        /*********下面是：基于Hough和消失点,拟合直线的车道线检测*********/
        //车道线检测标志为true，则开始检测
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

                // 应用回归以仅为通道的每一侧获得一条线
                lane = lanedetector.regression(left_right_lines, m_frame);

                // 通过确定线的消失点来预测转弯
                turn = lanedetector.predictTurn();

                // 绘制车道检测
                lanedetector.plotLane(m_frame, lane, turn);
            }
        }
        /*********上面是：基于Hough和消失点,拟合直线的车道线检测*********/


        //将图像显示到label中
        cv::cvtColor(m_frame, m_frame, CV_BGR2RGB);//颜色空间转换
        QImage image = QImage((uchar*)(m_frame.data), m_frame.cols, m_frame.rows, QImage::Format_RGB888);
        ui->label_video->setPixmap(QPixmap::fromImage(image));
        ui->label_video->show();
    }
    return 0;
}


/**
*@功能 是否要检测车道线--点击事件
*
*/
void MainWindow::on_checkBox_lane_clicked()
{
    QString flag;
    //被选中,isChecked返回为1，则为打开
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


/**
*@功能 是否要检测交通标志--点击事件
*
*/
void MainWindow::on_checkBox_sign_clicked()
{
    QString flag;
    //被选中,isChecked返回为1，则为打开
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


/**
*@功能 选择对应的label来显示交通标志图片
*@参数 image交通标志图片
*/
void MainWindow::selectLabelShow(QImage image)
{
    //按顺序显示检测到的图片
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

    //如果label全部已加载了图片，将其是否加载图片标志赋值为false
    if(m_label_flag[1] && m_label_flag[2] && m_label_flag[3] && m_label_flag[4])
    {
        m_label_flag[1] = false;
        m_label_flag[2] = false;
        m_label_flag[3] = false;
        m_label_flag[4] = false;
    }
}


/**
*@功能 显示检测到的交通标志图片列表
*
*/
void MainWindow::showImageList()
{
    //文件名列表
    QStringList fileNames;
    for(int i=1; i<= m_sign_count; i++)
    {
        fileNames.append("signimages/" + QString::number(i) + ".jpg");
    }

    //遍历所有文件名
    for(auto tmp : fileNames)
    {
        //定义QListWidgetItem对象
        QListWidgetItem *imageItem = new QListWidgetItem;
        //为单元项设置属性
        imageItem->setIcon(QIcon(tmp));
        imageItem->setText(tmp.remove(0,11));       //remove(0,11)去除signimages/
        //重新设置单元项图片的宽度和高度
        imageItem->setSizeHint(QSize(100,100));
        //将单元项添加到QListWidget中
        m_imagelist->addItem(imageItem);
    }

    //显示QListWidget
    m_imagelist->show();
}


/**
*@功能 显示检测到的交通标志图片列表--点击事件
*
*/
void MainWindow::on_pushButton_showSign_clicked()
{
    //有交通标志数据才显示
    if(m_sign_count)
    {
        //先清空以往检测的
        m_imagelist->clear();
        //显示图片列表
        showImageList();
    }
    else{
        QDateTime current_date_time =QDateTime::currentDateTime();
        QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
        ui->textEdit_log->append(current_date + " [Info] 没有交通标志数据...");
        ui->textEdit_log->moveCursor(QTextCursor::End);
    }
}

/**
*@功能 显示保存检测到的交通标志图片
*
*/
void MainWindow::showSaveImage()
{
    //深拷贝，防止更改原图像，对后面的车道线检测造成影响
    Mat roi = m_frame.clone();
    //颜色空间转换
    cv::cvtColor(roi, roi, CV_BGR2RGB);
    //拼接图片文件名
    QString fileName = "signimages/"+QString::number(m_num)+".jpg";
    //图片号加1
    m_num++;
    //Mat转QImage
    QImage image = QImage((uchar*)(roi.data), roi.cols, roi.rows, QImage::Format_RGB888);
    image.save(fileName,"JPG");
    //显示image
    selectLabelShow(image);
}
