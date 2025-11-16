#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "opencv2/opencv.hpp"
#include "color_threshold_hsv.h"
#include "button_ctrl.h"
#include "network_frame.h"
#include "calculate_rota_angle.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    /** @brief 返回当前组件的ui指针*/
    Ui::Widget *getUI_Widget();

private:
    void connect_Signal_Slots();

private:
    Ui::Widget *ui;
    Color_Threshold_HSV *_colorThresholdHSV_ptr;
    Button_Ctrl *_button_ctrl_ptr;
    NetWork_Frame *_net_work_frame_ptr;
    Calculate_Rota_Angle *_calculate_rota_angle_ptr;
};
#endif // WIDGET_H
