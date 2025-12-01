#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>

#include <QDesktopServices>
#include <QUrl>
#include <QDir>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "config.h"
#include "camera.h"
#include "colorprocess.h"
#include "algorithm.h"

// 全局变量
int NumberOfStringStability = 0;  // 完整的颜色字符串重复出现的次数
QString LastColorString;          // 上一次出现的颜色字符串

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 创建类的实例
    QTimer *timer = new QTimer(this);   // 定时器类
    timer->setInterval(Config::timerHZ);
    setTimerPtr(timer);                 // 赋值给Widget中的成员

    Algorithm *cubeSolvingServer = new Algorithm(this); // 获得魔方求解序列的类

    // 将所有Slider与SpinBox连接起来
    this->connectAllSilederToSpinBox();

    /****************** 链接信号与槽函数 ******************/

    // 若处于调试模式,将U号相机的画面复制给其它未被打开的相机
    connect(m_timer, &QTimer::timeout, this, [=](){
        Camera *cam_U =  (Camera*)m_camerasPtr[Config::U];
        this->copyCamera_UToOthers(cam_U->getCameraIndex(), cam_U->getFrame());
    });

    // timer与帧更新 --- 帧更新与显示
    connect(ui->openCameraBtn, &QPushButton::clicked, this, [=](){
        if(!is_Connected){
            is_Connected = true;

            // 将默认值加载到数字显示框中
            this->bindColorThresholdsToSpinBox();

            // 将完整字符串发送信号与字符串求解类中的接收槽函数连接在一起
            connect(this, &Widget::sendStableColorString, cubeSolvingServer, &Algorithm::returnSolvingString);
            // 将求解序列信号与求解序列显示框槽函数进行连接
            connect(cubeSolvingServer, &Algorithm::sendSolvingString, this, [=](const QString solvingString){
                ui->label_SolveOrderList->setText("求解序列: " + solvingString);
            });

            // 相机相关信号槽连接
            for(int i = 0; i < Config::CameraNum; ++i){
                // 处理与显示原始帧
                Camera *cam = (Camera*)m_camerasPtr[i];
                ColorProcess *colorProcess = m_colorProcessPtr[i];
                if(nullptr == colorProcess){
                    qDebug() << "错误：找不到相机或图像处理实例(相机ID：" << i << ")";
                    return;
                }

                // 移动摄像头\图像处理实例到指定线程中
                cam->moveToThread(m_threads[i]);
                colorProcess->moveToThread(m_threads[i]);

                connect(timer, &QTimer::timeout, cam, &Camera::FrameToQPixmap); //  将原始帧转化为QPixmap
                connect(cam, &Camera::sendQpixmap, this, &Widget::showQPixmap); //  将QPixmap显示在画布上
                connect(cam, &Camera::sendFrame, colorProcess, &ColorProcess::convertBGR2HSV);  // 将原始帧发送给图像处理实例获得灰度图

                // 计算每个标记点圆圈中的颜色
                connect(colorProcess, &ColorProcess::sendHSVFrame, this, [=](){
                    calculateColorStringByMarkPoints(colorProcess->getCameraIndex(), colorProcess);
                });
                // 将字符串显示在色块串提示框上
                connect(colorProcess, &ColorProcess::sendColorString, this, &Widget::spliceColorString);
                // 将颜色字符串显示在小方块上
                connect(colorProcess, &ColorProcess::sendColorString, this, &Widget::setPlanViewColors);
            }
            timer->start(Config::FrameRate);

            // 开启所有相机的线程
            startAllThreads();

        }
    });

    // 资源回收函数
    // connect(this, &QObject::destroyed, this, &Widget::closeAllThreads);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::destoryAllCameras()
{
    for(auto i = m_camerasPtr.begin(); i != m_camerasPtr.end(); ++i){
        // 检查当前相机指针是否为空指针
        if(i.value() != nullptr){
            Camera *cam = (Camera*)i.value();
            cam->closeCamera(); // 关闭相机
//            i.value()->deleteLater();
        }
    }
}

void Widget::destoryCameraByIndex(int index)
{
    if(m_camerasPtr.count(index) > 0){
        m_camerasPtr[index]->deleteLater();
        m_camerasPtr.remove(index);
    }else{
        qDebug() << "相机指针哈希表中不存在 " << index << " 号相机";
        QString message = QString("相机指针哈希表中不存在 %1 号相机").arg(index);
        ui->appBtnResponse->setText(message);
    }
}

void Widget::destoryAllColorProcess()
{
    for(auto i = m_colorProcessPtr.begin(); i != m_colorProcessPtr.end(); ++i){
        // 检查当前图像处理指针是否为空指针
        if(i.value() != nullptr){
            i.value()->deleteLater();
        }
    }

    // 清空哈希表
    m_colorProcessPtr.clear();
}

void Widget::destoryAllThreads()
{
    for(auto i = m_threads.begin(); i != m_threads.end(); ++i){
        // 检查当前线程指针是否为空
        if(i.value() != nullptr){
            i.value()->deleteLater();
        }
    }

    // 清空哈希表
    m_threads.clear();
}

