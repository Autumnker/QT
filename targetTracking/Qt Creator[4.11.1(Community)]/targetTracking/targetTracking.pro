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
    button_ctrl.cpp \
    calculate_rota_angle.cpp \
    color_select_dialog.cpp \
    color_threshold_hsv.cpp \
    config.cpp \
    main.cpp \
    network_frame.cpp \
    widget.cpp

HEADERS += \
    button_ctrl.h \
    calculate_rota_angle.h \
    color_select_dialog.h \
    color_threshold_hsv.h \
    config.h \
    network_frame.h \
    widget.h \

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

## 添加opencv4.11.0库
unix:!macx|win32: LIBS += -L$$PWD/opencv4.11.0_libs/lib/ -llibopencv_world4110.dll

INCLUDEPATH += $$PWD/opencv4.11.0_libs/include
DEPENDPATH += $$PWD/opencv4.11.0_libs/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/opencv4.11.0_libs/lib/libopencv_world4110.dll.lib
else:unix:!macx|win32-g++: PRE_TARGETDEPS += $$PWD/opencv4.11.0_libs/lib/libopencv_world4110.dll.a

# QResources 资源
RESOURCES += \
    src/src.qrc

