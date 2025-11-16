#include "button_ctrl.h"
#include <QColorDialog>
#include "ui_widget.h"
#include "widget.h"
#include "config.h"
#include "color_select_dialog.h"

Button_Ctrl::Button_Ctrl(QWidget *parent)
    : QWidget(parent) {
    Widget *parent_ptr = (Widget *)parent; // 获取父指针

    /** 组件指针初始化*/
    _connection_status          = parent_ptr->getUI_Widget()->Combox_connection_status;
    _color_of_connection_status = parent_ptr->getUI_Widget()->label_connection_status;
    _x_rota_angle               = parent_ptr->getUI_Widget()->LEdit_x_rota_angle;
    _y_rota_angle               = parent_ptr->getUI_Widget()->LEdit_y_rota_angle;
    _time_consuming             = parent_ptr->getUI_Widget()->Label_time_consuming;
    _select_recognition         = parent_ptr->getUI_Widget()->Combox_select_recognition;
    _Debug                      = parent_ptr->getUI_Widget()->Btn_Debug;
    _rotation_angle             = parent_ptr->getUI_Widget()->Btn_rotation_angle;
    _tracking                   = parent_ptr->getUI_Widget()->Btn_tracking;

    /** 初始化成员变量*/
    // 初始化连接状态中的ip和端口为配置文件中的默认值
    _ip   = Config::NetWork_Fram::ip;
    _port = Config::NetWork_Fram::port;
    _connection_status->addItem(QString("%1 | %2").arg(_ip).arg(_port));
    // 初始化连接状态图标颜色
    _color_of_connection_status->setStyleSheet(Config::Button_Ctrl::disconnected_color);
    // 开启/停止跟踪按钮
    _Btn_Track_Mode[Tracking]     = "停止跟踪";
    _Btn_Track_Mode[StopTracking] = "开启跟踪";
    // 初始状态下,发送旋转角度为不可用状态
    _rotation_angle->setDisabled(true);
    // 设置默认要追踪的颜色
    _tracking_color = QColor(Config::Button_Ctrl::DEFAULT_CHOICED_COLOR);
    // 识别模式字符串容器
    QString Rec_Mode_String[Total_Recognition_Mode] = {"颜色追踪", "YOLOV5 目标追踪"};
    for (int i = 0; i < Total_Recognition_Mode; ++i) {
        _Recognition_Mode_Str.push_back(Rec_Mode_String[i]);
    }
    // 将识别模式显示在下拉菜单中
    _select_recognition->addItem(Rec_Mode_String[Color_Recognition]);
    _select_recognition->addItem(Rec_Mode_String[YOLOV5_Recognition]);
    // 初始化舵机转角的限制
    _rota_angle_limit.first.first   = Config::Button_Ctrl::x_min_angle;
    _rota_angle_limit.first.second  = Config::Button_Ctrl::x_max_angle;
    _rota_angle_limit.second.first  = Config::Button_Ctrl::y_min_angle;
    _rota_angle_limit.second.second = Config::Button_Ctrl::y_max_angle;

    /** 连接信号与槽函数*/
    bindSignal_Slots();
}

Button_Ctrl::~Button_Ctrl() {
}

