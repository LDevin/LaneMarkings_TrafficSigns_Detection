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
    int runDetection(std::string fileName);         //功能主要运行车道检测的主要算法输出video，绘制检测到的车道。
    void selectLabelShow(std::string fileName); //选择对应的label来显示交通标志图片
    void showImageList();               //显示检测到的交通标志图片列表
    void showSaveImage(cv::Mat &roi);        //显示保存检测到的交通标志图片
private slots:
    void on_pushButton_open_clicked();      //打开视频文件-点击事件
    void on_pushButton_start_clicked();     //开始检测-点击事件
    void on_pushButton_stop_clicked();      //停止检测--点击事件
    void on_pushButton_end_clicked();       //手动结束检测--点击事件
    void on_checkBox_lane_clicked();        //是否要检测车道线--点击事件
    void on_checkBox_sign_clicked();        //是否要检测交通标志--点击事件
    void on_pushButton_showSign_clicked();  //显示检测到的交通标志图片列表--点击事件

private:
    Ui::MainWindow *ui;
    cv::Mat m_frame;                //视频的帧
    std::string m_video_filename;   //视频文件
    bool m_openvideo_flag;          //打开视频文件是否成功标志
    bool m_stop_flag;               //暂停检测标志
    bool m_start_flag;              //开始检测标志
    bool m_end_flag;                //结束检测标志
    bool m_lane_flag;               //车道线检测标志
    bool m_sign_flag;               //交通标志检测标志
    bool m_label_flag[5];           //label是否加载了交通标志图片
    int m_sign_count;               //检测到的交通标志的总个数
    int m_num;                      //图片号
    QListWidget *m_imagelist;       //图片列表

private:
    void closeEvent(QCloseEvent *event);    //重载关闭事件
};

#endif // MAINWINDOW_H
