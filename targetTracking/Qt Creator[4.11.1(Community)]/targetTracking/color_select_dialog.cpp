#include "color_select_dialog.h"
#include <QDebug>

Color_Select_Dialog::Color_Select_Dialog(QWidget *parent)
    : QDialog(parent)
    , selectedColorName("") {
    setupUI();
    setWindowTitle("选择追踪颜色");
    setFixedSize(300, 200);
}

void Color_Select_Dialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QLabel *titleLabel      = new QLabel("请选择要追踪的颜色:", this);
    mainLayout->addWidget(titleLabel);

    // 创建颜色按钮
    QHBoxLayout *firstRow  = new QHBoxLayout();
    QHBoxLayout *secondRow = new QHBoxLayout();

    // 定义六种颜色及其名称
    QList<QPair<QString, QColor>> colors = {
        {"Red", Qt::red},
        {"Green", Qt::green},
        {"Blue", Qt::blue},
        {"Yellow", Qt::yellow},
        {"Orange", QColor(255, 165, 0)}, // 橙色
        {"Purple", QColor(128, 0, 128)}  // 紫色
    };

    // 创建颜色按钮
    for (int i = 0; i < colors.size(); i++) {
        QPushButton *colorBtn = new QPushButton(this);
        colorBtn->setFixedSize(40, 40);
        colorBtn->setStyleSheet(QString("background-color: %1; border: none;")
                                    .arg(colors[i].second.name()));
        colorBtn->setToolTip(colors[i].first);

        // 存储按钮与颜色名称的映射
        colorButtons[colorBtn] = colors[i].first;

        // 连接信号槽
        connect(colorBtn, &QPushButton::clicked, this, &Color_Select_Dialog::onColorButtonClicked);

        // 添加到布局
        if (i < 3) {
            firstRow->addWidget(colorBtn);
        } else {
            secondRow->addWidget(colorBtn);
        }
    }

    mainLayout->addLayout(firstRow);
    mainLayout->addLayout(secondRow);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void Color_Select_Dialog::onColorButtonClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (clickedButton && colorButtons.contains(clickedButton)) {
        selectedColorName = colorButtons[clickedButton];
        accept(); // 关闭对话框并返回Accepted
    }
}

QString Color_Select_Dialog::getSelectedColorName() const {
    return selectedColorName;
}
