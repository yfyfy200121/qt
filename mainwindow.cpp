#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QActionGroup>
#include <QToolButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QMdiSubWindow>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDebug>
#include <QPalette>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QStyleFactory>
#include <opencv2/imgproc.hpp>
#define QIMAGE_TO_CVMAT(qimg, mat) \
{ \
        QImage img = qimg.convertToFormat(QImage::Format_RGB888); \
        mat = cv::Mat(img.height(), img.width(), CV_8UC3, const_cast<uchar*>(img.bits()), img.bytesPerLine()).clone(); \
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cv::dnn::Net net = cv::dnn::readNetFromONNX("yolov5s.onnx");

    // 设置应用风格
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // 设置调色板，美化界面
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    // 设置主窗口图标

    setWindowIcon(QIcon(":/images/images/hq.ico"));

    videoDisplayLabel = new QLabel(this);
    videoDisplayLabel->setBackgroundRole(QPalette::Base);
    videoDisplayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setVisible(false);

    // 将 imageScrollArea 添加为第一页

    QMenu *fileMenu = ui->menuBar->addMenu(tr("文件(&F)")); // 添加文件菜单
    QAction *action_Open = fileMenu->addAction(             // 添加打开菜单
        QIcon(":/images/images/open.png"), tr("打开文件(&O)"));
    action_Open->setShortcut(QKeySequence("Ctrl+O"));       // 设置快捷键
    ui->mainToolBar->addAction(action_Open);                // 在工具栏中添加动作



    // 在 MainWindow 构造函数中添加
    QAction *action_PlayVideo = new QAction(QIcon(":/images/images/bf.jpg"), tr("播放视频(&V)"), this);
    action_PlayVideo->setShortcut(QKeySequence("Ctrl+V"));

    // 绑定到你的视频处理函数
    connect(action_PlayVideo, &QAction::triggered, this, &MainWindow::setupVideoProcessing);

    // 将动作添加到工具栏
    ui->mainToolBar->addAction(action_PlayVideo);
    QAction *actionPlayVideo = new QAction(tr("播放视频(&V)"), this);
    actionPlayVideo->setShortcut(QKeySequence("Ctrl+V"));

    connect(actionPlayVideo, &QAction::triggered, this, &MainWindow::setupVideoProcessing);
    QSpinBox *spinBox = new QSpinBox(this);             // 创建QSpinBox
    spinBox->setRange(1, 1000);
    spinBox->setValue(100);
    spinBox->setSuffix(" %");
    ui->mainToolBar->addWidget(spinBox);                // 向工具栏添加QSpinBox部件


    fileMenu->addAction(actionPlayVideo);
    // 显示临时消息，显示2000毫秒即2秒
    ui->statusBar->showMessage(tr("欢迎使用图片查看器"), 2000);
    // 创建标签，设置标签样式并显示信息，然后将其以永久部件的形式添加到状态栏
    QLabel *permanent = new QLabel(this);
    permanent->setFrameStyle(QFrame::Box | QFrame::Sunken);
    permanent->setText("杨帆249350532");
    ui->statusBar->addPermanentWidget(permanent);

    // 加载自定义图标
    infoIcon = QIcon(":/images/images/hh.png");
    aboutIcon = QIcon(":/images/images/hq.png");

    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    // 添加 YOLO 按钮
    QAction *action_Yolo = new QAction(QIcon(":/images/images/find.png"), tr("YOLO 目标追踪"), this);
    action_Yolo->setShortcut(QKeySequence("Ctrl+Y"));
    connect(action_Yolo, &QAction::triggered, this, &MainWindow::on_yoloTrackingButton_clicked);
    ui->mainToolBar->addAction(action_Yolo);
    // 创建用于显示图片的标签
    imageLabel = new QLabel(this);
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setAlignment(Qt::AlignCenter);

    imageScrollArea = new QScrollArea(this);
    imageScrollArea->setBackgroundRole(QPalette::Dark);
    imageScrollArea->setWidget(imageLabel);
    imageScrollArea->setWidgetResizable(true);
    imageScrollArea->setMinimumHeight(300);

    // 创建缩略图列表
    thumbnailList = new QListWidget(this);
    thumbnailList->setViewMode(QListWidget::IconMode);
    thumbnailList->setIconSize(QSize(120, 120));
    thumbnailList->setResizeMode(QListWidget::Adjust);
    thumbnailList->setSpacing(5);
    thumbnailList->setMovement(QListWidget::Static);
    thumbnailList->setSelectionMode(QListWidget::SingleSelection);

    QScrollArea *thumbnailScrollArea = new QScrollArea(this);
    thumbnailScrollArea->setBackgroundRole(QPalette::Dark);
    thumbnailScrollArea->setWidget(thumbnailList);
    thumbnailScrollArea->setWidgetResizable(true);
    thumbnailScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    thumbnailScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    thumbnailScrollArea->setMaximumHeight(150);

    QMenu *helpMenu = ui->menuBar->addMenu(tr("帮助(&H)"));
    QAction *actionAbout = helpMenu->addAction(tr("关于(&A)"));
    connect(actionAbout, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);

    // 创建 stackedWidget 管理图像与视频显示
    stackedWidget = new QStackedWidget(this);

    stackedWidget->addWidget(imageScrollArea);     // 页面0 - 图像显示
    stackedWidget->addWidget(videoDisplayLabel);   // 页面1 - 视频显示
    stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoDisplayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stackedWidget->setCurrentIndex(0);
    imageLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    // 将 stackedWidget 加入主布局
    mainLayout->addWidget(stackedWidget, 1);       // 占据主要空间

    // 添加缩略图滚动区域
    mainLayout->addWidget(thumbnailScrollArea, 0);

    // 设置为主窗口
    setCentralWidget(centralWidget);


    // 创建工具箱
    toolBoxDock = new QDockWidget(tr("图像处理"), this);
    toolBoxDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    QWidget *toolBoxWidget = new QWidget(toolBoxDock);
    QVBoxLayout *toolBoxLayout = new QVBoxLayout(toolBoxWidget);

    // 按钮区域 - 顶部
    QWidget *buttonWidget = new QWidget(toolBoxWidget);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);

    // 灰度化按钮
    QPushButton *grayScaleButton = new QPushButton(tr("灰度化"), buttonWidget);
    buttonLayout->addWidget(grayScaleButton);

    // 均值滤波按钮
    QPushButton *meanFilterButton = new QPushButton(tr("3×3均值滤波"), buttonWidget);
    buttonLayout->addWidget(meanFilterButton);

    // 边缘检测按钮
    QPushButton *edgeDetectionButton = new QPushButton(tr("Sobel边缘检测"), buttonWidget);
    buttonLayout->addWidget(edgeDetectionButton);

    // 添加按钮区域到工具箱
    toolBoxLayout->addWidget(buttonWidget);

    // 滑块区域 - 底部
    QWidget *sliderWidget = new QWidget(toolBoxWidget);
    QVBoxLayout *sliderLayout = new QVBoxLayout(sliderWidget);

    // 二值化滑块
    QGroupBox *binaryGroup = new QGroupBox(tr("二值化"), sliderWidget);
    QVBoxLayout *binaryLayout = new QVBoxLayout(binaryGroup);

    binaryThresholdSlider = new QSlider(Qt::Horizontal, binaryGroup);
    binaryThresholdSlider->setRange(0, 255);
    binaryThresholdSlider->setValue(128);

    QLabel *binaryThresholdLabel = new QLabel(tr("阈值: 128"), binaryGroup);

    binaryLayout->addWidget(binaryThresholdSlider);
    binaryLayout->addWidget(binaryThresholdLabel);

    sliderLayout->addWidget(binaryGroup);

    // 伽马变换滑块
    QGroupBox *gammaGroup = new QGroupBox(tr("伽马变换"), sliderWidget);
    QVBoxLayout *gammaLayout = new QVBoxLayout(gammaGroup);

    gammaSlider = new QSlider(Qt::Horizontal, gammaGroup);
    gammaSlider->setRange(1, 50);  // 范围 0.1 到 5.0，乘以10以获得整数
    gammaSlider->setValue(10);     // 默认值 1.0

    QLabel *gammaLabel = new QLabel(tr("伽马值: 1.0"), gammaGroup);

    gammaLayout->addWidget(gammaSlider);
    gammaLayout->addWidget(gammaLabel);

    sliderLayout->addWidget(gammaGroup);

    // 添加重置按钮 - 放在滑块区域下方
    resetButton = new QPushButton(tr("重置图片"), sliderWidget);
    resetButton->setToolTip(tr("恢复原始图片"));
    resetButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4a86e8;"
        "    border-radius: 5px;"
        "    color: white;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3a76d8;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2a66c8;"
        "}"
        );
    sliderLayout->addWidget(resetButton);

    // 添加滑块区域到工具箱
    toolBoxLayout->addWidget(sliderWidget);

    // 添加伸展空间
    toolBoxLayout->addStretch(1);

    toolBoxDock->setWidget(toolBoxWidget);
    addDockWidget(Qt::RightDockWidgetArea, toolBoxDock);

    // 连接信号和槽
    connect(action_Open, &QAction::triggered, this, &MainWindow::on_action_Open_triggered);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        if (!imageLabel->pixmap().isNull()) {
            QPixmap scaledPixmap = imageLabel->pixmap().scaled(
                value / 100.0 * imageScrollArea->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                );
            imageLabel->setPixmap(scaledPixmap);
            imageLabel->adjustSize();
        }
    });
    connect(thumbnailList, &QListWidget::clicked, this, &MainWindow::on_thumbnail_clicked);

    // 连接图像处理按钮
    connect(grayScaleButton, &QPushButton::clicked, this, &MainWindow::on_grayScaleButton_clicked);
    connect(meanFilterButton, &QPushButton::clicked, this, &MainWindow::on_meanFilterButton_clicked);
    connect(edgeDetectionButton, &QPushButton::clicked, this, &MainWindow::on_edgeDetectionButton_clicked);

    // 连接滑块，实现实时预览
    connect(binaryThresholdSlider, &QSlider::valueChanged, [=](int value) {
        binaryThresholdLabel->setText(tr("阈值: %1").arg(value));
        on_binaryThresholdChanged(value);
    });

    connect(gammaSlider, &QSlider::valueChanged, [=](int value) {
        double gammaValue = value / 10.0;  // 转换回实际的伽马值
        gammaLabel->setText(tr("伽马值: %1").arg(gammaValue));
        on_gammaValueChanged(gammaValue);
    });

    // 连接重置按钮
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::on_resetButton_clicked);

    // 美化按钮
    QList<QPushButton*> allButtons = buttonWidget->findChildren<QPushButton*>();
    foreach (QPushButton* button, allButtons) {
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: #4a86e8;"
            "    border-radius: 5px;"
            "    color: white;"
            "    padding: 8px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #3a76d8;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #2a66c8;"
            "}"
            );
    }

    // 美化滑块
    QList<QSlider*> allSliders = sliderWidget->findChildren<QSlider*>();
    foreach (QSlider* slider, allSliders) {
        slider->setStyleSheet(
            "QSlider::groove:horizontal {"
            "    border: 1px solid #bbb;"
            "    background: white;"
            "    height: 10px;"
            "    border-radius: 4px;"
            "}"
            "QSlider::handle:horizontal {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4a86e8, stop:1 #3a76d8);"
            "    border: 1px solid #5c5c5c;"
            "    width: 18px;"
            "    margin: -4px 0;"
            "    border-radius: 8px;"
            "}"
            );

    }

    QGroupBox *videoProcessGroup = new QGroupBox(tr("视频处理"), toolBoxWidget);
    QVBoxLayout *videoProcessLayout = new QVBoxLayout(videoProcessGroup);

    // 创建视频处理按钮
    QPushButton *grayScaleVideoButton = new QPushButton(tr("灰度化"), videoProcessGroup);
    QPushButton *edgeDetectionVideoButton = new QPushButton(tr("边缘检测"), videoProcessGroup);
    QPushButton *meanFilterVideoButton = new QPushButton(tr("均值滤波"), videoProcessGroup);
    QPushButton *binaryVideoButton = new QPushButton(tr("二值化"), videoProcessGroup);
    QPushButton *mosaicVideoButton = new QPushButton(tr("马赛克"), videoProcessGroup);

    // 创建重置按钮
    QPushButton *resetVideoButton = new QPushButton(tr("重置视频"), videoProcessGroup);

    // 设置按钮样式
    QString buttonStyle =
        "QPushButton {"
        "    background-color: #4a86e8;"
        "    border-radius: 5px;"
        "    color: white;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3a76d8;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2a66c8;"
        "}";

    QPushButton* buttons[] = {
        grayScaleVideoButton,
        edgeDetectionVideoButton,
        meanFilterVideoButton,
        binaryVideoButton,
        mosaicVideoButton,
        resetVideoButton
    };

    for (QPushButton* btn : buttons) {
        btn->setStyleSheet(buttonStyle);
    }

    // 添加到布局
    videoProcessLayout->addWidget(grayScaleVideoButton);
    videoProcessLayout->addWidget(edgeDetectionVideoButton);
    videoProcessLayout->addWidget(meanFilterVideoButton);
    videoProcessLayout->addWidget(binaryVideoButton);
    videoProcessLayout->addWidget(mosaicVideoButton);
    videoProcessLayout->addWidget(resetVideoButton);  // 添加重置按钮
    videoProcessLayout->addStretch();                 // 保持美观

    // 将分组框加入工具箱主布局
    toolBoxLayout->addWidget(videoProcessGroup);

    connect(grayScaleVideoButton, &QPushButton::clicked, this, &MainWindow::on_grayscaleButton1_clicked);
    connect(edgeDetectionVideoButton, &QPushButton::clicked, this, &MainWindow::on_edgeDetectionButton1_clicked);
    connect(meanFilterVideoButton, &QPushButton::clicked, this, &MainWindow::on_meanFilterButton1_clicked);
    connect(binaryVideoButton, &QPushButton::clicked, this, &MainWindow::on_binaryButton1_clicked);
    connect(mosaicVideoButton, &QPushButton::clicked, this, &MainWindow::on_mosaicButton1_clicked);

    // 新增：连接重置按钮
    connect(resetVideoButton, &QPushButton::clicked, this, &MainWindow::on_resetVideoButton_clicked);
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Open_triggered()
{
    // 打开文件对话框，支持选择多个图片文件
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("打开图片"),
        "",
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*)")
        );

    if (!fileNames.isEmpty()) {
        // 清空现有图片和缩略图
        imagePaths.clear();
        thumbnailList->clear();

        // 添加新选择的图片
        for (const QString &fileName : fileNames) {
            imagePaths.append(fileName);

            // 加载缩略图
            QPixmap thumbnail(fileName);
            if (!thumbnail.isNull()) {
                // 缩放缩略图到合适大小
                thumbnail = thumbnail.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // 创建列表项
                QListWidgetItem *item = new QListWidgetItem(QIcon(thumbnail), QFileInfo(fileName).fileName());
                item->setData(Qt::UserRole, fileName);
                thumbnailList->addItem(item);
            }
        }

        // 如果有图片，显示第一张
        if (!imagePaths.isEmpty()) {
            QPixmap pixmap(imagePaths.first());
            if (!pixmap.isNull()) {
                originalPixmap = pixmap; // 保存原始图片

                QPixmap scaledPixmap = pixmap.scaled(
                    imageScrollArea->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );
                imageLabel->setPixmap(scaledPixmap);
                imageLabel->adjustSize();

                // 选中第一个缩略图
                if (thumbnailList->count() > 0) {
                    thumbnailList->setCurrentRow(0);
                }

                // 在状态栏显示图片信息
                ui->statusBar->showMessage(tr("已加载 %1 张图片").arg(imagePaths.size()));
                stackedWidget->setCurrentIndex(0);
            }
        }
    }
}

