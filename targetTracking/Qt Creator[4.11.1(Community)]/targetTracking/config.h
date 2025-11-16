#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

namespace Config {
namespace Widget {
constexpr const char *PROJ_NAME = "目标追踪上位机";
}
namespace Button_Ctrl {
constexpr const char *CHOICE_COLOR          = "选择要识别的颜色";
constexpr const char *DEFAULT_CHOICED_COLOR = "White";                    // 默认追踪颜色:白色
constexpr const char *disconnected_color    = "background-color: red;";   // 成功连接到下位机时图标颜色
constexpr const char *connected_color       = "background-color: green;"; // 连接到下位机失败时图标颜色
constexpr const int x_min_angle             = -90;                        // x轴方向上旋转角度的最小值
constexpr const int x_max_angle             = 90;                         // x轴方向上旋转角度的最大值
constexpr const int y_min_angle             = -10;                        // y轴方向上旋转角度的最小值
constexpr const int y_max_angle             = 80;                         // y轴方向上旋转角度的最大值
}
namespace NetWork_Fram {
constexpr const char *ip    = "192.168.4.1"; // 下位机IP
constexpr const qint16 port = 8888;          // 下位机端口
}
namespace Color_Threshold_HSV {
constexpr const char *hsvConfigPath = "../HSVConfig";
constexpr const int minNum          = 0;
constexpr const int maxNum          = 255;

// 默认颜色阈值<颜色顺序：红、绿、蓝、黄、橙、紫>
constexpr const int H_min_array[] = {0, 35, 100, 20, 11, 0};
constexpr const int H_max_array[] = {10, 70, 130, 30, 25, 180};
constexpr const int S_min_array[] = {100, 100, 10, 100, 100, 30};
constexpr const int S_max_array[] = {255, 255, 255, 255, 255, 220};
constexpr const int V_min_array[] = {100, 100, 100, 100, 100, 255};
constexpr const int V_max_array[] = {255, 255, 255, 255, 255, 255};
}
namespace Calculate_Rota_Angle {
constexpr const int ROTA_FREQ = 500; // 相机旋转信号发送频率(ms)
}

}

#endif // CONFIG_H
