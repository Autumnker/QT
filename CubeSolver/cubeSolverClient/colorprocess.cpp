#include "colorprocess.h"
#include <QDebug>

using namespace Config;

ColorProcess::ColorProcess(QObject *parent) : QObject(parent)
{
    // 使用 Config 命名空间中的值初始化颜色阈值
    for (auto it = Config::colorThresholds.begin(); it != Config::colorThresholds.end(); ++it) {
        int colorIndex = it.key();
        const Config::Threshold& threshold = it.value();
        m_HLowerArray[colorIndex] = threshold.hLower;
        m_HUpperArray[colorIndex] = threshold.hUpper;
        m_SLowerArray[colorIndex] = threshold.sLower;
        m_SUpperArray[colorIndex] = threshold.sUpper;
        m_VLowerArray[colorIndex] = threshold.vLower;
        m_VUpperArray[colorIndex] = threshold.vUpper;
    }
}

ColorProcess::ColorProcess(int index, QObject *parent) : QObject(parent)
{
    // 初始化处理函数的相机编号
    m_cameraIndex = index;

    // 使用 Config 命名空间中的值初始化颜色阈值
    for (auto it = Config::colorThresholds.begin(); it != Config::colorThresholds.end(); ++it) {
        int colorIndex = it.key();
        const Config::Threshold& threshold = it.value();
        m_HLowerArray[colorIndex] = threshold.hLower;
        m_HUpperArray[colorIndex] = threshold.hUpper;
        m_SLowerArray[colorIndex] = threshold.sLower;
        m_SUpperArray[colorIndex] = threshold.sUpper;
        m_VLowerArray[colorIndex] = threshold.vLower;
        m_VUpperArray[colorIndex] = threshold.vUpper;
    }
}

void ColorProcess::convertBGR2HSV(const int &cameraIndex, const cv::Mat& BGRframe)
{
    if(!BGRframe.isContinuous() || BGRframe.empty())
        return;

    cv::cvtColor(BGRframe, m_hsvFrame, cv::COLOR_BGR2HSV);
    m_cameraIndex = cameraIndex;

    // 发送灰度图信号
    emit sendHSVFrame(m_hsvFrame);
}

char ColorProcess::recognizeColorInCircle(const QPoint& center)
{
    int radius = Config::CircleRadius;
    std::vector<int> count(Config::colorThresholds.size(), 0);

    // 遍历圆形区域内的所有像素
    for (int y = std::max(0, center.y() - radius); y < std::min(m_hsvFrame.rows, center.y() + radius + 1); ++y) {
        for (int x = std::max(0, center.x() - radius); x < std::min(m_hsvFrame.cols, center.x() + radius + 1); ++x) {
            if (std::pow(x - center.x(), 2) + std::pow(y - center.y(), 2) <= std::pow(radius, 2)) {
                cv::Vec3b hsv = m_hsvFrame.at<cv::Vec3b>(y, x);
                int h = hsv[0];
                int s = hsv[1];
                int v = hsv[2];

                // 检查每个颜色阈值
                for (auto it = Config::colorThresholds.begin(); it != Config::colorThresholds.end(); ++it) {
                    int colorIndex = it.key();
                    if (h >= m_HLowerArray[colorIndex] && h <= m_HUpperArray[colorIndex] &&
                        s >= m_SLowerArray[colorIndex] && s <= m_SUpperArray[colorIndex] &&
                        v >= m_VLowerArray[colorIndex] && v <= m_VUpperArray[colorIndex]) {
                        ++count[colorIndex];
                        break;
                    }
                }
            }
        }
    }

    // 找到出现次数最多的颜色
    int maxCount = 0;
    int maxIndex = 0;
    for (int i = 0; i < Config::colorThresholds.size(); ++i) {
        if (count[i] > maxCount) {
            maxCount = count[i];
            maxIndex = i;
        }
    }

    // 根据颜色索引返回对应的颜色字符
    return Config::colorChars[maxIndex];
}

void ColorProcess::colorRecognition(const QVector<QVector<QPoint>> &marks)
{
    if(marks.isEmpty())
        return;

    m_colorString = ""; // 清空字符串

    if (m_cameraIndex == Config::U || m_cameraIndex == Config::D || m_cameraIndex == Config::BR || m_cameraIndex == Config::LF) {

        for (const auto& row : marks) {
            for (const auto& point : row) {
                // 获取当前圆形区域内的颜色
                char colorChar = recognizeColorInCircle(point);
                m_colorString.append(colorChar);
            }
        }

        // 获取按照正确几何关系旋转过后的字符串
        roateColorString(m_cameraIndex, m_colorString, m_roatedColorString);

        // 发送当前相机编号与颜色字符串
        emit sendColorString(m_cameraIndex, m_roatedColorString);

    } else {
        qDebug() << "colorProcess--->colorRecognition--->相机编号不正确";
    }
}

