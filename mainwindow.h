#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDockWidget>
#include <QSlider>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QImage>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QVideoFrame>
#include <QString>
#include <QImage>
#include <QMdiSubWindow>
#include<QStackedWidget>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_triggered();
    void on_thumbnail_clicked(const QModelIndex &index);

    void on_grayScaleButton_clicked();
    void on_binaryButton_clicked();
    void on_meanFilterButton_clicked();
    void on_gammaButton_clicked();
    void on_edgeDetectionButton_clicked();
    void on_binaryThresholdChanged(int value);
    void on_gammaValueChanged(double value);

    void on_actionAbout_triggered();

    void on_resetButton_clicked();

    void on_grayscaleButton1_clicked();   // 灰度化（视频）
    void on_binaryButton1_clicked();      // 二值化（视频）
    void on_edgeDetectionButton1_clicked();// 边缘检测（视频）
    void on_meanFilterButton1_clicked();  // 均值滤波（视频）
    void on_mosaicButton1_clicked();
    void on_resetVideoButton_clicked();

    void on_yoloTrackingButton_clicked();
private:
    Ui::MainWindow *ui;
    QLabel *imageLabel;           // 用于显示当前图片的标签
    QListWidget *thumbnailList;   // 用于显示图片缩略图的列表
    QStringList imagePaths;       // 存储所有打开的图片路径
    QScrollArea *imageScrollArea; // 图片显示区域的滚动窗口
    QDockWidget *toolBoxDock;     // 工具箱停靠窗口
    QPixmap originalPixmap;       // 原始图片，用于恢复和处理
    QSlider *gammaSlider;
    // 图像处理控件
    QSlider *binaryThresholdSlider;
    QDoubleSpinBox *gammaSpinBox;

    QIcon infoIcon;               // 自定义信息图标
    QIcon aboutIcon;      // 存储原始图片
    QPushButton *resetButton;

    QString currentProcessingMode;  // 当前图像处理模式："grayscale", "binary", etc.
    QLabel *videoDisplayLabel;      // 显示视频帧的 QLabel
    QMdiSubWindow *videoSubWindow;
    QMediaPlayer *videoPlayer;
    QVideoSink *videoSink;
    QMdiArea *mdiArea;

    void setupVideoProcessing();
    void onVideoFrame(const QVideoFrame &frame);
     // 用于显示视频帧
    QScrollArea *videoScrollArea;

    QStackedWidget *stackedWidget;


    cv::dnn::Net net = cv::dnn::readNetFromONNX("C:/Users/11568/Desktop/Qt/yangfan249350532/yolov5s.onnx");    // 滚动区域
    QImage applyImageProcessing(const QImage &image);

};
#endif // MAINWINDOW_H