void Widget::destoryColorProcessByIndex(int index)
{
    if(m_colorProcessPtr.count(index) > 0){
        m_colorProcessPtr[index]->deleteLater();
        m_colorProcessPtr.remove(index);
    }else{
        qDebug() << "图像处理指针哈希表中不存在 " << index << " 号图像处理实例";
        QString message = QString("图像处理指针哈希表中不存在 %1 号图像处理实例").arg(index);
        ui->appBtnResponse->setText(message);
    }
}

void Widget::destoryThreadByIndex(int index)
{
    if(m_threads.count(index) > 0){
        m_threads[index]->deleteLater();
        m_threads.remove(index);
    }else{
        qDebug() << "线程哈希表中不存在 " << index << "号线程实例";
        QString message = QString("线程指针哈希表中不存在 %1 号线程实例").arg(index);
        ui->appBtnResponse->setText(message);
    }
}

void Widget::clearPanels()
{
    ui->camera_U->clear();
    ui->camera_D->clear();
    ui->camera_LF->clear();
    ui->camera_BR->clear();

    // 清空QPixmap
    for(auto i = m_camerasPtr.begin(); i != m_camerasPtr.end(); ++i){
        Camera *cam = (Camera*)i.value();
        QPixmap qPixmap;
        cam->setQPixmap(qPixmap);
    }
}

void Widget::clearSmallPanels()
{
    QString planName;
    QString styleSheet;
    for(int i = 0; i < 54; ++i){
        planName = QString("plan%1").arg(i);
        QLabel *label = this->findChild<QLabel*>(planName);
        if(label){
            styleSheet = QString("background-color: %1;").arg(Config::colorFaceMap['N']);
            label->setStyleSheet(styleSheet);
        }
    }
}

// 清空文字显示框
void Widget::clearLabelText()
{
    ui->appBtnResponse->setText("提示信息");
    ui->label_ColorOrderList->setText("色块序列");
    ui->label_SolveOrderList->setText("求解序列");
    ui->label_MachineOrderMessage->setText("机器指令");
}

// 清空所有
void Widget::clearAllComponents()
{
    this->clearPanels();
    this->clearSmallPanels();
    this->clearLabelText();
}

void Widget::copyQPixmap(QObject *src, QObject *dest)
{
    if(nullptr == src){
        qDebug() << "Widget::copyQPixmap--->src指针为空" << "\n";
        return;
    }

    Camera *camSrc = (Camera*)src;
    Camera *camDest = (Camera*)dest;
    QPixmap qPixmapSrc= camSrc->getQPixmap();
    camDest->setQPixmap(qPixmapSrc);
}

void Widget::calculateColorStringByMarkPoints(const int &cameraIndex, ColorProcess* colorProcess)
{
    if (!colorProcess) {
        qDebug() << "错误：colorProcessPtr 为空";
        return;
    }

    switch(cameraIndex){
        case Config::U : {
            QVector<QVector<QPoint>> marks_U = ui->camera_U->getMarking();
            colorProcess->colorRecognition(marks_U);
            break;
        }
        case Config::D : {
            QVector<QVector<QPoint>> marks_D = ui->camera_D->getMarking();
            colorProcess->colorRecognition(marks_D);
            break;
        }
        case Config::LF : {
            QVector<QVector<QPoint>> marks_LF = ui->camera_LF->getMarking();
            colorProcess->colorRecognition(marks_LF);
            break;
        }
        case Config::BR : {
            QVector<QVector<QPoint>> marks_BR = ui->camera_BR->getMarking();
            colorProcess->colorRecognition(marks_BR);
            break;
        }
        default: {
            qDebug() << "Widget::calculateColorStringByMarkPoints--->相机索引错误" << "\n";
        }
    }
}

// 拼接字符串的具体规则
void spliceRule(QString src, QString &dest){
    if(src.length() != 9){
        for(int i = 0; i < 9; ++i){
            if(src[i] != '\0'){
                dest += src[i];
            }else{
                dest += 'N';
            }
        }

    }else{
        dest += src;
    }
}

