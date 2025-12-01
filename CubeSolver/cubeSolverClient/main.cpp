 #include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 注册类型
//  qRegisterMetaType<QHash<int, QPixmap>>("QHash<int, QPixmap>");
    qRegisterMetaType<cv::Mat>("cv::Mat");

    Widget w;
    w.show();

    return a.exec();
}
