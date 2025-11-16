#ifndef BUTTON_CTRL_H
#define BUTTON_CTRL_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

/**
* @brief 控制组件相关
* @todo 转角的计算时间功能未完成
*/
class Button_Ctrl : public QWidget {
    Q_OBJECT
public:
    explicit Button_Ctrl(QWidget *parent = nullptr);
    ~Button_Ctrl();

    /** @brief 获取识别模式列表*/
    QVector<QString> getRecognition_Mode_Str() const;
    /** @brief 获取连接状态列表*/
    QVector<QString> getConnection_status_val() const;
    /** @brief 获取当前识别模式索引*/
    int getRecognition_mode_val() const;
    /** @brief 判断当前是否处于调试模式*/
    bool isDebug() const;
    /** @brief 判断当前是否处于追踪模式*/
    bool isTracking() const;
    /** @brief 获取当前正在使用的 ip 和 port*/
    QPair<QString, qint16> getIP_Port() const;
    /** @brief 获取当前正在追踪的颜色*/
    QColor getTracking_Color() const;
    /** @brief 获取当前正在追踪的颜色的名字*/
    QString getTracking_Color_Name() const;
    /** @brief 获取旋转角度限制*/
    QPair<QPair<int, int>, QPair<int, int>> getRota_angle_limit() const;

private:
    /** @brief 连接信号与槽函数*/
    void bindSignal_Slots();

signals:
    /** @brief [信号]发送舵机转角*/
    void send_x_y_rota_angle(int x_angle, int y_angle);
    /** @brief [信号]发送通过Combox(复选框)选中的ip和端口号*/
    void send_ip_port(QString ip, qint16 port);
    /** @brief 发送追踪开启/关闭信息*/
    void send_tracking_status(bool yes);

public slots:
    /** @brief 根据信号改变连接状态显示label的颜色*/
    void changeConnectionStatusColor(bool connected);
    void onReceiveXAndY(int x, int y);

public:
    // 识别模式索引
    enum Recognition_Mode_Index
    {
        Color_Recognition,
        YOLOV5_Recognition,
        Total_Recognition_Mode
    };

private:
    // ip地址相关
    QString _ip;
    qint16 _port;
    // 按钮值相关
    int _x_rota_angle_val;
    int _y_rota_angle_val;
    int _time_consuming_val;

    // 识别模式字符串
    QVector<QString> _Recognition_Mode_Str;

    QVector<QString> _connection_status_val; // 连接信息
    int _recognition_mode_val;               // 识别模式
    bool _is_debug_mode_val;                 // 是否处于调试模式
    bool _is_tracking_val;                   // 是否开启跟踪
    QColor _tracking_color;                  // 要追踪的颜色
    QString _tracking_color_name;            //  要追踪的颜色的名字

    // 舵机转角阈值
    QPair<QPair<int, int>, QPair<int, int>> _rota_angle_limit; // 舵机转角阈值

    // 开启/停止跟踪
    enum Track_Mode_Index
    {
        Tracking,
        StopTracking,
        Total_Track_Mode
    };
    QString _Btn_Track_Mode[Total_Track_Mode];

    // 组件相关
    QComboBox *_connection_status;
    QLabel *_color_of_connection_status;
    QLineEdit *_x_rota_angle;
    QLineEdit *_y_rota_angle;
    QLabel *_time_consuming;
    QComboBox *_select_recognition;
    QPushButton *_Debug;
    QPushButton *_rotation_angle;
    QPushButton *_tracking;
};

#endif // BUTTON_CTRL_H
