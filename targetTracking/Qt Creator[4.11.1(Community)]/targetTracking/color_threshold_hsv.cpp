#include "color_threshold_hsv.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include "widget.h"
#include "ui_widget.h"

using namespace Config::Color_Threshold_HSV;

Color_Threshold_HSV::Color_Threshold_HSV(QWidget *parent)
    : QWidget(parent) {
    // 初始化共有成员
    QString ColorArray[]   = {"Red", "Green", "Blue", "Yellow", "Orange", "Purple"};
    QString ChannelArray[] = {"H", "S", "V"};
    QString RangeArray[]   = {"min", "max"};
    for (QString &color : ColorArray) {
        _colorArray.push_back(color);
    }
    for (QString &channel : ChannelArray) {
        _channelArray.push_back(channel);
    }
    for (QString &range : RangeArray) {
        _rangeArray.push_back(range);
    }

    // 初始化组件阈值数组
    for (int index = 0; index < ColorCount; ++index) {
        _H_value.push_back(qMakePair(minColorThreshold, maxColorThreshold));
        _S_value.push_back(qMakePair(minColorThreshold, maxColorThreshold));
        _V_value.push_back(qMakePair(minColorThreshold, maxColorThreshold));
    }
    // 初始化指针容器
    initSliderSpinBoxVector();
    // 关联窗口组件
    Widget *widget_ptr  = (Widget *)parent;
    _loading_parameters = widget_ptr->getUI_Widget()->Btn_loading_parameters;
    _save_parameters    = widget_ptr->getUI_Widget()->Btn_save_parameters;
    _reset_parameters   = widget_ptr->getUI_Widget()->Btn_reset_parameters;
    // 绑定信号与槽函数
    bindSilder_spinBox();
    connect(_loading_parameters, &QPushButton::clicked, this, &Color_Threshold_HSV::loadHSVConfig);
    connect(_save_parameters, &QPushButton::clicked, this, &Color_Threshold_HSV::saveHSVConfig);
    connect(_reset_parameters, &QPushButton::clicked, this, &Color_Threshold_HSV::resetHSVConfig);
    // 重置阈值参数
    resetHSVConfig();
}

Color_Threshold_HSV::~Color_Threshold_HSV() {
}

void Color_Threshold_HSV::bindSilder_spinBox() {
    Widget *parent_ptr = (Widget *)this->parent();

    // 绑定循环
    for (int i = 0; i < ColorCount; ++i) {
        QString color = _colorArray[i];

        for (QString &channel : _channelArray) {
            for (QString &range : _rangeArray) {
                // 构造组件名称
                QString sliderName  = QString("%1_%2_%3_Slider").arg(color).arg(channel).arg(range);
                QString spinBoxName = QString("%1_%2_%3_spinBox").arg(color).arg(channel).arg(range);

                // 获取对象指针
                QSlider *slider   = parent_ptr->getUI_Widget()->widget_slider_spinBox->findChild<QSlider *>(sliderName);
                QSpinBox *spinBox = parent_ptr->getUI_Widget()->widget_slider_spinBox->findChild<QSpinBox *>(spinBoxName);

                // 绑定
                if (slider && spinBox) {
                    connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
                    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

                    // 绑定阈值参数
                    if (channel == "H") {
                        if (range == "min") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _H_value[i].first = value; });
                        } else if (range == "max") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _H_value[i].second = value; });
                        }
                    } else if (channel == "S") {
                        if (range == "min") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _S_value[i].first = value; });
                        } else if (range == "max") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _S_value[i].second = value; });
                        }
                    } else if (channel == "V") {
                        if (range == "min") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _V_value[i].first = value; });
                        } else if (range == "max") {
                            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) { _V_value[i].second = value; });
                        }
                    }

                } else {
                    qDebug() << sliderName << "或" << spinBoxName << "指针为空";
                }
            }
        }
    }
}

QVector<QString> Color_Threshold_HSV::getColorArray() const {
    return _colorArray;
}

QVector<QString> Color_Threshold_HSV::getChannelArray() const {
    return _channelArray;
}

QVector<QString> Color_Threshold_HSV::getRangeArray() const {
    return _rangeArray;
}

QVector<QPair<int, int>> Color_Threshold_HSV::getHpairArray() const {
    return _H_value;
}

