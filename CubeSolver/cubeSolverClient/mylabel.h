#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QPoint>
#include <QVector>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = nullptr);

    // 设置四个坐标点
    void setFourPoints();

    // 设置六个坐标点
    void setSixPoints();

    // 是否点击了某个点
    bool isPressPoint(QPoint& clickPoint, QPoint& point, bool& isDragging_x_Point, int threshold);

    // 通过两直线的四个点计算其交点坐标
    QPoint calculateIntersection(const QPoint& p1, const QPoint& p2, const QPoint& p3, const QPoint& p4);

    // 计算四边形四条边的三等分点
    QVector<QPoint> calculateEdgeTrisectPoints(const QPoint& p1, const QPoint& p2);

    // 计算四边形的九个标记点
    QVector<QVector<QPoint>> calculateSquareMarkPoints(const QPoint& TL, const QPoint& TR, const QPoint& DL, const QPoint& DR);

    // 统计标记点坐标
    void calculateMarkPoints();

    // getter和setter方法
    QVector<QPoint> getPoints();
    void setPoints(QVector<QPoint> points);
    QVector<bool> getIsDraggingPoints();
    void setIsDraggingPoints(QVector<bool> isDraggingPoints);
    bool getIsDragging();
    void setIsDragging(bool isDragging);
    QVector<QVector<QPoint>> getMarking();
    void setMarking(QVector<QVector<QPoint>>& markPoints);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:

private:
    QVector<QPoint> m_points;
    QVector<bool> m_isDraggingPoints;
    bool m_isDragging;
    QVector<QVector<QPoint>> m_marking;

};

#endif // MYLABEL_H
