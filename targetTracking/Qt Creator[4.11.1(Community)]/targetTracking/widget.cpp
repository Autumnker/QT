#include "widget.h"
#include "ui_widget.h"
#include "config.h"
#include <QTimer>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget) {
    ui->setupUi(this);

    // 设置窗口名称
    this->setWindowTitle(Config::Widget::PROJ_NAME);

    // 创建颜色阈值调节对象指针
    _colorThresholdHSV_ptr = new Color_Threshold_HSV(this);

    // 创建按控制节对象指针
    _button_ctrl_ptr = new Button_Ctrl(this);

    // 创建获取网络视频流帧对象指针
    _net_work_frame_ptr = new NetWork_Frame(this);

    // 创建计算角度类对象
    _calculate_rota_angle_ptr = new Calculate_Rota_Angle(_button_ctrl_ptr, _colorThresholdHSV_ptr, this);

    // 建立信号/槽函连接
    connect_Signal_Slots();

    // 创建定时器
    QTimer *timer = new QTimer(this);
    timer->setInterval(Config::Calculate_Rota_Angle::ROTA_FREQ);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (_button_ctrl_ptr->isDebug()) {  // 调试模式手动发送旋转信号,不需要定时发送(发送信号逻辑可优化得更为平滑)
            return;
        }
        _calculate_rota_angle_ptr->calculateRotaAngle(_net_work_frame_ptr->getCurrentImage());
    });
    timer->start();
}

Widget::~Widget() {
    delete ui;
}

Ui::Widget *Widget::getUI_Widget() {
    return ui;
}

void Widget::connect_Signal_Slots() {
    // 下位机IP和Port选择下拉菜单变动信号与相机ip和port获取槽函数
    connect(_button_ctrl_ptr, &Button_Ctrl::send_ip_port, _net_work_frame_ptr, &NetWork_Frame::onReceiveIpPort);
    // 连接到下位机的颜色提示
    connect(_net_work_frame_ptr, &NetWork_Frame::connectionStatusChanged, _button_ctrl_ptr, &Button_Ctrl::changeConnectionStatusColor);
    // 按钮关闭视频流
    connect(_button_ctrl_ptr, &Button_Ctrl::send_tracking_status, _net_work_frame_ptr, &NetWork_Frame::onReceiveTrackingState);
    // 连接发送旋转角信号与向下位机发送旋转角槽函数
    connect(_button_ctrl_ptr, &Button_Ctrl::send_x_y_rota_angle, _net_work_frame_ptr, &NetWork_Frame::onReceiveXangleYangle);
    connect(_calculate_rota_angle_ptr, &Calculate_Rota_Angle::sendRotaAngle, _net_work_frame_ptr, &NetWork_Frame::onReceiveXangleYangle);
    // 连接计算出来的旋转方向信号和展示出来的信号
    connect(_calculate_rota_angle_ptr, &Calculate_Rota_Angle::sendRotaAngle, _button_ctrl_ptr, &Button_Ctrl::onReceiveXAndY);
}