QVector<QPair<int, int>> Color_Threshold_HSV::getSpairArray() const {
    return _S_value;
}

QVector<QPair<int, int>> Color_Threshold_HSV::getvpairArray() const {
    return _V_value;
}

void Color_Threshold_HSV::initSliderSpinBoxVector() {
    Ui_Widget *ui = ((Widget *)(this->parent()))->getUI_Widget();
    for (int index = 0; index < ColorCount; ++index) {
        // 找到相应的QSpinBox组件指针
        QSpinBox *H_spinBox_min = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_H_min_spinBox"));
        QSpinBox *H_spinBox_max = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_H_max_spinBox"));
        QSpinBox *S_spinBox_min = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_S_min_spinBox"));
        QSpinBox *S_spinBox_max = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_S_max_spinBox"));
        QSpinBox *V_spinBox_min = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_V_min_spinBox"));
        QSpinBox *V_spinBox_max = ui->widget_slider_spinBox->findChild<QSpinBox *>(QString(_colorArray[index] + "_V_max_spinBox"));
        // 找到相应的QSlider组件指针
        QSlider *H_slider_min = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_H_min_Slider"));
        QSlider *H_slider_max = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_H_max_Slider"));
        QSlider *S_slider_min = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_S_min_Slider"));
        QSlider *S_slider_max = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_S_max_Slider"));
        QSlider *V_slider_min = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_V_min_Slider"));
        QSlider *V_slider_max = ui->widget_slider_spinBox->findChild<QSlider *>(QString(_colorArray[index] + "_V_max_Slider"));
        // 将组件指针赋值给指针容器
        _H_slider_spinbox.push_back(qMakePair(qMakePair(H_slider_min, H_spinBox_min), qMakePair(H_slider_max, H_spinBox_max)));
        _S_slider_spinbox.push_back(qMakePair(qMakePair(S_slider_min, S_spinBox_min), qMakePair(S_slider_max, S_spinBox_max)));
        _V_slider_spinbox.push_back(qMakePair(qMakePair(V_slider_min, V_spinBox_min), qMakePair(V_slider_max, V_spinBox_max)));
        // 重置滑动条/数字输入框阈值
        int min = Config::Color_Threshold_HSV::minNum;
        int max = Config::Color_Threshold_HSV::maxNum;

        H_spinBox_min->setMinimum(min);
        H_spinBox_min->setMaximum(max);
        H_spinBox_max->setMinimum(min);
        H_spinBox_max->setMaximum(max);
        H_slider_min->setMinimum(min);
        H_slider_min->setMaximum(max);
        H_slider_max->setMinimum(min);
        H_slider_max->setMaximum(max);

        S_spinBox_min->setMinimum(min);
        S_spinBox_min->setMaximum(max);
        S_spinBox_max->setMinimum(min);
        S_spinBox_max->setMaximum(max);
        S_slider_min->setMinimum(min);
        S_slider_min->setMaximum(max);
        S_slider_max->setMinimum(min);
        S_slider_max->setMaximum(max);

        V_spinBox_min->setMinimum(min);
        V_spinBox_min->setMaximum(max);
        V_spinBox_max->setMinimum(min);
        V_spinBox_max->setMaximum(max);
        V_slider_min->setMinimum(min);
        V_slider_min->setMaximum(max);
        V_slider_max->setMinimum(min);
        V_slider_max->setMaximum(max);
    }
}

void Color_Threshold_HSV::loadHSVConfig() {
    // 1. 弹出文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择HSV配置文件",
        Config::Color_Threshold_HSV::hsvConfigPath, // 默认打开配置目录
        "JSON文件 (*.json);;所有文件 (*.*)");

    if (filePath.isEmpty()) return; // 用户取消选择

    // 2. 读取JSON文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + filePath);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        QMessageBox::critical(this, "错误", "无效的JSON格式");
        return;
    }

    QJsonObject jsonObj = doc.object();

    // 将数值加载进各个参数中
    for (int index = 0; index < ColorCount; ++index) {
        _H_value[index].first  = jsonObj[QString(_colorArray[index] + "_H" + "_min")].toInt();
        _H_value[index].second = jsonObj[QString(_colorArray[index] + "_H" + "_max")].toInt();
        _S_value[index].first  = jsonObj[QString(_colorArray[index] + "_S" + "_min")].toInt();
        _S_value[index].second = jsonObj[QString(_colorArray[index] + "_S" + "_max")].toInt();
        _V_value[index].first  = jsonObj[QString(_colorArray[index] + "_V" + "_min")].toInt();
        _V_value[index].second = jsonObj[QString(_colorArray[index] + "_V" + "_max")].toInt();

        emit _H_slider_spinbox[index].first.first->valueChanged(_H_value[index].first);
        emit _H_slider_spinbox[index].second.first->valueChanged(_H_value[index].second);
        emit _S_slider_spinbox[index].first.first->valueChanged(_S_value[index].first);
        emit _S_slider_spinbox[index].second.first->valueChanged(_S_value[index].second);
        emit _V_slider_spinbox[index].first.first->valueChanged(_V_value[index].first);
        emit _V_slider_spinbox[index].second.first->valueChanged(_V_value[index].second);
    }
}

