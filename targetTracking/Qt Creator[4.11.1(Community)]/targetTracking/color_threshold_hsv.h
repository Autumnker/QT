#ifndef COLORTHRESHOLDHSV_H
#define COLORTHRESHOLDHSV_H

#include <QWidget>
#include <QPushButton>
#include <QJsonObject>
#include <QVector>
#include <QSpinBox>
#include <QSlider>
#include <QUrl>

/**
* @brief 颜色阈值相关
*/
class Color_Threshold_HSV : public QWidget {
    Q_OBJECT
public:
    explicit Color_Threshold_HSV(QWidget *parent = nullptr);
    ~Color_Threshold_HSV();

    /** @brief 绑定滑动组件和数字输入框*/
    void bindSilder_spinBox();

    // 颜色相关字符串数组
    /** @brief 获取颜色数组*/
    QVector<QString> getColorArray() const;
    /** @brief 获取通道数组*/
    QVector<QString> getChannelArray() const;
    /** @brief 获取范围限定数组*/
    QVector<QString> getRangeArray() const;

    // 颜色阈值相关数组
    /** @brief 获取H通道最大最小值数组*/
    QVector<QPair<int, int>> getHpairArray() const;
    /** @brief 获取s通道最大最小值数组*/
    QVector<QPair<int, int>> getSpairArray() const;
    /** @brief 获取v通道最大最小值数组*/
    QVector<QPair<int, int>> getvpairArray() const;

private:
    /** @brief 初始化滑动条/数字输入框指针容器*/
    void initSliderSpinBoxVector();
    /** @brief 打开路径时如果没有对应文件夹则自动创建*/
    bool openUrlWithAutoCreate(const QUrl &url);
    /** @brief 获取当前所有参数的阈值并通过jsonObj传出*/
    void getAllThresholds(QJsonObject &jsonObj);

signals:

private slots:
    /** @brief 加载阈值参数*/
    void loadHSVConfig();
    /** @brief 保存阈值参数*/
    void saveHSVConfig();
    /** @brief 重置阈值参数*/
    void resetHSVConfig();

public:
    // 颜色索引
    enum ColorIndex
    {
        RedIdx,
        GreenIdx,
        BlueIdx,
        YellowIdx,
        OrangeIdx,
        PurpleIdx,
        ColorCount
    };
    // 颜色阈值
    const int minColorThreshold = 0;
    const int maxColorThreshold = 255;

private:
    // 颜色相关
    QVector<QString> _colorArray;
    QVector<QString> _channelArray;
    QVector<QString> _rangeArray;
    // 组件值相关
    QVector<QPair<int, int>> _H_value;
    QVector<QPair<int, int>> _S_value;
    QVector<QPair<int, int>> _V_value;
    // 滑动条/数字输入框相关
    QVector<QPair<QPair<QSlider *, QSpinBox *>, QPair<QSlider *, QSpinBox *>>> _H_slider_spinbox;
    QVector<QPair<QPair<QSlider *, QSpinBox *>, QPair<QSlider *, QSpinBox *>>> _S_slider_spinbox;
    QVector<QPair<QPair<QSlider *, QSpinBox *>, QPair<QSlider *, QSpinBox *>>> _V_slider_spinbox;
    // 按钮相关
    QPushButton *_loading_parameters;
    QPushButton *_save_parameters;
    QPushButton *_reset_parameters;
};

#endif // COLORTHRESHOLDHSV_H
