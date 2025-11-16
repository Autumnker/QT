#include "network_frame.h"
#include "ui_widget.h"
#include "widget.h"

NetWork_Frame::NetWork_Frame(QObject *parent)
    : QObject(parent) {
    // 初始化 TCP 客户端
    _tcpSocket = new QTcpSocket(this);
    // 连接相关信号
    connect(_tcpSocket, &QTcpSocket::connected, this, &NetWork_Frame::onConnected);
    connect(_tcpSocket, &QTcpSocket::disconnected, this, &NetWork_Frame::onDisconnected);
    connect(_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &NetWork_Frame::onSocketError);
    connect(_tcpSocket, &QTcpSocket::readyRead, this, &NetWork_Frame::readVideoData);
    connect(_tcpSocket, &QTcpSocket::stateChanged, this, &NetWork_Frame::onStateChanged);


    // 初始化 QGraphicsView
    Widget *parent_ptr = (Widget *)parent;
    _view              = parent_ptr->getUI_Widget()->GView_cameraView;
    _scene             = new QGraphicsScene(this);
    _view->setScene(_scene);
    _pixmapItem = _scene->addPixmap(QPixmap());
    _view->show();
}

NetWork_Frame::~NetWork_Frame() {
}

void NetWork_Frame::startStreaming(QString ip, qint16 port) {
    // 检查是否已经连接到相同的地址和端口
    if (_tcpSocket->state() == QAbstractSocket::ConnectedState &&
        _currentIp == ip && _currentPort == port) {
        qDebug() << "Already connected to" << ip << ":" << port;
        return;
    }

    // 如果已连接但地址不同，先断开
    if (_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        _tcpSocket->disconnectFromHost();
        if (_tcpSocket->state() == QAbstractSocket::ConnectedState) {
            _tcpSocket->waitForDisconnected(1000);
        }
    }

    // 保存当前连接信息
    _currentIp   = ip;
    _currentPort = port;

    // 连接到新主机
    _tcpSocket->connectToHost(ip, port);

    // 等待连接建立（可选，可以异步处理）
    if (_tcpSocket->waitForConnected(3000)) {
        qDebug() << "Connected to" << ip << ":" << port;
    } else {
        qDebug() << "Connection failed:" << _tcpSocket->errorString();
    }
}

void NetWork_Frame::stopStreaming() {
    if (_tcpSocket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Stopping video stream...";

        // 断开连接
        _tcpSocket->disconnectFromHost();

        // 等待断开完成
        if (_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            _tcpSocket->waitForDisconnected(1000);
        }

        // 清空缓冲区
        _buffer.clear();

        // 可选：显示断开连接的消息或图像
        QImage blankImage(_view->width(), _view->height(), QImage::Format_RGB32);
        blankImage.fill(Qt::black);
        _pixmapItem->setPixmap(QPixmap::fromImage(blankImage));

        qDebug() << "Video stream stopped";
    } else {
        qDebug() << "Not connected, no need to stop streaming";
    }

    // 重置连接信息
    _currentIp.clear();
    _currentPort = 0;
}

QImage &NetWork_Frame::getCurrentImage() {
    return _image;
}

void NetWork_Frame::onConnected() {
    qDebug() << "Connected to server";
    emit connectionStatusChanged(true);
}

void NetWork_Frame::onDisconnected() {
    qDebug() << "Disconnected from server";
    emit connectionStatusChanged(false);
}

void NetWork_Frame::onSocketError(QAbstractSocket::SocketError error) {
    qDebug() << "Socket error:" << error << " - " << _tcpSocket->errorString();
    emit connectionError(_tcpSocket->errorString());
}

void NetWork_Frame::onStateChanged(QAbstractSocket::SocketState state) {
    QString stateStr;
    switch (state) {
    case QAbstractSocket::UnconnectedState: stateStr = "Unconnected"; break;
    case QAbstractSocket::HostLookupState: stateStr = "HostLookup"; break;
    case QAbstractSocket::ConnectingState: stateStr = "Connecting"; break;
    case QAbstractSocket::ConnectedState: stateStr = "Connected"; break;
    case QAbstractSocket::BoundState: stateStr = "Bound"; break;
    case QAbstractSocket::ClosingState: stateStr = "Closing"; break;
    case QAbstractSocket::ListeningState: stateStr = "Listening"; break;
    }
    qDebug() << "Socket state changed to:" << stateStr;
    emit socketStateChanged(stateStr);
}

bool NetWork_Frame::isConnected() const {
    return _tcpSocket->state() == QAbstractSocket::ConnectedState;
}

QString NetWork_Frame::connectionState() const {
    return _tcpSocket->state() == QAbstractSocket::ConnectedState ? "Connected" : "Disconnected";
}

void NetWork_Frame::onReceiveIpPort(QString ip, qint16 port) {
    startStreaming(ip, port);
}

void NetWork_Frame::onReceiveTrackingState(bool tracking) {
    if (!tracking) {
        stopStreaming();
        return;
    }
    // 保留
}

void NetWork_Frame::onReceiveXangleYangle(int x_rota_angle, int y_rota_angle) {
    // 检查TCP连接状态
    if (_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Cannot send command: not connected to ESP32-CAM";
        return;
    }

    // 格式化命令字符串: "X角度,Y角度\n"
    QString command = QString("%1,%2\n").arg(x_rota_angle).arg(y_rota_angle);

    // 转换为UTF-8字节数组
    QByteArray data = command.toUtf8();

    // 发送命令
    qint64 bytesWritten = _tcpSocket->write(data);

    if (bytesWritten == -1) {
        qDebug() << "Failed to send command:" << _tcpSocket->errorString();
    } else if (bytesWritten < data.size()) {
        qDebug() << "Command partially sent:" << bytesWritten << "of" << data.size() << "bytes";
    } else {
        qDebug() << "Command sent successfully:" << command.trimmed();
    }

    // 确保数据被立即发送
    _tcpSocket->flush();
}

void NetWork_Frame::readVideoData() {
    _buffer += _tcpSocket->readAll();
    int startIdx = _buffer.indexOf("\xFF\xD8");
    int endIdx   = _buffer.indexOf("\xFF\xD9", startIdx);

    while (startIdx != -1 && endIdx != -1) {
        QByteArray jpegData = _buffer.mid(startIdx, endIdx - startIdx + 2);
        processJPEGFrame(jpegData);
        _buffer.remove(0, endIdx + 2);
        startIdx = _buffer.indexOf("\xFF\xD8");
        endIdx   = _buffer.indexOf("\xFF\xD9", startIdx);
    }
}

void NetWork_Frame::processJPEGFrame(const QByteArray &jpegData) {
    QImage image;
    if (image.loadFromData(jpegData, "JPEG")) {
        // 同时进行上下和左右翻转
        image = image.mirrored(true, true);
        _pixmapItem->setPixmap(QPixmap::fromImage(image));
        // 自动适应 QGraphicsView 大小，保持宽高比
        _view->fitInView(_pixmapItem, Qt::KeepAspectRatio);

        _image = image;
        // 发送图片信号
        emit frameReceived(image);
    }
}