void Color_Threshold_HSV::saveHSVConfig() {
    // 1. 准备路径和文件名
    QString fileName = QString("HSVConfig_%1.json")
                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd__hh_mm_ss"));
    QString dirPath  = Config::Color_Threshold_HSV::hsvConfigPath;
    QString fullPath = QDir(dirPath).filePath(fileName);

    // 2. 确保目录存在
    QUrl dirUrl = QUrl::fromLocalFile(dirPath);
    if (!openUrlWithAutoCreate(dirUrl)) {
        QMessageBox::critical(this, "错误", "无法创建配置目录");
        return;
    }

    // 3. 写入JSON文件
    QJsonObject jsonConfig;
    getAllThresholds(jsonConfig); // 获取所有参数

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "错误", "无法创建配置文件");
        return;
    }
    file.write(QJsonDocument(jsonConfig).toJson());
    file.close();

    // 5. 提示
    QMessageBox::information(this, "成功", QString("配置已保存为:\n%1").arg(fileName), QMessageBox::Ok);
}

void Color_Threshold_HSV::resetHSVConfig() {
    for (int index = 0; index < ColorCount; ++index) {
        // 找到默认位置的值
        int H_min = Config::Color_Threshold_HSV::H_min_array[index];
        int H_max = Config::Color_Threshold_HSV::H_max_array[index];
        int S_min = Config::Color_Threshold_HSV::S_min_array[index];
        int S_max = Config::Color_Threshold_HSV::S_max_array[index];
        int V_min = Config::Color_Threshold_HSV::V_min_array[index];
        int V_max = Config::Color_Threshold_HSV::V_max_array[index];
        // 将默认位置的值赋值给QSpinBox,并发送阈值改变信号
        emit _H_slider_spinbox[index].first.first->valueChanged(H_min);
        emit _H_slider_spinbox[index].second.first->valueChanged(H_max);
        emit _S_slider_spinbox[index].first.first->valueChanged(S_min);
        emit _S_slider_spinbox[index].second.first->valueChanged(S_max);
        emit _V_slider_spinbox[index].first.first->valueChanged(V_min);
        emit _V_slider_spinbox[index].second.first->valueChanged(V_max);
    }
}

bool Color_Threshold_HSV::openUrlWithAutoCreate(const QUrl &url) {
    if (url.isLocalFile()) {
        QString path = url.toLocalFile();
        QDir dir(path);

        if (QFileInfo(path).isFile()) {
            dir.cdUp();
        }

        if (!dir.exists() && !dir.mkpath(".")) {
            qWarning() << "openUrlWithAutoCreate--->自动创建路径失败:" << dir.path();
            return false;
        }
    }
    return true; // 返回目录是否创建成功
}

void Color_Threshold_HSV::getAllThresholds(QJsonObject &jsonObj) {
    for (int index = 0; index < ColorCount; ++index) {
        jsonObj[QString(_colorArray[index] + "_H" + "_min")] = _H_value[index].first;
        jsonObj[QString(_colorArray[index] + "_H" + "_max")] = _H_value[index].second;
        jsonObj[QString(_colorArray[index] + "_S" + "_min")] = _S_value[index].first;
        jsonObj[QString(_colorArray[index] + "_S" + "_max")] = _S_value[index].second;
        jsonObj[QString(_colorArray[index] + "_V" + "_min")] = _V_value[index].first;
        jsonObj[QString(_colorArray[index] + "_V" + "_max")] = _V_value[index].second;
    }
}
