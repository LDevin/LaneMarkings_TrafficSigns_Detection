#include "mainwindow.h"
#include "ui_mainwindow.h"

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
           QDir::homePath() + "/Videos", "video files(*.avi;*.mp4;*.wmv);;all files(*.*)");

    if (!fileName.isEmpty())
    {
        //开打视频文件
        m_capture.open(fileName.toStdString());
        //获取视频帧数
        auto frame_number = m_capture.get(cv::CAP_PROP_FRAME_COUNT);
        //获取fps(帧数/秒)
        auto fps = m_capture.get(cv::CAP_PROP_FPS);
        qDebug() << "[Info] 打开视频:" << fileName << " 总共" << frame_number << "帧  fps:" << fps;
    }
    else
    {
        qDebug() << "[Info] 没有选择视频";
    }
}
/*
*开始检测
*/
void MainWindow::on_pushButton_start_clicked()
{
    //QMessageBox::information(this,tr("提示"),tr("开始检测"),0,0,0);
    //获取当前时间
    QDateTime current_date_time =QDateTime::currentDateTime();
    //字符串化
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss");
    //追加内容
    ui->textEdit_log->append(current_date+"  追加");
    //将光标移到最后
    ui->textEdit_log->moveCursor(QTextCursor::End);
}
/*
*停止检测
*/
void MainWindow::on_pushButton_stop_clicked()
{
    QMessageBox::information(this,tr("提示"),tr("停止检测"),0,0,0);
}
/*
*结束检测
*/
void MainWindow::on_pushButton_end_clicked()
{
    QMessageBox::information(this,tr("提示"),tr("结束检测"),0,0,0);
}