// 将各个相机的颜色字符串拼接起来
QString Widget::spliceColorString()
{
    // 清空字符串以存储本轮色块信息
    totalColorString = "";

    // 调试模式下：允许手动输入
    if(is_Debug){
        // 如果已经建立连接则不进行任何操作
        if (debugConnection) {
            return totalColorString;
        }
        // 建立新连接并保存句柄
        debugConnection = connect(ui->label_ColorOrderList, &QLineEdit::returnPressed, this, [&, this](){
            ui->appBtnResponse->setText("当前处于调试模式");
            QString inputText = ui->label_ColorOrderList->text();
            int inputTextLength = inputText.length();
            if(inputTextLength == 54){
                totalColorString = inputText;
                emit sendStableColorString(totalColorString);
                ui->appBtnResponse->setText("调试输入完毕,色块为:\n" + totalColorString);
            }else{
                QString message = "色块数量应该为54!,当前数量为: " + QString::number(inputTextLength);
                ui->appBtnResponse->setText(message);
            }

        });

        return totalColorString; // 注意：初次调用返回空，需等待用户输入
    }

    // 非Debug模式下的正常处理方法
    // 非调试模式时断开连接
    if (debugConnection) {
        QObject::disconnect(debugConnection);
        debugConnection = QMetaObject::Connection(); // 重置为无效连接
    }

    // 分解字符串
    QString LF_string = m_colorProcessPtr[Config::LF]->getColorString();
    int LF_len = LF_string.length();
    QString L_string = LF_string.left(LF_len/2);
    QString F_string = LF_string.mid(LF_len/2);

    QString BR_string = m_colorProcessPtr[Config::BR]->getColorString();
    int BR_len = BR_string.length();
    QString B_string = BR_string.left(BR_len/2);
    QString R_string = BR_string.mid(BR_len/2);

    QString U_string = m_colorProcessPtr[Config::U]->getColorString();
    QString D_string = m_colorProcessPtr[Config::D]->getColorString();
    // 拼接字符串，顺序U R F D L B
    spliceRule(U_string, totalColorString);
    spliceRule(R_string, totalColorString);
    spliceRule(F_string, totalColorString);
    spliceRule(D_string, totalColorString);
    spliceRule(L_string, totalColorString);
    spliceRule(B_string, totalColorString);

    // 在“色块序列”显示框显示色块序列
    if(!totalColorString.isEmpty()){
        if(!is_GetColor){
            ui->label_ColorOrderList->setText("实时色块序列: " + totalColorString);
        }

        if(LastColorString.isEmpty()){
            LastColorString = totalColorString; // 将第一次出现的字符记录下来
            NumberOfStringStability = 1;        // 出现次数+1
        }else{
            if(totalColorString == LastColorString){
                NumberOfStringStability++;
            }else{
                LastColorString = totalColorString;
                NumberOfStringStability = 1;
            }
        }
    }

    if(NumberOfStringStability >= Config::coutToSendThresholdValue){    // 字符串重复出现次数达到要求的次数
        // qDebug() << "字符串稳定出现: " << LastColorString << " 次数为: " << NumberOfStringStability << "\n";
        if(is_GetColor){
            ui->label_ColorOrderList->setText("稳定色块序列: " + totalColorString);
            emit sendStableColorString(LastColorString);    // 发送完整的字符串信号
        }

        NumberOfStringStability = 0;
        LastColorString = "";
    }

    return totalColorString;
}

// 开启所有线程
void Widget::startAllThreads()
{
    for(auto it = m_threads.begin(); it != m_threads.end(); ++it){
        if(it.value() != nullptr){
            it.value()->start();
        }
    }
}

// 关闭所有线程
void Widget::closeAllThreads()
{
    for(auto it = m_threads.begin(); it != m_threads.end(); ++it){
        if(it.value() != nullptr){
            it.value()->quit();
            it.value()->wait();
            it.value()->deleteLater();
            it.value() = nullptr;   // 线程置空指针
        }
    }
}

// 阈值滑块与按钮的关联函数
void Widget::connectSliderToSpinBox(QSpinBox *spinBox, QSlider *slider)
{
    // 定义一个函数指针指向 QSpinBox::valueChanged(int) 信号 ---> 因为该函数有多个重载
    void (QSpinBox::*spinBoxValueChangedInt)(int) = &QSpinBox::valueChanged;

    // 进行信号槽连接
    connect(spinBox, spinBoxValueChangedInt, slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);
    connect(spinBox, spinBoxValueChangedInt, this, &Widget::bindSpinBoxToColorThresholds);

}

// 连接所有滑块与按钮
void Widget::connectAllSilederToSpinBox()
{
    // 黄色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_yellow, ui->h_LowerSlider_yellow);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_yellow, ui->h_UpperSlider_yellow);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_yellow, ui->s_LowerSlider_yellow);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_yellow, ui->s_UpperSlider_yellow);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_yellow, ui->v_LowerSlider_yellow);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_yellow, ui->v_UpperSlider_yellow);
    // 蓝色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_blue, ui->h_LowerSlider_blue);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_blue, ui->h_UpperSlider_blue);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_blue, ui->s_LowerSlider_blue);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_blue, ui->s_UpperSlider_blue);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_blue, ui->v_LowerSlider_blue);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_blue, ui->v_UpperSlider_blue);
    // 红色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_red, ui->h_LowerSlider_red);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_red, ui->h_UpperSlider_red);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_red, ui->s_LowerSlider_red);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_red, ui->s_UpperSlider_red);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_red, ui->v_LowerSlider_red);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_red, ui->v_UpperSlider_red);
    // 绿色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_green, ui->h_LowerSlider_green);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_green, ui->h_UpperSlider_green);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_green, ui->s_LowerSlider_green);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_green, ui->s_UpperSlider_green);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_green, ui->v_LowerSlider_green);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_green, ui->v_UpperSlider_green);
    // 橙色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_orange, ui->h_LowerSlider_orange);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_orange, ui->h_UpperSlider_orange);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_orange, ui->s_LowerSlider_orange);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_orange, ui->s_UpperSlider_orange);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_orange, ui->v_LowerSlider_orange);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_orange, ui->v_UpperSlider_orange);
    // 白色
    this->connectSliderToSpinBox(ui->h_LowerSpinBox_white, ui->h_LowerSlider_white);
    this->connectSliderToSpinBox(ui->h_UpperSpinBox_white, ui->h_UpperSlider_white);
    this->connectSliderToSpinBox(ui->s_LowerSpinBox_white, ui->s_LowerSlider_white);
    this->connectSliderToSpinBox(ui->s_UpperSpinBox_white, ui->s_UpperSlider_white);
    this->connectSliderToSpinBox(ui->v_LowerSpinBox_white, ui->v_LowerSlider_white);
    this->connectSliderToSpinBox(ui->v_UpperSpinBox_white, ui->v_UpperSlider_white);
}