void MainWindow::on_thumbnail_clicked(const QModelIndex &index)
{
    if (index.isValid() && index.row() < imagePaths.size()) {
        QString fileName = imagePaths[index.row()];
        QPixmap pixmap(fileName);

        if (!pixmap.isNull()) {
            originalPixmap = pixmap; // 保存原始图片

            QPixmap scaledPixmap = pixmap.scaled(
                imageScrollArea->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                );
            imageLabel->setPixmap(scaledPixmap);
            imageLabel->adjustSize();

            // 在状态栏显示当前图片信息
            ui->statusBar->showMessage(tr("显示图片: %1").arg(QFileInfo(fileName).fileName()));
        }
    }
}

// 灰度化处理
void MainWindow::on_grayScaleButton_clicked()
{
    if (originalPixmap.isNull()) return;

    QImage image = originalPixmap.toImage();

    // 灰度化处理 ((R+G+B)/3)
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int gray = qRound((qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3.0);
            image.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("已应用灰度化处理"));
}

// 二值化处理
void MainWindow::on_binaryButton_clicked()
{
    if (originalPixmap.isNull()) return;

    int threshold = binaryThresholdSlider->value();
    QImage image = originalPixmap.toImage();

    // 二值化处理
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int gray = qRound((qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3.0);
            int binary = (gray >= threshold) ? 255 : 0;
            image.setPixel(x, y, qRgb(binary, binary, binary));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("已应用二值化处理，阈值: %1").arg(threshold));
}

