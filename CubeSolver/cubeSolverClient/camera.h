#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QPixmap>

class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = nullptr);
    explicit Camera(int index = -1, QObject *parent = nullptr);
    ~Camera();

    // 打开相机
    bool openCamera();

    // 关闭相机
    void closeCamera();

    // 更新原始帧
    bool updateFrame();

    // 将原始帧转化为QPixmap
    void FrameToQPixmap();

    // setter和getter函数
    void setCameraIndex(int index);
    int getCameraIndex();
    void setFrame(cv::Mat& frame);
    cv::Mat getFrame();
    void setQPixmap(QPixmap& qPixmap);
    QPixmap getQPixmap();
    void setM_Camera(cv::VideoCapture videoCapture);
    cv::VideoCapture getM_Camera();

signals:
    void sendFrame(const int &cameraIndex, const cv::Mat &frame);
    void sendQpixmap(int cameraIndex, QPixmap qPixmap);


private:
    int m_cameraIndex = -1;
    cv::VideoCapture m_camera;
    cv::Mat m_frame;
    QPixmap m_qPixmap;

};

#endif // CAMERA_H
