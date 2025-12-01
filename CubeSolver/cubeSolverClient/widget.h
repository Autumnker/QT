#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QHash>
#include <QTimer>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>

#include "colorprocess.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 创造相机指针实例并填充哈希表
    void createCamerasPtrHash(int num);

    // 创建每个相机的图像处理实例并填充哈希表
    void createColorProcessHash(int num);

    // 创建每个相机的线程哈希表
    void createThreadsHash(int num);

    // 销毁所有相机并清空相机指针哈希表
    void destoryAllCameras();

    // 销毁所有相机并清空图像处理指针哈希表
    void destoryAllColorProcess();

    // 销毁所有的线程哈希表
    void destoryAllThreads();

    // 根据id销毁相机并清除相机指针哈希表
    void destoryCameraByIndex(int index);

    // 根据id销毁相机并清除图像处理指针哈希表
    void destoryColorProcessByIndex(int index);

    // 根据id销毁线程并清除哈希表
    void destoryThreadByIndex(int index);

    // 清空画布(相机画面)
    void clearPanels();

    // 清空小方块
    void clearSmallPanels();

    // 清空文字显示框
    void clearLabelText();

    // 清空所有
    void clearAllComponents();

    // 相机间的QPixmap拷贝
    void copyQPixmap(QObject* src, QObject* dest);

    // 根据圆圈标记点计算颜色字符串
    void calculateColorStringByMarkPoints(const int &cameraIndex, ColorProcess* colorProcessPtr);

    // 将各个相机的颜色字符串拼接起来
    QString spliceColorString();

    // 开启所有线程
    void startAllThreads();

    // 关闭所有线程
    void closeAllThreads();

    /*************************阈值组件相关*****************************/
    // 阈值滑块与按钮的关联函数
    void connectSliderToSpinBox(QSpinBox *spinBox, QSlider *slider);

    // 连接所有滑块与按钮
    void connectAllSilederToSpinBox();

    // 从当前colorprocess实例中读取阈值并将其值赋给数字显示框(默认使用ColorProcess_U的参数)
    void bindColorThresholdsToSpinBox();

    // 将当前数字显示框中的数值赋值给所有colorprocess的阈值
    void bindSpinBoxToColorThresholds();

    // 获取当前所有参数
    void getAllThresholds(QJsonObject &jsonObj);


    /******************************计时器*****************************/
    void setTimerPtr(QTimer *timer);
    QTimer *getTimerPtr();
    void startTimer();
    void stopTimer();

    /*************************单面颜色显示****************************/
    // 给单个相机绘制小方块颜色的函数
    void setSingleCameraPlanViewColors(const int &labelStartIndex, const QString &colorString);
    // 设置颜色显示区域中每个小方块的颜色
    void setPlanViewColors(const int &camIndex, const QString &colorString);

    /*************************以下为槽函数****************************/
    // 接收并显示画面
    void showQPixmap(const int &cameraIndex, const QPixmap& qPixmap);

    // 当处于Debug模式时，将参数中的的画面传给其它没有相机的画布
    int copyCamera_UToOthers(int cameraIndex, cv::Mat frame);

    // 公有成员
    bool is_Debug = false;
    bool is_CameraOpen = false;
    bool is_Connected = false;
    bool is_GetColor = false;

signals:
    void sendStableColorString(const QString wholeColorString);

private slots:

    void on_openCameraBtn_clicked();

    void on_closeCameraBtn_clicked();

    void on_debugBtn_clicked();

    void on_resetHSVConfigBtn_clicked();

    void on_saveHSVConfigBtn_clicked();

    void on_loadHSVConfigBtn_clicked();

    void on_getColorBtn_clicked();

private:
    Ui::Widget *ui;

    // 当前(本轮)识别到的完整字符串
    QString totalColorString;

    // 用于保存连接的句柄
    QMetaObject::Connection debugConnection;

    // 计时器
    QTimer *m_timer;

    // 相机指针哈希表
    QHash<int, QObject*> m_camerasPtr;
    // 图像处理类哈希表
    QHash<int, ColorProcess*> m_colorProcessPtr;
    // 线程哈希表
    QHash<int, QThread*> m_threads;

};
#endif // WIDGET_H
