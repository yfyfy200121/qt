QT       += core gui
QT += multimedia  multimediawidgets
QT += multimedia
QT += core gui widgets multimedia multimediawidgets concurrent opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resoures.qrc

INCLUDEPATH += "C:/opencv/build/include"

# 库文件路径和链接参数
LIBS += -L"C:/opencv/build/x64/mingw64/lib" \
    -lopencv_core4110 \
    -lopencv_imgproc4110 \
    -lopencv_highgui4110 \
    -lopencv_dnn4110