// 3×3均值滤波
void MainWindow::on_meanFilterButton_clicked()
{
    if (originalPixmap.isNull()) return;

    QImage image = originalPixmap.toImage();
    QImage result = image.copy();

    // 3×3均值滤波核
    const int kernelSize = 3;
    const int kernel[] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };
    const int kernelSum = 9;

    // 应用滤波
    for (int y = 1; y < image.height() - 1; y++) {
        for (int x = 1; x < image.width() - 1; x++) {
            int rSum = 0, gSum = 0, bSum = 0;

            // 计算卷积
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    QRgb pixel = image.pixel(x + kx, y + ky);
                    int weight = kernel[(ky + 1) * kernelSize + (kx + 1)];
                    rSum += qRed(pixel) * weight;
                    gSum += qGreen(pixel) * weight;
                    bSum += qBlue(pixel) * weight;
                }
            }

            // 归一化
            int r = qBound(0, rSum / kernelSum, 255);
            int g = qBound(0, gSum / kernelSum, 255);
            int b = qBound(0, bSum / kernelSum, 255);

            result.setPixel(x, y, qRgb(r, g, b));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(result);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("已应用3×3均值滤波"));
}

// 伽马变换
void MainWindow::on_gammaButton_clicked()
{
    if (originalPixmap.isNull()) return;

    double gamma = gammaSpinBox->value();
    QImage image = originalPixmap.toImage();

    // 创建查找表
    unsigned char table[256];
    for (int i = 0; i < 256; i++) {
        table[i] = qBound(0, qRound(255.0 * pow(i / 255.0, gamma)), 255);
    }

    // 应用伽马变换
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int r = table[qRed(pixel)];
            int g = table[qGreen(pixel)];
            int b = table[qBlue(pixel)];
            image.setPixel(x, y, qRgb(r, g, b));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("已应用伽马变换，伽马值: %1").arg(gamma));
}