void Button_Ctrl::bindSignal_Slots() {
    // 把连接信息显示在下拉菜单中<需要网络模块>
    connect(_connection_status, &QComboBox::currentTextChanged, this, [this](QString ip_port) {
        QStringList list = ip_port.split('|', Qt::SkipEmptyParts);
        if (list.size() >= 2) {
            _ip   = list[0].trimmed();
            _port = list[1].trimmed().toShort();
        }
        emit send_ip_port(_ip, _port);
    });

    // 把舵机旋转角度显示在行编辑器中<需要计算模块>

    // 从行编辑器中获取输入值
    connect(_x_rota_angle, &QLineEdit::editingFinished, this, [this]() {
        _x_rota_angle_val = _x_rota_angle->text().toInt();
        // 检查 x 方向转角是否在范围内,否则取整
        int x_min = _rota_angle_limit.first.first;
        int x_max = _rota_angle_limit.first.second;

        _x_rota_angle_val = _x_rota_angle_val < x_min ? x_min : _x_rota_angle_val;
        _x_rota_angle_val = _x_rota_angle_val > x_max ? x_max : _x_rota_angle_val;

        _x_rota_angle->setText(QString::number(_x_rota_angle_val));
    });
    connect(_y_rota_angle, &QLineEdit::editingFinished, this, [this]() {
        _y_rota_angle_val = _y_rota_angle->text().toInt();
        // 检查 y 方向转角是否在范围内,否则取整
        int y_min = _rota_angle_limit.second.first;
        int y_max = _rota_angle_limit.second.second;

        _y_rota_angle_val = _y_rota_angle_val < y_min ? y_min : _y_rota_angle_val;
        _y_rota_angle_val = _y_rota_angle_val > y_max ? y_max : _y_rota_angle_val;

        _y_rota_angle->setText(QString::number(_y_rota_angle_val));
    });

    // 将识别模式的选择索引赋值给成员变量"recognition_mode_val、tracking_color"
    // 将识别模式的选择索引赋值给成员变量"recognition_mode_val、tracking_color"
    connect(_select_recognition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        _recognition_mode_val = index; // 将当前使用的追踪方法的索引赋值给成员变量

        if (index == Color_Recognition) {
            // 创建自定义颜色选择对话框
            Color_Select_Dialog colorDialog(this);

            // 显示对话框并等待用户选择
            if (colorDialog.exec() == QDialog::Accepted) {
                _tracking_color_name = colorDialog.getSelectedColorName();

                if (!_tracking_color_name.isEmpty()) {
                    // 根据颜色名称设置对应的QColor
                    QColor selectedColor;
                    if (_tracking_color_name == "Red")
                        selectedColor = Qt::red;
                    else if (_tracking_color_name == "Green")
                        selectedColor = Qt::green;
                    else if (_tracking_color_name == "Blue")
                        selectedColor = Qt::blue;
                    else if (_tracking_color_name == "Yellow")
                        selectedColor = Qt::yellow;
                    else if (_tracking_color_name == "Orange")
                        selectedColor = QColor(255, 165, 0);
                    else if (_tracking_color_name == "Purple")
                        selectedColor = QColor(128, 0, 128);

                    _tracking_color = selectedColor; // 将追踪的颜色改为刚刚选择的颜色
                    _select_recognition->setItemText(index, _Recognition_Mode_Str[Color_Recognition] + ": " + _tracking_color_name);
                }
            }
        }
    });

    // 当开启调试模式时,发送旋转角可用,开启跟踪不可用
    // 当关闭调试模式时,发送旋转角不可用,开启跟踪可用
    connect(_Debug, &QPushButton::toggled, this, [=](bool isDebug) {
        _rotation_angle->setEnabled(isDebug);
        _tracking->setEnabled(!isDebug);
        _is_debug_mode_val = isDebug; // 成员变量赋值
    });

    // 发送自定义旋转角度
    connect(_rotation_angle, &QPushButton::clicked, this, [this]() {
        emit send_x_y_rota_angle(_x_rota_angle_val, _y_rota_angle_val);
    });

    // 当点击"开启跟踪"时,文本变为"停止跟踪"
    // 当点击"停止跟踪"时,文本变为"开启跟踪"
    connect(_tracking, &QPushButton::toggled, this, [=](bool isTracking) {
        // 根据按钮状态改变提示字符
        if (isTracking) {
            _tracking->setText(_Btn_Track_Mode[Tracking]);
        } else {
            _tracking->setText(_Btn_Track_Mode[StopTracking]);
        }

        _is_tracking_val = isTracking; // 将当前是否处于追踪状态赋值给成员变量
        emit send_tracking_status(_is_tracking_val);
        if (_is_tracking_val) {
            emit send_ip_port(_ip, _port);
        }
    });
}

void Button_Ctrl::changeConnectionStatusColor(bool connected) {
    if (connected) {
        _color_of_connection_status->setStyleSheet(Config::Button_Ctrl::connected_color);
    } else {
        _color_of_connection_status->setStyleSheet(Config::Button_Ctrl::disconnected_color);
    }
}

void Button_Ctrl::onReceiveXAndY(int x, int y) {
    if (isDebug()) {
        return;
    }
    _x_rota_angle_val = x;
    _y_rota_angle_val = y;
    _x_rota_angle->setText(QString::number(x));
    _y_rota_angle->setText(QString::number(y));
}

QVector<QString> Button_Ctrl::getRecognition_Mode_Str() const {
    return _Recognition_Mode_Str;
}

QVector<QString> Button_Ctrl::getConnection_status_val() const {
    return _connection_status_val;
}

int Button_Ctrl::getRecognition_mode_val() const {
    return _recognition_mode_val;
}

bool Button_Ctrl::isDebug() const {
    return _is_debug_mode_val;
}

bool Button_Ctrl::isTracking() const {
    return _is_tracking_val;
}

QPair<QPair<int, int>, QPair<int, int>> Button_Ctrl::getRota_angle_limit() const {
    return _rota_angle_limit;
}

QPair<QString, qint16> Button_Ctrl::getIP_Port() const {
    return qMakePair(_ip, _port);
}

QString Button_Ctrl::getTracking_Color_Name() const {
    return _tracking_color_name;
}

QColor Button_Ctrl::getTracking_Color() const {
    return _tracking_color;
}