// 从当前colorprocess实例中读取阈值并将其值赋给数字显示框(默认使用ColorProcess_U的参数)
void Widget::bindColorThresholdsToSpinBox()
{
    // 将colorprocess中的阈值加载到数字显示框中
    QHash<int, int> H_upper =  m_colorProcessPtr[Config::U]->getHUpperThreshold();
    QHash<int, int> H_lower =  m_colorProcessPtr[Config::U]->getHLowerThreshold();
    QHash<int, int> S_upper =  m_colorProcessPtr[Config::U]->getSUpperThreshold();
    QHash<int, int> S_lower =  m_colorProcessPtr[Config::U]->getSLowerThreshold();
    QHash<int, int> V_upper =  m_colorProcessPtr[Config::U]->getVUpperThreshold();
    QHash<int, int> V_lower =  m_colorProcessPtr[Config::U]->getVLowerThreshold();

    for(int i = 0; i < Config::ColorNum; ++i){
        QString color = Config::colorOrder[i];  // 得到当前索引的颜色

        // 构建组件名称
        QString HU_name = "h_UpperSpinBox_" + color;
        QString HL_name = "h_LowerSpinBox_" + color;
        QString SU_name = "s_UpperSpinBox_" + color;
        QString SL_name = "s_LowerSpinBox_" + color;
        QString VU_name = "v_UpperSpinBox_" + color;
        QString VL_name = "v_LowerSpinBox_" + color;

        // 创建指向组件的指针
        QSpinBox *HU_ptr  = this->findChild<QSpinBox*>(HU_name);
        QSpinBox *HL_ptr  = this->findChild<QSpinBox*>(HL_name);
        QSpinBox *SU_ptr  = this->findChild<QSpinBox*>(SU_name);
        QSpinBox *SL_ptr  = this->findChild<QSpinBox*>(SL_name);
        QSpinBox *VU_ptr  = this->findChild<QSpinBox*>(VU_name);
        QSpinBox *VL_ptr  = this->findChild<QSpinBox*>(VL_name);

        if(HU_ptr == nullptr || HL_ptr == nullptr || SU_ptr == nullptr || SL_ptr  == nullptr || VU_ptr  == nullptr || VL_ptr == nullptr){
            qDebug() << "Widget::bindColorThresholdsToSpinBox--->空指针" << "\n";
            return;
        }

        // 通过指针设定组件的值
        HU_ptr->setValue(H_upper[i]);
        HL_ptr->setValue(H_lower[i]);
        SU_ptr->setValue(S_upper[i]);
        SL_ptr->setValue(S_lower[i]);
        VU_ptr->setValue(V_upper[i]);
        VL_ptr->setValue(V_lower[i]);

    }
}

// 将当前数字显示框中的数值赋值给所有colorprocess的阈值
void Widget::bindSpinBoxToColorThresholds()
{
    // 构建阈值<颜色索引, 阈值>
    QHash<int, int> HUpperArray;
    QHash<int, int> HLowerArray;
    QHash<int, int> SUpperArray;
    QHash<int, int> SLowerArray;
    QHash<int, int> VLowerArray;
    QHash<int, int> VUpperArray;

    // 找到数字显示框
    for(int i = 0; i < Config::ColorNum; ++i){
        // 获得颜色名字
        QString color = Config::colorOrder[i];

        if(color.isEmpty()){
            qDebug() << "Widget::bindSpinBoxToColorThresholds--->颜色为空" << "\n";
            return;
        }

        // 找到其六个阈值组件
        QString HU_name = "h_UpperSpinBox_" + color;
        QString HL_name = "h_LowerSpinBox_" + color;
        QString SU_name = "s_UpperSpinBox_" + color;
        QString SL_name = "s_LowerSpinBox_" + color;
        QString VU_name = "v_UpperSpinBox_" + color;
        QString VL_name = "v_LowerSpinBox_" + color;

        // 构建组件指针
        QSpinBox *HU_ptr  = this->findChild<QSpinBox*>(HU_name);
        QSpinBox *HL_ptr  = this->findChild<QSpinBox*>(HL_name);
        QSpinBox *SU_ptr  = this->findChild<QSpinBox*>(SU_name);
        QSpinBox *SL_ptr  = this->findChild<QSpinBox*>(SL_name);
        QSpinBox *VU_ptr  = this->findChild<QSpinBox*>(VU_name);
        QSpinBox *VL_ptr  = this->findChild<QSpinBox*>(VL_name);

        if(HU_ptr == nullptr || HL_ptr == nullptr || SU_ptr == nullptr || SL_ptr  == nullptr || VU_ptr  == nullptr || VL_ptr == nullptr){
            qDebug() << "Widget::bindSpinBoxToColorThresholds--->空指针" << "\n";
            return;
        }

        // 获得当前阈值
        int HU_value = HU_ptr->value();
        int HL_value = HL_ptr->value();
        int SU_value = SU_ptr->value();
        int SL_value = SL_ptr->value();
        int VU_value = VU_ptr->value();
        int VL_value = VL_ptr->value();

        HUpperArray.insert(i,HU_value);
        HLowerArray.insert(i,HL_value);
        SUpperArray.insert(i,SU_value);
        SLowerArray.insert(i,SL_value);
        VUpperArray.insert(i,VU_value);
        VLowerArray.insert(i,VL_value);

        // 将数字显示框中的数字赋值给所有colorprocess的阈值
        for(auto it = m_colorProcessPtr.begin(); it != m_colorProcessPtr.end(); ++it){
            ColorProcess *colorProcess_ptr = (ColorProcess*)it.value();
            colorProcess_ptr->setHUpperThreshold(HUpperArray);
            colorProcess_ptr->setHLowerThreshold(HLowerArray);
            colorProcess_ptr->setSUpperThreshold(SUpperArray);
            colorProcess_ptr->setSLowerThreshold(SLowerArray);
            colorProcess_ptr->setVUpperThreshold(VUpperArray);
            colorProcess_ptr->setVLowerThreshold(VLowerArray);
        }
    }
}

