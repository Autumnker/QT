#ifndef COLORPROCESS_H
#define COLORPROCESS_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QPoint>
#include <opencv2/opencv.hpp>

#include "config.h"

class ColorProcess : public QObject
{
    Q_OBJECT
public:
    explicit ColorProcess(QObject *parent = nullptr);
    explicit ColorProcess(int index = -1, QObject *parent = nullptr);

    // 槽函数
    // 将原始Mat转换到HSV空间
    void convertBGR2HSV(const int &cameraIndex, const cv::Mat& BGRframe);

    // 识别单个圆形中的图片颜色
    char recognizeColorInCircle(const QPoint& center);

    // 根据阈值进行颜色识别(单个相机)
    void colorRecognition(const QVector<QVector<QPoint>>& marks);

    // 旋转字符串，使其符合魔方的空间几何关系
    void roateColorString(int cameraIndex, QString &src, QString &dest);

    // getter和setter方法
    int getCameraIndex();
    void setCameraIndex(int cameraIndex);

    cv::Mat getHSVframe();
    void setHSVframe(const cv::Mat& frame);

    QString getColorString();
    void setColorString(QString colorString);

    QString getRoatedColorString();
    void setRoatedColorString(QString colorString);

    // 获取和设置单个颜色阈值
    QHash<int, int> getHUpperThreshold();
    QHash<int, int> getSUpperThreshold();
    QHash<int, int> getVUpperThreshold();
    void setHUpperThreshold(QHash<int, int>& hUpperThreshold);
    void setSUpperThreshold(QHash<int, int>& sUpperThreshold);
    void setVUpperThreshold(QHash<int, int>& vUpperThreshold);
    QHash<int, int> getHLowerThreshold();
    QHash<int, int> getSLowerThreshold();
    QHash<int, int> getVLowerThreshold();
    void setHLowerThreshold(QHash<int, int>& hLowerThreshold);
    void setSLowerThreshold(QHash<int, int>& sLowerThreshold);
    void setVLowerThreshold(QHash<int, int>& vLowerThreshold);

signals:
    void sendHSVFrame(cv::Mat hsvFrame);
    void sendColorString(const int &camIndex, const QString &colorString);
    void clearColorString();

private:
    // 相机编号
    int m_cameraIndex;

    // 转换后的Mat
    cv::Mat m_hsvFrame;

    // 色块字符串
    QString m_colorString;

    // 旋转后的色块字符串
    QString m_roatedColorString;

    // 颜色阈值<颜色编号, 阈值>
    QHash<int, int> m_HUpperArray;
    QHash<int, int> m_SUpperArray;
    QHash<int, int> m_VUpperArray;
    QHash<int, int> m_HLowerArray;
    QHash<int, int> m_SLowerArray;
    QHash<int, int> m_VLowerArray;

};

#endif // COLORPROCESS_H
