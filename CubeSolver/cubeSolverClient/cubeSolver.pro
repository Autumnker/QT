QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Config.cpp \
    algorithm.cpp \
    camera.cpp \
    colorprocess.cpp \
    main.cpp \
    mylabel.cpp \
    widget.cpp

HEADERS += \
    algorithm.h \
    camera.h \
    colorprocess.h \
    config.h \
    mylabel.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


## 添加opencv头文件目录和库文件 4.5.1
#INCLUDEPATH += E:/opencv/opencv4.5.1/build_with_MinGW64/install/include
#LIBS += E:/opencv/opencv4.5.1/build_with_MinGW64/lib/libopencv_world451.dll.a


## 添加opencv头文件目录和库文件 4.5.1-25-3-19
#win32: LIBS += -L$$PWD/opencv_libs_MinGW64/lib/ -llibopencv_world451.dll

#INCLUDEPATH += $$PWD/opencv_libs_MinGW64/include
#DEPENDPATH += $$PWD/opencv_libs_MinGW64/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/opencv_libs_MinGW64/lib/libopencv_world451.dll.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/opencv_libs_MinGW64/lib/libopencv_world451.dll.a


## 添加opencv头文件目录和库文件 4.11.0-2025-4-4

win32: LIBS += -L$$PWD/opencv4.11.0_libs/lib/ -llibopencv_world4110.dll

INCLUDEPATH += $$PWD/opencv4.11.0_libs/include
DEPENDPATH += $$PWD/opencv4.11.0_libs/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/opencv4.11.0_libs/lib/libopencv_world4110.dll.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/opencv4.11.0_libs/lib/libopencv_world4110.dll.a
