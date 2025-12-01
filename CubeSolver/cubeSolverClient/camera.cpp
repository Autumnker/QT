#include <QDebug>
#include "camera.h"
#include "config.h"

Camera::Camera(QObject *parent) : QObject(parent)
{

}

Camera::Camera(int index, QObject *parent) : QObject(parent)
{
    m_cameraIndex = index;
}

Camera::~Camera()
{
    if(m_camera.isOpened()){
        m_camera.release(); // 释放摄像头
    }
}

bool Camera::openCamera()
{
    if(m_cameraIndex >= 0){
        // 使用DirectShow后端，设置分辨率和帧率
        m_camera.open(m_cameraIndex/*, cv::CAP_DSHOW*/);
        if(m_camera.isOpened()){
            m_camera.set(cv::CAP_PROP_FRAME_WIDTH, 960);
            m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, 540);
            m_camera.set(cv::CAP_PROP_FPS, 30);
            return true;
        }
    }
    qDebug() << "无法打开相机" << m_cameraIndex;
    return false;
}

// 关闭相机
void Camera::closeCamera()
{
    if(m_camera.isOpened()){
        m_camera.release();
    }
}

bool Camera::updateFrame()
{
    if(m_camera.isOpened()){
        bool statue = m_camera.read(m_frame);
        return statue;
    }

    return false;
}

void Camera::FrameToQPixmap()
{
    // 更新帧
    updateFrame();

    if(!m_frame.empty()){
        // 将OpenCV Mat转换为QImage
        QImage qimg(m_frame.data, m_frame.cols, m_frame.rows, m_frame.step, QImage::Format_RGB888);
        m_qPixmap = QPixmap::fromImage(qimg.rgbSwapped());
    }

    // 为了兼容调试模式，无论相机是否被打开都要发送
    emit sendFrame(m_cameraIndex, m_frame);
    emit sendQpixmap(m_cameraIndex, m_qPixmap);
}

void Camera::setCameraIndex(int index)
{
    m_cameraIndex = index;
}

int Camera::getCameraIndex()
{
    return m_cameraIndex;
}

void Camera::setFrame(cv::Mat& frame)
{
    m_frame = frame;
}

cv::Mat Camera::getFrame()
{
    return m_frame;
}

void Camera::setQPixmap(QPixmap &qPixmap)
{
    m_qPixmap = qPixmap;
}

QPixmap Camera::getQPixmap()
{
    return m_qPixmap;
}

void Camera::setM_Camera(cv::VideoCapture videoCapture)
{
    m_camera = videoCapture;
}

cv::VideoCapture Camera::getM_Camera()
{
    return m_camera;
}
