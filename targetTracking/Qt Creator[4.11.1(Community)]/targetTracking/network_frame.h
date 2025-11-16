#ifndef FRAME_H
#define FRAME_H

#include <QObject>
#include <QImage>
#include <QTcpSocket>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "config.h"

/**
* @class NetWork_Frame
* @brief 处理从网络相机中传输过来的原始帧,将其转化为 QGraphicsView 可显示的方式
* 改类负责：
* - 接收网络相机的原始数据帧
* - 将原始数据帧转化为qt可识别的图象格式QPixmap
* - 提供接口供 QGraphicsView 显示
*/
class NetWork_Frame : public QObject {
    Q_OBJECT
public:
    explicit NetWork_Frame(QObject *parent = nullptr);
    ~NetWork_Frame();

    /**
    * @brief 连接下位机并接收视频流
    * @param ip下位机ip
    * @param port 下位机端口
    */
    void startStreaming(QString ip = Config::NetWork_Fram::ip, qint16 port = Config::NetWork_Fram::port);
    /** @brief 停止视频流*/
    void stopStreaming();
    /** @brief 获取当前图片帧*/
    QImage &getCurrentImage();

public slots:
    /** @brief 检查当前连接状态*/
    bool isConnected() const;
    /** @brief 获取当前连接状态字符串*/
    QString connectionState() const;
    /** @brief 接收信号中携带的ip和port*/
    void onReceiveIpPort(QString ip, qint16 port);
    /** @brief 接收信号中携带的tracking状态*/
    void onReceiveTrackingState(bool tracking);
    /** @brief 接收信号中携带的x旋转角度,y旋转角度并发送给下位机*/
    void onReceiveXangleYangle(int x_rota_angle, int y_rota_angle);

private slots:
    /** @brief 从缓冲区中读取数据*/
    void readVideoData();
    /** @brief 将缓冲区中数据转换成QPixmap*/
    void processJPEGFrame(const QByteArray &jpegData);
    /** @brief 连接建立时的槽函数*/
    void onConnected();
    /** @brief 连接断开时的槽函数*/
    void onDisconnected();
    /** @brief socket错误处理*/
    void onSocketError(QAbstractSocket::SocketError error);
    /** @brief 状态变化处理*/
    void onStateChanged(QAbstractSocket::SocketState state);

signals:
    void connectionStatusChanged(bool connected);
    void connectionError(const QString &errorString);
    void socketStateChanged(const QString &state);
    void frameReceived(QImage &image);

private:
    QTcpSocket *_tcpSocket;           // tcp套接字
    QGraphicsScene *_scene;           // 场景指针
    QGraphicsView *_view;             // 视图指针
    QGraphicsPixmapItem *_pixmapItem; // 像素图片指针
    QImage _image;                    // 当前图片
    QByteArray _buffer;               // 接收消息缓冲区
    QString _currentIp;               // 当前ip
    qint16 _currentPort;              // 当前port
};

#endif // FRAME_H
