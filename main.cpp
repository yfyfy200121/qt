#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("数字图像处理软件V1.0杨帆249350532");
    w.show();
    return a.exec();
}