// 给单个相机绘制小方块颜色的函数
void Widget::setSingleCameraPlanViewColors(const int &labelStartIndex, const QString &colorString){
    for(int i = 0; i < colorString.length(); ++i){
        QString color = Config::colorFaceMap[colorString[i]];
        if(!color.isEmpty()){
            QString labelName = QString("plan%1").arg(labelStartIndex + i); // 构建该小方块的名称
            QLabel *label = this->findChild<QLabel*>(labelName);            // 创建指向画布上该小方块的指针
            if(label){
                QString newStyleSheet = QString("background-color: %1;").arg(color);
                label->setStyleSheet(newStyleSheet);
            }
        }
    }
}

// 设置颜色显示区域中每个小方块的颜色
void Widget::setPlanViewColors(const int &camIndex, const QString &colorString)
{   
    // U 相机 plan0 ~ plan8
    if(Config::U == camIndex){
        int labelStartIndex = 0;
        setSingleCameraPlanViewColors(labelStartIndex, colorString);
    }

    // D 相机 plan9 ~ plan17
    if(Config::D == camIndex){
        int labelStartIndex = 9;
        setSingleCameraPlanViewColors(labelStartIndex, colorString);
    }

    // LF 相机 L:plan18 ~ plan26 \ F:plan27 ~ plan35
    if(Config::LF == camIndex){
        int labelStartIndex = 18;
        setSingleCameraPlanViewColors(labelStartIndex, colorString);
    }

    // BR 相机 B:plan36 ~ plan44 \ R:plan45 ~ plan53
    if(Config::BR == camIndex){
        int labelStartIndex = 36;
        setSingleCameraPlanViewColors(labelStartIndex, colorString);
    }
}

// 在画布上显示相机图像
void Widget::showQPixmap(const int &cameraIndex, const QPixmap &qPixmap)
{
    switch(cameraIndex){
    case Config::U :
        ui->camera_U->setPixmap(qPixmap);
        break;
    case Config::D :
        ui->camera_D->setPixmap(qPixmap);
        break;
    case Config::LF :
        ui->camera_LF->setPixmap(qPixmap);
        break;
    case Config::BR :
        ui->camera_BR->setPixmap(qPixmap);
        break;
    default:
        qDebug() << "相机索引超出范围";
        ui->appBtnResponse->setText("相机索引超出范围");
    }
}

// 当处于Debug模式时，将U号相机的画面传给其它没有相机的画布
int Widget::copyCamera_UToOthers(int cameraIndex, cv::Mat frame)
{
    for(auto it = m_camerasPtr.begin(); it != m_camerasPtr.end(); ++it){
        Camera *cam = (Camera*)it.value();
        cv::VideoCapture capture = cam->getM_Camera();
        if(!capture.isOpened()){    // 如果相机没有被打开
            if(is_Debug){           // 如果处于调试模式
                cam->setFrame(frame);
            }else{                  // 如果没处于调试模式
                cv::Mat emptyMat;
                cam->setFrame(emptyMat);
            }
        }
    }

    return cameraIndex;
}

// 创建相机哈希表
void Widget::createCamerasPtrHash(int num)
{
    for(int i = 0; i < num; ++i){
        Camera *cam = new Camera(i);
        m_camerasPtr.insert(i, cam);
    }
}

// 创建图像处理哈希表
void Widget::createColorProcessHash(int num)
{
    for(int i = 0; i < num; ++i){
        ColorProcess *process = new ColorProcess(i);
        m_colorProcessPtr.insert(i, process);
    }
}

void Widget::createThreadsHash(int num)
{
    for(int i = 0; i < num; ++i){
        QThread *thread = new QThread;
        m_threads.insert(i, thread);
    }
}

