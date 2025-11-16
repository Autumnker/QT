// calculate_rota_angle.cpp
#include "calculate_rota_angle.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

Calculate_Rota_Angle::Calculate_Rota_Angle(Button_Ctrl *button_ctrl,
                                           Color_Threshold_HSV *colorThreshold,
                                           QObject *parent)
    : QObject(parent)
    , _buttonCtrl(button_ctrl)
    , _colorThreshold(colorThreshold) {
}

QPair<int, int> Calculate_Rota_Angle::calcRotaAngle(QImage &image) {
    if (image.isNull()) {
        return QPair<int, int>(0, 0);
        qDebug() << "当前 image 为空";
    }

    if (!_buttonCtrl || !_colorThreshold) {
        qDebug() << "Button controller or color threshold not set";
        return QPair<int, int>(0, 0);
    }

    // 1. 获取当前选择的颜色
    QString currentColor = _buttonCtrl->getTracking_Color_Name();

    qDebug() << "currentColor = " << currentColor;

    if (currentColor.isEmpty()) {
        emit trackingInfoUpdated("No color selected");
        return QPair<int, int>(0, 0);
    }

    // 2. 将QImage转换为OpenCV Mat
    cv::Mat cvImage;
    if (image.format() == QImage::Format_RGB32) {
        cvImage = cv::Mat(image.height(), image.width(), CV_8UC4,
                          (void *)image.constBits(), image.bytesPerLine());
        cv::cvtColor(cvImage, cvImage, cv::COLOR_RGBA2BGR);
    } else {
        QImage converted = image.convertToFormat(QImage::Format_RGB32);
        cvImage          = cv::Mat(converted.height(), converted.width(), CV_8UC4,
                          (void *)converted.constBits(), converted.bytesPerLine());
        cv::cvtColor(cvImage, cvImage, cv::COLOR_RGBA2BGR);
    }

    // 3. 转换为HSV颜色空间
    cv::Mat hsvImage;
    cv::cvtColor(cvImage, hsvImage, cv::COLOR_BGR2HSV);

    // 4. 创建颜色掩码
    cv::Mat mask = createMask(hsvImage, currentColor);
    if (mask.empty()) {
        emit trackingInfoUpdated(QString("Failed to create mask for color: %1").arg(currentColor));
        return QPair<int, int>(0, 0);
    }

    // 5. 形态学操作
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // 6. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        emit trackingInfoUpdated(QString("No %1 objects detected").arg(currentColor));
        return QPair<int, int>(0, 0);
    }

    // 7. 找到最佳目标
    cv::Point targetCenter;
    double targetArea;
    if (!findBestTarget(contours, targetCenter, targetArea)) {
        emit trackingInfoUpdated(QString("No valid %1 target found").arg(currentColor));
        return QPair<int, int>(0, 0);
    }

    // 8. 计算需要旋转的角度
    QPair<int, int> angles = calculateAngles(targetCenter, cvImage.size());

    QString info = QString("%1 target: Area=%2, Center=(%3,%4), Angles=(%5,%6)")
                       .arg(currentColor)
                       .arg(targetArea)
                       .arg(targetCenter.x)
                       .arg(targetCenter.y)
                       .arg(angles.first)
                       .arg(angles.second);

    emit trackingInfoUpdated(info);
    return angles;
}

void Calculate_Rota_Angle::calculateRotaAngle(QImage &image) {
    QPair<int, int> angles = calcRotaAngle(image);
    emit sendRotaAngle(angles.first, angles.second);
}

cv::Mat Calculate_Rota_Angle::createMask(const cv::Mat &hsvImage, const QString &colorName) {
    // 获取颜色索引
    QVector<QString> colors = _colorThreshold->getColorArray();
    int colorIndex          = colors.indexOf(colorName);
    if (colorIndex == -1) {
        qDebug() << "Color not found:" << colorName;
        return cv::Mat();
    }

    // 获取HSV阈值
    QVector<QPair<int, int>> H_values = _colorThreshold->getHpairArray();
    QVector<QPair<int, int>> S_values = _colorThreshold->getSpairArray();
    QVector<QPair<int, int>> V_values = _colorThreshold->getvpairArray();

    int H_min = H_values[colorIndex].first;
    int H_max = H_values[colorIndex].second;
    int S_min = S_values[colorIndex].first;
    int S_max = S_values[colorIndex].second;
    int V_min = V_values[colorIndex].first;
    int V_max = V_values[colorIndex].second;

    // 处理红色特殊情况（在HSV色环中红色位于两端）
    if (H_min > H_max) {
        cv::Mat mask1, mask2;

        // 第一种范围 (0到H_max)
        cv::inRange(hsvImage,
                    cv::Scalar(0, S_min, V_min),
                    cv::Scalar(H_max, S_max, V_max),
                    mask1);

        // 第二种范围 (H_min到180)
        cv::inRange(hsvImage,
                    cv::Scalar(H_min, S_min, V_min),
                    cv::Scalar(180, S_max, V_max),
                    mask2);

        // 合并两个掩码
        cv::Mat mask;
        cv::bitwise_or(mask1, mask2, mask);
        return mask;
    } else {
        // 常规颜色范围
        cv::Mat mask;
        cv::inRange(hsvImage,
                    cv::Scalar(H_min, S_min, V_min),
                    cv::Scalar(H_max, S_max, V_max),
                    mask);
        return mask;
    }
}

bool Calculate_Rota_Angle::findBestTarget(const std::vector<std::vector<cv::Point>> &contours,
                                          cv::Point &center, double &area) {
    if (contours.empty()) return false;

    // 找到面积最大的轮廓
    double maxArea = 0;
    int maxIndex   = -1;

    for (size_t i = 0; i < contours.size(); i++) {
        double contourArea = cv::contourArea(contours[i]);
        if (contourArea > maxArea) {
            maxArea  = contourArea;
            maxIndex = i;
        }
    }

    if (maxIndex == -1) return false;

    // 计算轮廓的中心
    cv::Moments moments = cv::moments(contours[maxIndex]);
    if (moments.m00 == 0) return false;

    center.x = static_cast<int>(moments.m10 / moments.m00);
    center.y = static_cast<int>(moments.m01 / moments.m00);
    area     = maxArea;

    // 画一根从相机中心到目标重心的线

    return true;
}

QPair<int, int> Calculate_Rota_Angle::calculateAngles(const cv::Point &targetCenter,
                                                      const cv::Size &imageSize) {
    // 计算图像中心
    int imageCenterX = imageSize.width / 2;
    int imageCenterY = imageSize.height / 2;

    // 计算偏差（像素单位）
    int deltaX = targetCenter.x - imageCenterX;
    int deltaY = targetCenter.y - imageCenterY;

    // 将像素偏差映射到旋转角度
    // 假设最大偏差对应最大角度（±90度）
    double xAngle = -static_cast<double>(deltaX) / imageCenterX * 90.0;
    double yAngle = static_cast<double>(deltaY) / imageCenterY * 90.0;

    // 限制角度在-90到90度之间
    xAngle = std::max(-90.0, std::min(90.0, xAngle));
    yAngle = std::max(-90.0, std::min(90.0, yAngle));

    return QPair<int, int>(static_cast<int>(std::round(xAngle)),
                           static_cast<int>(std::round(yAngle)));
}