// Sobel边缘检测
void MainWindow::on_edgeDetectionButton_clicked()
{
    if (originalPixmap.isNull()) return;

    QImage image = originalPixmap.toImage();
    QImage result = image.copy();

    // 转换为灰度图
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    // Sobel算子 - x方向
    const int sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    // Sobel算子 - y方向
    const int sobelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    // 应用Sobel算子
    for (int y = 1; y < grayImage.height() - 1; y++) {
        for (int x = 1; x < grayImage.width() - 1; x++) {
            int gx = 0, gy = 0;

            // 计算梯度
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    uchar pixel = grayImage.pixel(x + kx, y + ky);
                    gx += pixel * sobelX[ky + 1][kx + 1];
                    gy += pixel * sobelY[ky + 1][kx + 1];
                }
            }

            // 计算梯度幅值
            int g = qBound(0, qRound(sqrt(gx * gx + gy * gy)), 255);

            // 设置结果像素
            result.setPixel(x, y, qRgb(g, g, g));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(result);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("已应用Sobel边缘检测"));
}

// 二值化阈值变化 - 实时预览
void MainWindow::on_binaryThresholdChanged(int value)
{
    if (originalPixmap.isNull()) return;

    int threshold = value;
    QImage image = originalPixmap.toImage();

    // 二值化处理
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int gray = qRound((qRed(pixel) + qGreen(pixel) + qBlue(pixel)) / 3.0);
            int binary = (gray >= threshold) ? 255 : 0;
            image.setPixel(x, y, qRgb(binary, binary, binary));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("实时二值化预览，阈值: %1").arg(threshold));
}

