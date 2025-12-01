#include "algorithm.h"
#include "config.h"

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

Algorithm::Algorithm(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &Algorithm::receiveSolvingString);
}

// 接收完整的色块序列
void Algorithm::receiveColorString(const QString wholeColorString)
{
    setWholeColorString(wholeColorString);
}

// 向服务器发送色块序列
void Algorithm::sendColorStringToServer()
{
    QUrl serverUrl(Config::solverServerUrl + m_wholeColorString);
    QNetworkRequest request(serverUrl);

    // 发起GET请求
    networkManager->get(request);
    qDebug() << "发送请求:" << serverUrl.toString();
}

// 接收求解序列
void Algorithm::receiveSolvingString(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        // 解析JSON响应
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObj = jsonDoc.object();

        if (jsonObj["status"].toString() == "success") {
            QString solution = jsonObj["solution"].toString();
            setSolvingString(solution);
            emit sendSolvingString(solution);  // 发射信号
//            qDebug() << "求解序列: " << solution;
        } else {
            qWarning() << "错误响应: " << jsonObj["error"].toString();
        }

        reply->deleteLater();
}

// 接受到完整的颜色字符串后返回求解序列
void Algorithm::returnSolvingString(const QString wholeColorString)
{
    // 获得完整色块序列
    receiveColorString(wholeColorString);

    // 将完整色块序列发送给服务器
    sendColorStringToServer();

    // 接收服务器响应(由构造函数中的信号槽机制自动实现)
    // 发送求解序列(在槽函数receiveSolvingString中已经实现)
}

// setter和getter函数
void Algorithm::setWholeColorString(QString wholeColorString)
{
    m_wholeColorString = wholeColorString;
}

QString Algorithm::getWholeColorString()
{
    return m_wholeColorString;
}

void Algorithm::setSolvingString(QString solvingString)
{
    m_solvingString = solvingString;
}

QString Algorithm::getSolvingString()
{
    return m_solvingString;
}
