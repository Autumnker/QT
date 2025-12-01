#include "config.h"

namespace Config {
    int FrameRate = 20;                     //  相机帧率(单位ms)
    int LineWidth = 3;                      //  画布上的绿色线宽
    int CircleRadius = 20;                  //  画布上的红色圆圈半径
    int CircleWidth = 1;                    //  画布上的红色圆圈线宽

    // 定义并初始化 colorChars 数组
    const char colorChars[ColorNum] = {'U', 'R', 'F', 'D', 'L', 'B'};
    const QHash<QChar, QString>colorFaceMap = {
        {'U', "yellow"},
        {'R', "green"},
        {'F', "red"},
        {'D', "#DFDFDF"},   // 浅白色
        {'L', "blue"},
        {'B', "orange"},
        {'N', "gray"}
    };

    const QHash<int, QString>colorOrder = {
        {U_yellow,  "yellow"},
        {R_green,   "green"},
        {F_red,     "red"},
        {D_white,   "white"},
        {L_blue,    "blue"},
        {B_orange,  "orange"}
    };

    // 默认颜色阈值
    const QHash<int, Threshold> colorThresholds = {
            {U_yellow, {20, 30, 100, 255, 100, 255}},
            {R_green, {35, 70, 100, 255, 100, 255}},
            {F_red, {0, 10, 100, 255, 100, 255}},
            {D_white, {0, 180, 0, 30, 220, 255}},
            {L_blue, {100, 130, 100, 255, 100, 255}},
            {B_orange, {11, 25, 100, 255, 100, 255}}
    };

}