// 伽马值变化 - 实时预览
void MainWindow::on_gammaValueChanged(double value)
{
    if (originalPixmap.isNull()) return;

    double gamma = value;
    QImage image = originalPixmap.toImage();

    // 创建查找表
    unsigned char table[256];
    for (int i = 0; i < 256; i++) {
        table[i] = qBound(0, qRound(255.0 * pow(i / 255.0, gamma)), 255);
    }

    // 应用伽马变换
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int r = table[qRed(pixel)];
            int g = table[qGreen(pixel)];
            int b = table[qBlue(pixel)];
            image.setPixel(x, y, qRgb(r, g, b));
        }
    }

    // 显示处理后的图片
    QPixmap processedPixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(processedPixmap.scaled(
        imageScrollArea->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    ui->statusBar->showMessage(tr("实时伽马变换预览，伽马值: %1").arg(gamma));
}

// 关于对话框
void MainWindow::on_actionAbout_triggered()
{

    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("关于本软件"));
    aboutBox.setTextFormat(Qt::RichText); // 允许使用HTML格式
    aboutBox.setText(tr("<h3>图片查看器</h3>"
                        "<p>版本: 1.0</p>"
                        "<p>作者: 杨帆</p>"
                        "<p>学号: 249350532</p>"
                        "<p>本软件用于查看和处理图片文件，支持灰度化、二值化、均值滤波、伽马变换和边缘检测等功能。</p>"));
    aboutBox.setIconPixmap(aboutIcon.pixmap(64, 64, QIcon::Active, QIcon::On));
    aboutBox.setStandardButtons(QMessageBox::Ok);
    aboutBox.exec();
}
void MainWindow::on_resetButton_clicked()
{
    if (!originalPixmap.isNull()) {
        // 恢复原始图片
        QPixmap scaledPixmap = originalPixmap.scaled(
            imageScrollArea->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );
        imageLabel->setPixmap(scaledPixmap);
        imageLabel->adjustSize();

        ui->statusBar->showMessage(tr("已恢复原始图片"));
    }
}
void MainWindow::setupVideoProcessing()
{

    QString filePath = QFileDialog::getOpenFileName(this, "选择视频文件");
    if (filePath.isEmpty()) return;

    videoPlayer = new QMediaPlayer(this);
    videoSink = new QVideoSink(this);
    videoDisplayLabel->setVisible(true);
    connect(videoSink, &QVideoSink::videoFrameChanged, this, &MainWindow::onVideoFrame);
    videoPlayer->setVideoSink(videoSink);

    videoPlayer->setSource(QUrl::fromLocalFile(filePath));
    videoPlayer->play();

    videoDisplayLabel->setAlignment(Qt::AlignCenter);

    // 设置标签大小
    videoDisplayLabel->setMinimumSize(400, 300); // 可选最小尺寸
    videoDisplayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stackedWidget->setCurrentIndex(1);
}
// 原代码保持不变...