// 点击"打开相机"按钮
void Widget::on_openCameraBtn_clicked()
{
    if(!is_CameraOpen){
        ui->appBtnResponse->setText("正在打开所有可用相机，请稍等……");

        // 计算绿色边框交点坐标以及标记点坐标
        ui->camera_U->setFourPoints();
        ui->camera_U->calculateMarkPoints();
        ui->camera_D->setFourPoints();
        ui->camera_D->calculateMarkPoints();
        ui->camera_BR->setSixPoints();
        ui->camera_BR->calculateMarkPoints();
        ui->camera_LF->setSixPoints();
        ui->camera_LF->calculateMarkPoints();

        // 更新画布上的标记点坐标(根据绿色边框线的顶点坐标计算)
        ui->camera_U->calculateMarkPoints();
        ui->camera_D->calculateMarkPoints();
        ui->camera_LF->calculateMarkPoints();
        ui->camera_BR->calculateMarkPoints();

        createCamerasPtrHash(Config::CameraNum);    // 创建相机实例
        createColorProcessHash(Config::CameraNum);  // 创建图像处理实例
        createThreadsHash(Config::CameraNum);       // 创建线程实例

        // 打开摄像头
        for(auto it = m_camerasPtr.begin(); it != m_camerasPtr.end(); ++it){
            if(it.value() != nullptr){
                Camera *cam = (Camera*)it.value();
                cam->openCamera();
            }
        }

        is_CameraOpen = true;
        ui->appBtnResponse->setText("摄像头已被打开");

    }else{

        if(!m_camerasPtr.isEmpty()){
            qDebug() << "相机已经被打开";
            ui->appBtnResponse->setText("相机已经被打开");
        }
    }
}

// 点击"关闭相机"按钮
void Widget::on_closeCameraBtn_clicked()
{
    is_Connected = false;

    clearAllComponents();

    if(is_CameraOpen){
        // 停止信号源
        this->stopTimer();

        // 关闭摄像头硬件
        destoryAllCameras();

        // 停止线程
        closeAllThreads();

        // 清理UI
        clearAllComponents();

        is_CameraOpen = false;
    }else{
        qDebug() << "相机已经被关闭";
        ui->appBtnResponse->setText("相机已经被关闭");
    }
}

// 点击"调试模式"按钮
void Widget::on_debugBtn_clicked()
{
    if(is_Debug){
        is_Debug = false;

    }else{
        is_Debug = true;
    }

    // 清理组件信息
    clearAllComponents();
}

// 点击"重置参数"按钮
void Widget::on_resetHSVConfigBtn_clicked()
{
    // 从默认配置Config.c中加载阈值
    // 数字显示框组件名称
    QString color_name;
    QString HU_SpinBox_name;
    QString HL_SpinBox_name;
    QString SU_SpinBox_name;
    QString SL_SpinBox_name;
    QString VU_SpinBox_name;
    QString VL_SpinBox_name;

    // 组件指针
    QSpinBox *HU_ptr = nullptr;
    QSpinBox *HL_ptr = nullptr;
    QSpinBox *SU_ptr = nullptr;
    QSpinBox *SL_ptr = nullptr;
    QSpinBox *VU_ptr = nullptr;
    QSpinBox *VL_ptr = nullptr;

    // 数字显示框中的值
    int HU_value = 0;
    int HL_value = 0;
    int SU_value = 0;
    int SL_value = 0;
    int VU_value = 0;
    int VL_value = 0;

    for(int colorIndex = 0; colorIndex < Config::ColorNum; ++colorIndex){
        color_name = Config::colorOrder[colorIndex];

        HU_SpinBox_name = "h_UpperSpinBox_" + color_name;
        HL_SpinBox_name = "h_LowerSpinBox_" + color_name;
        SU_SpinBox_name = "s_UpperSpinBox_" + color_name;
        SL_SpinBox_name = "s_LowerSpinBox_" + color_name;
        VU_SpinBox_name = "v_UpperSpinBox_" + color_name;
        VL_SpinBox_name = "v_LowerSpinBox_" + color_name;

        HU_ptr = this->findChild<QSpinBox*>(HU_SpinBox_name);
        HL_ptr = this->findChild<QSpinBox*>(HL_SpinBox_name);
        SU_ptr = this->findChild<QSpinBox*>(SU_SpinBox_name);
        SL_ptr = this->findChild<QSpinBox*>(SL_SpinBox_name);
        VU_ptr = this->findChild<QSpinBox*>(VU_SpinBox_name);
        VL_ptr = this->findChild<QSpinBox*>(VL_SpinBox_name);

        if(HU_ptr == nullptr || HL_ptr == nullptr || SU_ptr == nullptr || SL_ptr == nullptr || VU_ptr == nullptr || VL_ptr == nullptr){
            qDebug() << "Widget::on_resetHSVConfigBtn_clicked--->空指针" << "\n";
            return;
        }

        HU_value = Config::colorThresholds[colorIndex].hUpper;
        HL_value = Config::colorThresholds[colorIndex].hLower;
        SU_value = Config::colorThresholds[colorIndex].sUpper;
        SL_value = Config::colorThresholds[colorIndex].sLower;
        VU_value = Config::colorThresholds[colorIndex].vUpper;
        VL_value = Config::colorThresholds[colorIndex].vLower;

        HU_ptr->setValue(HU_value);
        HL_ptr->setValue(HL_value);
        SU_ptr->setValue(SU_value);
        SL_ptr->setValue(SL_value);
        VU_ptr->setValue(VU_value);
        VL_ptr->setValue(VL_value);

    }

    // 计算轮廓线交点以及标记点默认坐标
    ui->camera_U->setFourPoints();
    ui->camera_U->calculateMarkPoints();
    ui->camera_D->setFourPoints();
    ui->camera_D->calculateMarkPoints();
    ui->camera_BR->setSixPoints();
    ui->camera_BR->calculateMarkPoints();
    ui->camera_LF->setSixPoints();
    ui->camera_LF->calculateMarkPoints();
}

