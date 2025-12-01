#ifndef CONFIG_H
#define CONFIG_H

#include <QHash>
#include <QString>

namespace Config {
    enum CameraIndex{U, D, LF, BR, CameraNum};
    extern int FrameRate;

    enum U_D_PointDirection{Top, Left , Right, Down, U_D};
    enum LF_BR_PointDirection{TopLeft, TopRight, MidLeft , MidRight, DownLeft, DownRight, LF_BR};
    extern int LineWidth;                                                                           //  画布上的魔方边界线宽
    extern int CircleRadius;                                                                        //  画布上的红色圆圈半径
    extern int CircleWidth;                                                                         //  画布上的红色圆圈线宽

    enum ColorIndex{U_yellow, R_green, F_red, D_white, L_blue, B_orange, ColorNum};                 // 颜色索引
    extern const char colorChars[ColorNum];                                                         // 颜色字符串
    extern const QHash<QChar, QString>colorFaceMap;                                                 // 颜色名称与面的映射关系
    extern const QHash<int, QString>colorOrder;                                                     // 颜色名字与数字序列的映射关系

    // 定义颜色阈值
    struct Threshold {
        int hLower;
        int hUpper;
        int sLower;
        int sUpper;
        int vLower;
        int vUpper;
    };

    // 默认颜色阈值
    extern const QHash<int, Threshold> colorThresholds;

    // 配置文件默认存放路径
    const QString ConfigPath = "./Config/";

    // 求解服务器的url(GET方法)
    const QString solverServerUrl = "http://localhost:8083/solve?cubestring=";

    // 定时器频率(ms)
    const int timerHZ = 50;

    // 色块序列连续出现多少次才能被发送给求解序列服务器
    const int coutToSendThresholdValue = 10;
}


#endif // CONFIG_H