// 旋转字符串，使其符合魔方的空间几何关系
void ColorProcess::roateColorString(int cameraIndex, QString &src, QString &dest)
{
    m_roatedColorString = "";   // 清空字符串

    if(cameraIndex == Config::U){
        dest = src;

    }else if(cameraIndex == Config::D){
        dest.append(src[2]).append(src[5]).append(src[8]).append(src[1]).append(src[4]).append(src[7]).append(src[0]).append(src[3]).append(src[6]);

    }else if(cameraIndex == Config::LF){
        int LF_len = src.length();
        QString L_string = src.left(LF_len/2);
        QString F_string = src.mid(LF_len/2);
        QString L_roated;
        QString F_roated;
        L_roated.append(L_string[2]).append(L_string[5]).append(L_string[8]).append(L_string[1]).append(L_string[4]).append(L_string[7]).append(L_string[0]).append(L_string[3]).append(L_string[6]);
        F_roated.append(F_string[2]).append(F_string[5]).append(F_string[8]).append(F_string[1]).append(F_string[4]).append(F_string[7]).append(F_string[0]).append(F_string[3]).append(F_string[6]);
        dest = L_roated + F_roated;

    }else if(cameraIndex == Config::BR){
        int BR_len = src.length();
        QString B_string = src.left(BR_len/2);
        QString R_string = src.mid(BR_len/2);
        QString B_roated;
        QString R_roated;
        B_roated.append(B_string[6]).append(B_string[3]).append(B_string[0]).append(B_string[7]).append(B_string[4]).append(B_string[1]).append(B_string[8]).append(B_string[5]).append(B_string[2]);
        R_roated.append(R_string[6]).append(R_string[3]).append(R_string[0]).append(R_string[7]).append(R_string[4]).append(R_string[1]).append(R_string[8]).append(R_string[5]).append(R_string[2]);
        dest = B_roated + R_roated;
    }
}

int ColorProcess::getCameraIndex()
{
    return m_cameraIndex;
}

void ColorProcess::setCameraIndex(int cameraIndex)
{
    m_cameraIndex = cameraIndex;
}

cv::Mat ColorProcess::getHSVframe()
{
    return m_hsvFrame;
}

void ColorProcess::setHSVframe(const cv::Mat &frame)
{
    m_hsvFrame = frame;
}

QString ColorProcess::getColorString()
{
    return m_colorString;
}

void ColorProcess::setColorString(QString colorString)
{
    m_colorString = colorString;
}

QString ColorProcess::getRoatedColorString()
{
    return m_roatedColorString;
}

void ColorProcess::setRoatedColorString(QString colorString)
{
    m_roatedColorString = colorString;
}

QHash<int, int> ColorProcess::getHUpperThreshold()
{
    return m_HUpperArray;
}

QHash<int, int> ColorProcess::getSUpperThreshold()
{
    return m_SUpperArray;
}

QHash<int, int> ColorProcess::getVUpperThreshold()
{
    return m_VUpperArray;
}

void ColorProcess::setHUpperThreshold(QHash<int, int> &hUpperThreshold)
{
    m_HUpperArray = hUpperThreshold;
}

void ColorProcess::setSUpperThreshold(QHash<int, int> &sUpperThreshold)
{
    m_SUpperArray = sUpperThreshold;
}

void ColorProcess::setVUpperThreshold(QHash<int, int> &vUpperThreshold)
{
    m_VUpperArray = vUpperThreshold;
}

QHash<int, int> ColorProcess::getHLowerThreshold()
{
    return m_HLowerArray;
}

QHash<int, int> ColorProcess::getSLowerThreshold()
{
    return m_SLowerArray;
}

QHash<int, int> ColorProcess::getVLowerThreshold()
{
    return m_VLowerArray;
}

void ColorProcess::setHLowerThreshold(QHash<int, int> &hLowerThreshold)
{
    m_HLowerArray = hLowerThreshold;
}

void ColorProcess::setSLowerThreshold(QHash<int, int> &sLowerThreshold)
{
    m_SLowerArray = sLowerThreshold;
}

void ColorProcess::setVLowerThreshold(QHash<int, int> &vLowerThreshold)
{
    m_VLowerArray = vLowerThreshold;
}