// 默认自动创建路径的函数
bool openUrlWithAutoCreate(const QUrl &url) {
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

// 获取当前所有参数
void Widget::getAllThresholds(QJsonObject &jsonObj){
    if(m_colorProcessPtr.isEmpty()){
        return;
    }

    // 将colorprocess中的阈值加载到数字显示框中(默认使用U面的colorprocess)
    QHash<int, int> H_upper =  m_colorProcessPtr[Config::U]->getHUpperThreshold();
    QHash<int, int> H_lower =  m_colorProcessPtr[Config::U]->getHLowerThreshold();
    QHash<int, int> S_upper =  m_colorProcessPtr[Config::U]->getSUpperThreshold();
    QHash<int, int> S_lower =  m_colorProcessPtr[Config::U]->getSLowerThreshold();
    QHash<int, int> V_upper =  m_colorProcessPtr[Config::U]->getVUpperThreshold();
    QHash<int, int> V_lower =  m_colorProcessPtr[Config::U]->getVLowerThreshold();

    QString HU_name;
    QString HL_name;
    QString SU_name;
    QString SL_name;
    QString VU_name;
    QString VL_name;

    QSpinBox *HU_ptr = nullptr;
    QSpinBox *HL_ptr = nullptr;
    QSpinBox *SU_ptr = nullptr;
    QSpinBox *SL_ptr = nullptr;
    QSpinBox *VU_ptr = nullptr;
    QSpinBox *VL_ptr = nullptr;

    int HU_value = -1;
    int HL_value = -1;
    int SU_value = -1;
    int SL_value = -1;
    int VU_value = -1;
    int VL_value = -1;

    for(int i = 0; i < Config::ColorNum; ++i){
        QString color_name = Config::colorOrder[i];  // 得到当前索引的颜色

        // 构建组件名称
        HU_name = "h_UpperSpinBox_" + color_name;
        HL_name = "h_LowerSpinBox_" + color_name;
        SU_name = "s_UpperSpinBox_" + color_name;
        SL_name = "s_LowerSpinBox_" + color_name;
        VU_name = "v_UpperSpinBox_" + color_name;
        VL_name = "v_LowerSpinBox_" + color_name;

        // 创建指向组件的指针
        HU_ptr  = this->findChild<QSpinBox*>(HU_name);
        HL_ptr  = this->findChild<QSpinBox*>(HL_name);
        SU_ptr  = this->findChild<QSpinBox*>(SU_name);
        SL_ptr  = this->findChild<QSpinBox*>(SL_name);
        VU_ptr  = this->findChild<QSpinBox*>(VU_name);
        VL_ptr  = this->findChild<QSpinBox*>(VL_name);

        if(HU_ptr == nullptr || HL_ptr == nullptr || SU_ptr == nullptr || SL_ptr  == nullptr || VU_ptr  == nullptr || VL_ptr == nullptr){
            qDebug() << "Widget::bindColorThresholdsToSpinBox--->空指针" << "\n";
            return;
        }

        // 通过指针设定组件的值
        HU_value =  HU_ptr->value();
        HL_value = HL_ptr->value();
        SU_value = SU_ptr->value();
        SL_value = SL_ptr->value();
        VU_value = VU_ptr->value();
        VL_value = VL_ptr->value();

        // 写入json文件
        jsonObj[HU_name] = HU_value;
        jsonObj[HL_name] = HL_value;
        jsonObj[SU_name] = SU_value;
        jsonObj[SL_name] = SL_value;
        jsonObj[VU_name] = VU_value;
        jsonObj[VL_name] = VL_value;

    }

    // 获取绿色轮廓线的坐标
    QVector<QPoint> U_points = ui->camera_U->getPoints();
    QVector<QPoint> D_points = ui->camera_D->getPoints();
    QVector<QPoint> LF_points = ui->camera_LF->getPoints();
    QVector<QPoint> BR_points = ui->camera_BR->getPoints();

    // 创建轮廓点的JSON结构
    QJsonObject contoursObj;

    // 将QVector<QPoint>转换为QJsonArray的lambda函数
    auto pointsToJson = [](const QVector<QPoint>& points) {
        QJsonArray arr;
        for (const QPoint& p : points) {
            arr.append(QJsonArray{p.x(), p.y()});
        }
        return arr;
    };

    // 存储各面轮廓点（保持你的原始变量名）
    contoursObj["U"] = pointsToJson(U_points);
    contoursObj["D"] = pointsToJson(D_points);
    contoursObj["LF"] = pointsToJson(LF_points);
    contoursObj["BR"] = pointsToJson(BR_points);

    // 将轮廓数据合并到主JSON对象
    jsonObj["contours"] = contoursObj;
}

/******************************计时器*****************************/
void Widget::setTimerPtr(QTimer *timer)
{
    m_timer = timer;
}

QTimer* Widget::getTimerPtr()
{
    return m_timer;
}

void Widget::startTimer()
{
    m_timer->start();
}

void Widget::stopTimer()
{
    if(m_timer->isActive()){
        m_timer->stop();
    }
}

// 点击"保存参数"按钮
void Widget::on_saveHSVConfigBtn_clicked() {
    // 1. 准备路径和文件名
    QString fileName = QString("HSVConfig_%1.json")
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd__hh_mm_ss"));
    QString dirPath = Config::ConfigPath;
    QString fullPath = QDir(dirPath).filePath(fileName);

    // 2. 确保目录存在
    QUrl dirUrl = QUrl::fromLocalFile(dirPath);
    if (!openUrlWithAutoCreate(dirUrl)) {
        QMessageBox::critical(this, "错误", "无法创建配置目录");
        return;
    }

    // 3. 写入JSON文件
    QJsonObject jsonConfig;
    getAllThresholds(jsonConfig);   // 获取所有参数

    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "错误", "无法创建配置文件");
        return;
    }
    file.write(QJsonDocument(jsonConfig).toJson());
    file.close();

    // 5. 提示
    QMessageBox::information(this, "成功",
        QString("配置已保存为:\n%1").arg(fileName),
        QMessageBox::Ok);
}

