#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class Algorithm : public QObject
{
    Q_OBJECT
public:
    explicit Algorithm(QObject *parent = nullptr);

    // 网络请求管理
    QNetworkAccessManager *networkManager;

    // 接收完整的色块序列
    void receiveColorString(const QString wholeColorString);

    // 向服务器发送色块序列
    void sendColorStringToServer();

    // 接收求解序列
    void receiveSolvingString(QNetworkReply *reply);

    // 接受到完整的颜色字符串后返回求解序列
    void returnSolvingString(const QString wholeColorString);

    // setter和getter函数
    void setWholeColorString(QString wholeColorString);
    QString getWholeColorString();
    void setSolvingString(QString solvingString);
    QString getSolvingString();

signals:
    void sendSolvingString(const QString solvingString);

private:
    QString m_wholeColorString;
    QString m_solvingString;

};

#endif // ALGORITHM_H