// 原代码保持不变...

void MainWindow::onVideoFrame(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return;

    QImage image = frame.toImage();

    if (!currentProcessingMode.isEmpty()) {
        // 创建处理后的图像副本
        QImage processedImage = applyImageProcessing(image);

        // 显示处理后的图像
        QPixmap pixmap = QPixmap::fromImage(processedImage);
        videoDisplayLabel->setPixmap(pixmap.scaled(
            videoDisplayLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    } else {
        // 自动缩放并保持比例
        QPixmap pixmap = QPixmap::fromImage(image);
        videoDisplayLabel->setPixmap(pixmap.scaled(
            videoDisplayLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    }
}

QImage MainWindow::applyImageProcessing(const QImage &image)
{
    QImage processedImage = image.convertToFormat(QImage::Format_RGB888);

    if (currentProcessingMode == "grayscale") {
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                QRgb pixel = image.pixel(x, y);
                int gray = qGray(pixel);
                processedImage.setPixel(x, y, qRgb(gray, gray, gray));
            }
        }
    }
    else if (currentProcessingMode == "binary") {
        int threshold = 128;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                QRgb pixel = image.pixel(x, y);
                int gray = qGray(pixel);
                int binary = (gray >= threshold) ? 255 : 0;
                processedImage.setPixel(x, y, qRgb(binary, binary, binary));
            }
        }
    }
    else if (currentProcessingMode == "edge") {
        QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
        QImage resultImage = grayImage.copy();
        const int sobelX[3][3] = {
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };

        const int sobelY[3][3] = {
            {-1, -2, -1},
            {0, 0, 0},
            {1, 2, 1}
        };

        for (int y = 1; y < grayImage.height() - 1; ++y) {
            for (int x = 1; x < grayImage.width() - 1; ++x) {
                int gx = 0, gy = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        uchar pixel = grayImage.pixel(x + kx, y + ky);
                        gx += pixel * sobelX[ky + 1][kx + 1];
                        gy += pixel * sobelY[ky + 1][kx + 1];
                    }
                }
                int g = qBound(0, static_cast<int>(sqrt(gx * gx + gy * gy)), 255);
                resultImage.setPixel(x, y, qRgb(g, g, g));
            }
        }
        processedImage = resultImage;
    }
    else if (currentProcessingMode == "mean") {
        // 确保图像是 32 位 ARGB 格式（每个像素 4 字节）
        QImage inputImage = image.convertToFormat(QImage::Format_ARGB32);
        QImage resultImage(inputImage.size(), QImage::Format_ARGB32);

        const int kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
        const int kernelSum = 9;

        int width = inputImage.width();
        int height = inputImage.height();

        const uchar *srcBits = inputImage.constBits();
        int srcBytesPerLine = inputImage.bytesPerLine();

        uchar *destBits = resultImage.bits();
        int destBytesPerLine = resultImage.bytesPerLine();

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                int rSum = 0, gSum = 0, bSum = 0;

                for (int ky = -1; ky <= 1; ++ky) {
                    const uchar *line = srcBits + (y + ky) * srcBytesPerLine;
                    for (int kx = -1; kx <= 1; ++kx) {
                        int offset = (x + kx) * 4;
                        int weight = kernel[(ky + 1) * 3 + (kx + 1)];

                        bSum += line[offset] * weight;
                        gSum += line[offset + 1] * weight;
                        rSum += line[offset + 2] * weight;
                    }
                }

                int destOffset = y * destBytesPerLine + x * 4;
                destBits[destOffset] = bSum / kernelSum;
                destBits[destOffset + 1] = gSum / kernelSum;
                destBits[destOffset + 2] = rSum / kernelSum;
                destBits[destOffset + 3] = 255; // Alpha
            }
        }

        processedImage = resultImage;
    }
    else if (currentProcessingMode == "mosaic") {
        int blockSize = 10;
        for (int y = 0; y < image.height(); y += blockSize) {
            for (int x = 0; x < image.width(); x += blockSize) {
                QRgb blockColor = image.pixel(x, y);
                for (int dy = 0; dy < blockSize && y + dy < image.height(); ++dy) {
                    for (int dx = 0; dx < blockSize && x + dx < image.width(); ++dx) {
                        processedImage.setPixel(x + dx, y + dy, blockColor);
                    }
                }
            }
        }
    }
    else if (currentProcessingMode == "yolo") {
        // 将 QImage 转换为 OpenCV Mat
        cv::Mat frame;
        QImage imageCopy = image.convertToFormat(QImage::Format_RGB888);
        QIMAGE_TO_CVMAT(imageCopy, frame); // 确保是 RGB 格式

        if (frame.empty()) {
            qDebug() << "图像为空，无法进行 YOLO 处理";
            return processedImage;
        }

        // 图像预处理
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(), true, false);

        // 输入模型
        net.setInput(blob);
        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        // 解析输出
        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;
        std::vector<int> classIds;

        float x_factor = frame.cols / 640.0f;
        float y_factor = frame.rows / 640.0f;

        for (int i = 0; i < outputs[0].size[1]; ++i) {
            float* data = reinterpret_cast<float*>(outputs[0].data) + i * outputs[0].size[2];

            float confidence = data[4];
            if (confidence < 0.5) continue;

            float* classes_scores = data + 5;
            cv::Mat scores(1, outputs[0].size[2] - 5, CV_32FC1, classes_scores);
            cv::Point class_id_point;
            double max_class_score;
            cv::minMaxLoc(scores, nullptr, &max_class_score, nullptr, &class_id_point);
            if (max_class_score > 0.5) {
                int class_id = class_id_point.x;
                float cx = data[0];
                float cy = data[1];
                float w = data[2];
                float h = data[3];

                int left = int((cx - 0.5 * w) * x_factor);
                int top = int((cy - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);

                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
                classIds.push_back(class_id);
            }
        }

        // NMS 非极大值抑制
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.4, indices);

        // 绘制检测框与标签
        static const std::vector<std::string> classNames = {
            "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat",
            "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
            "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack",
            // ... 其他类别省略，保留完整80类
        };

        for (int idx : indices) {
            if (classIds[idx] >= classNames.size()) continue;

            const auto& clsName = classNames[classIds[idx]];
            cv::Scalar color = cv::Scalar(0, 255, 0); // 绿色边框

            // 修复：使用 Qt 的格式化方式，避免 C 风格格式化字符串问题
            QString label = QString("%1 %2").arg(QString::fromStdString(clsName))
                                .arg(confidences[idx], 0, 'f', 2);

            // 绘制矩形框
            cv::rectangle(frame, boxes[idx], color, 2);

            // 显示类别与置信度
            cv::putText(frame, label.toStdString(), boxes[idx].tl(),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
        }
        // 转回 QImage
        processedImage = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888).copy();
    }

    return processedImage;
}

// 原代码保持不变...
// 原代码保持不变...

void MainWindow::on_grayscaleButton1_clicked()
{
    currentProcessingMode = "grayscale";
}

void MainWindow::on_binaryButton1_clicked()
{
    currentProcessingMode = "binary";
}

void MainWindow::on_edgeDetectionButton1_clicked()
{
    currentProcessingMode = "edge";
}

void MainWindow::on_meanFilterButton1_clicked()
{
    currentProcessingMode = "mean";
}

void MainWindow::on_mosaicButton1_clicked()
{
    currentProcessingMode = "mosaic";
}
void MainWindow::on_resetVideoButton_clicked()
{
    // 清除当前视频处理模式
    currentProcessingMode.clear();

    // 可选：更新 UI 或提示信息
    ui->statusBar->showMessage(tr("已重置视频处理效果"));
}
void MainWindow::on_yoloTrackingButton_clicked()
{
    currentProcessingMode = "yolo";
}
