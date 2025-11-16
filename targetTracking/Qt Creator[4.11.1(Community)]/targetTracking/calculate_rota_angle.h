// calculate_rota_angle.h
#ifndef CALCULATE_ROTA_ANGLE_H
#define CALCULATE_ROTA_ANGLE_H

#include <QPair>
#include <QColor>
#include <QImage>
#include <QObject>
#include "opencv2/opencv.hpp"
#include "button_ctrl.h"
#include "color_threshold_hsv.h"

/**
* @brief 根据图片识别目标/计算需要的旋转角度
*/
class Calculate_Rota_Angle : public QObject {
    Q_OBJECT
public:
    explicit Calculate_Rota_Angle(Button_Ctrl *button_ctrl, Color_Threshold_HSV *colorThreshold, QObject *parent = nullptr);

    /** @brief 根据当前选择的颜色和图片计算旋转角度*/
    QPair<int, int> calcRotaAngle(QImage &image);

private:
    Button_Ctrl *_buttonCtrl;             // 按钮管理对象
    Color_Threshold_HSV *_colorThreshold; // 颜色阈值配置对象

    // 使用HSV阈值创建掩码
    cv::Mat createMask(const cv::Mat &hsvImage, const QString &colorName);

    // 处理轮廓并找到最佳目标
    bool findBestTarget(const std::vector<std::vector<cv::Point>> &contours, cv::Point &center, double &area);

    // 计算需要旋转的角度
    QPair<int, int> calculateAngles(const cv::Point &targetCenter, const cv::Size &imageSize);

public slots:
    void calculateRotaAngle(QImage &image);

signals:
    void sendRotaAngle(int x_angle, int y_angle);
    void trackingInfoUpdated(const QString &info);
};

#endif // CALCULATE_ROTA_ANGLE_H