// 点击"加载参数"按钮
void Widget::on_loadHSVConfigBtn_clicked()
{
    // 1. 弹出文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择HSV配置文件",
        Config::ConfigPath,  // 默认打开配置目录
        "JSON文件 (*.json);;所有文件 (*.*)"
    );

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

    // 3. 解析并应用参数
    QString color_name;
    QString HU_name, HL_name, SU_name, SL_name, VU_name, VL_name;
    QSpinBox *HU_ptr = nullptr, *HL_ptr = nullptr, *SU_ptr = nullptr,
             *SL_ptr = nullptr, *VU_ptr = nullptr, *VL_ptr = nullptr;

    bool hasError = false;

    for (int i = 0; i < Config::ColorNum; ++i) {
        color_name = Config::colorOrder[i];

        // 构建组件名称
        HU_name = "h_UpperSpinBox_" + color_name;
        HL_name = "h_LowerSpinBox_" + color_name;
        SU_name = "s_UpperSpinBox_" + color_name;
        SL_name = "s_LowerSpinBox_" + color_name;
        VU_name = "v_UpperSpinBox_" + color_name;
        VL_name = "v_LowerSpinBox_" + color_name;

        // 获取组件指针
        HU_ptr = findChild<QSpinBox*>(HU_name);
        HL_ptr = findChild<QSpinBox*>(HL_name);
        SU_ptr = findChild<QSpinBox*>(SU_name);
        SL_ptr = findChild<QSpinBox*>(SL_name);
        VU_ptr = findChild<QSpinBox*>(VU_name);
        VL_ptr = findChild<QSpinBox*>(VL_name);

        // 检查指针和JSON键是否存在
        if (!HU_ptr || !HL_ptr || !SU_ptr || !SL_ptr || !VU_ptr || !VL_ptr ||
            !jsonObj.contains(HU_name) || !jsonObj.contains(HL_name) ||
            !jsonObj.contains(SU_name) || !jsonObj.contains(SL_name) ||
            !jsonObj.contains(VU_name) || !jsonObj.contains(VL_name)) {
            hasError = true;
            continue;
        }

        // 设置组件值(值改变自动触发信号改变colorprocess中的阈值)
        HU_ptr->setValue(jsonObj[HU_name].toInt());
        HL_ptr->setValue(jsonObj[HL_name].toInt());
        SU_ptr->setValue(jsonObj[SU_name].toInt());
        SL_ptr->setValue(jsonObj[SL_name].toInt());
        VU_ptr->setValue(jsonObj[VU_name].toInt());
        VL_ptr->setValue(jsonObj[VL_name].toInt());
    }

    // 加载轮廓点数据
    if (jsonObj.contains("contours")) {
        QJsonObject contoursObj = jsonObj["contours"].toObject();

        // 从JSON恢复点数据的lambda函数
        auto jsonToPoints = [](const QJsonArray& arr) -> QVector<QPoint> {
            QVector<QPoint> points;
            points.reserve(arr.size());
            for (const QJsonValue& val : arr) {
                QJsonArray xy = val.toArray();
                if (xy.size() == 2) {
                    points.append(QPoint(xy[0].toInt(), xy[1].toInt()));
                }
            }
            return points;
        };

        // 加载各面轮廓点
        if (contoursObj.contains("U")) {
            ui->camera_U->setPoints(jsonToPoints(contoursObj["U"].toArray()));
        }
        if (contoursObj.contains("D")) {
            ui->camera_D->setPoints(jsonToPoints(contoursObj["D"].toArray()));
        }
        if (contoursObj.contains("LF")) {
            ui->camera_LF->setPoints(jsonToPoints(contoursObj["LF"].toArray()));
        }
        if (contoursObj.contains("BR")) {
            ui->camera_BR->setPoints(jsonToPoints(contoursObj["BR"].toArray()));
        }

        // 重新计算标记点
        ui->camera_U->calculateMarkPoints();
        ui->camera_D->calculateMarkPoints();
        ui->camera_LF->calculateMarkPoints();
        ui->camera_BR->calculateMarkPoints();
    }

    // 错误处理
    if (hasError) {
        QMessageBox::warning(this, "警告", "部分参数加载失败（文件格式不匹配）");
    } else {
        QMessageBox::information(this, "成功", "HSV参数和轮廓点已加载");
    }
}

void Widget::on_getColorBtn_clicked()
{
    if(is_GetColor){
        is_GetColor = false;
    }else{
        is_GetColor = true;
    }
}
