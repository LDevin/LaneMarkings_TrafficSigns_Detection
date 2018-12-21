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
    void fillHole(const cv::Mat srcBw, cv::Mat &dstBw);
    bool isInside(cv::Rect rect1, cv::Rect rect2);
    void selectLabelShow(QImage image);
    void showImageList();
private slots:
    void on_pushButton_open_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_pushButton_end_clicked();

    void on_checkBox_lane_clicked();

    void on_checkBox_sign_clicked();

    void on_pushButton_showSign_clicked();

private:
    Ui::MainWindow *ui;
    cv::Mat m_frame;                //视频的帧
    std::string m_fileName;         //视频文件
    bool m_openvideo_flag;          //打开视频文件是否成功标志
    bool m_stop_flag;               //暂停检测标志
    bool m_start_flag;              //开始检测标志
    bool m_end_flag;                //结束检测标志
    bool m_lane_flag;               //车道线检测标志
    bool m_sign_flag;               //交通标志检测标志
    bool m_label_flag[5];           //label是否加载了交通标志图片
    int m_sign_count;               //检测到的交通标志的总个数
    QListWidget *m_imageList;       //图片列表
};

#endif // MAINWINDOW_H
