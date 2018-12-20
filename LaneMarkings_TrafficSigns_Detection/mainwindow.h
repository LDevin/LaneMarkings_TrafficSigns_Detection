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

    void on_checkBox_lane_clicked();

    void on_checkBox_sign_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat m_frame;            //视频的帧
    std::string m_fileName;     //视频文件
    bool m_openvideo_flag;       //打开视频文件是否成功标志
    bool m_stop_flag;            //暂停检测标志
    bool m_start_flag;           //开始检测标志
    bool m_end_flag;             //结束检测标志
};

#endif // MAINWINDOW_H
