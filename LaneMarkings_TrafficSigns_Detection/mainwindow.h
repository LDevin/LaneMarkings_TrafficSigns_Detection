#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <opencv2/videoio.hpp>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    int runLaneDetection(std::string fileName);     //开始运行车道线检测
private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_pushButton_end_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat m_frame;            //视频的帧
    QString m_current_date;     //当前时间
    int m_openvideo_flag;       //打开视频文件是否成功标志
    std::string m_fileName;     //视频文件
    int m_stop_flag;            //暂停检测标志
    int m_start_flag;           //开始检测标志
    int m_end_flag;             //结束检测标志
};

#endif // MAINWINDOW_H
