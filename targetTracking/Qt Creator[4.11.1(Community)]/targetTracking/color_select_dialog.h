#ifndef COLOR_SELECTION_DIALOG_H
#define COLOR_SELECTION_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QColor>
#include <QString>
#include <QMap>

class Color_Select_Dialog : public QDialog {
    Q_OBJECT

public:
    explicit Color_Select_Dialog(QWidget *parent = nullptr);
    QString getSelectedColorName() const;

private slots:
    void onColorButtonClicked();

private:
    void setupUI();
    QMap<QPushButton *, QString> colorButtons;
    QString selectedColorName;
};

#endif // COLOR_SELECTION_DIALOG_H
