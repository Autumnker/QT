#include "mylabel.h"
#include <QPainter>
#include <QMouseEvent>
#include "config.h"

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
    setSixPoints();         //  生成顶点
    calculateMarkPoints();  //  生成标记点
}

void MyLabel::setFourPoints()
{
    // 清除已有交点坐标
    m_points.clear();

    QRect labelRect = rect();

    // 上
    QPoint top = QPoint(labelRect.width()/2, 0);
    // 左
    QPoint left = QPoint(0, labelRect.height()/2);
    // 右
    QPoint right = QPoint(labelRect.width(), labelRect.height()/2);
    // 下
    QPoint down = QPoint(labelRect.width()/2, labelRect.height());

    m_points.push_back(top);
    m_points.push_back(left);
    m_points.push_back(right);
    m_points.push_back(down);

    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);

    update();
}

void MyLabel::setSixPoints()
{
    m_points.clear();

    QRect labelRect = rect();

    // 左上
    QPoint leftTop = QPoint(labelRect.width()/8, /*labelRect.height()/10*/0);
    // 右上
    QPoint rightTop = QPoint(labelRect.width()/8*7, /*labelRect.height()/10*/0);
    // 左
    QPoint left = QPoint(/*labelRect.width()/7*/0, labelRect.height()/2);
    // 右
    QPoint right = QPoint(labelRect.width()/*/7*6*/, labelRect.height()/2);
    // 左下
    QPoint leftDown = QPoint(labelRect.width()/8, labelRect.height()/*/10*9*/);
    // 右下
    QPoint rightDown = QPoint(labelRect.width()/8*7, labelRect.height()/*/10*9*/);

    m_points.push_back(leftTop);
    m_points.push_back(rightTop);
    m_points.push_back(left);
    m_points.push_back(right);
    m_points.push_back(leftDown);
    m_points.push_back(rightDown);

    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);
    m_isDraggingPoints.push_back(false);

    update();
}

// 是否点击了某个点
bool MyLabel::isPressPoint(QPoint &clickPoint,QPoint &point, bool& isDragging_x_Point, int threshold)
{
    if((clickPoint - point).manhattanLength() <= threshold){
        m_isDragging = true;
        isDragging_x_Point = true;

        return true;
    }

    return false;
}

// 计算两条直线的交点
QPoint MyLabel::calculateIntersection(const QPoint& p1, const QPoint& p2, const QPoint& p3, const QPoint& p4) {
    double x1 = p1.x();
    double y1 = p1.y();
    double x2 = p2.x();
    double y2 = p2.y();
    double x3 = p3.x();
    double y3 = p3.y();
    double x4 = p4.x();
    double y4 = p4.y();

    double denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denominator == 0) {
        // 两条直线平行，没有交点
        return QPoint();
    }

    double px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denominator;
    double py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denominator;

    return QPoint(px, py);
}

// 计算四边形四条边的三等分点
QVector<QPoint> MyLabel::calculateEdgeTrisectPoints(const QPoint& p1, const QPoint& p2) {
    QVector<QPoint> trisectPoints;
    double dx = (p2.x() - p1.x()) / 3.0;
    double dy = (p2.y() - p1.y()) / 3.0;

    trisectPoints.append(QPoint(p1.x() + dx, p1.y() + dy));
    trisectPoints.append(QPoint(p1.x() + 2 * dx, p1.y() + 2 * dy));
    return trisectPoints;
}

// 计算四边形划分成 9 个部分后每个部分的中心点坐标
QVector<QVector<QPoint>> MyLabel::calculateSquareMarkPoints(const QPoint& TL, const QPoint& TR, const QPoint& DL, const QPoint& DR) {
    QVector<QVector<QPoint>> result(3, QVector<QPoint>(3));

    // 计算四条边的三等分点
    QVector<QPoint> topTrisectPoints = calculateEdgeTrisectPoints(TL, TR);
    QVector<QPoint> leftTrisectPoints = calculateEdgeTrisectPoints(TL, DL);
    QVector<QPoint> rightTrisectPoints = calculateEdgeTrisectPoints(TR, DR);
    QVector<QPoint> bottomTrisectPoints = calculateEdgeTrisectPoints(DL, DR);

    // 计算所有交点坐标
    QVector<QVector<QPoint>> intersectionPoints(4, QVector<QPoint>(4));
    intersectionPoints[0][0] = TL;
    intersectionPoints[0][1] = topTrisectPoints[0];
    intersectionPoints[0][2] = topTrisectPoints[1];
    intersectionPoints[0][3] = TR;
    intersectionPoints[1][0] = leftTrisectPoints[0];
    intersectionPoints[1][3] = rightTrisectPoints[0];
    intersectionPoints[2][0] = leftTrisectPoints[1];
    intersectionPoints[2][3] = rightTrisectPoints[1];
    intersectionPoints[3][0] = DL;
    intersectionPoints[3][1] = bottomTrisectPoints[0];
    intersectionPoints[3][2] = bottomTrisectPoints[1];
    intersectionPoints[3][3] = DR;

    intersectionPoints[1][1] = calculateIntersection(intersectionPoints[1][0], intersectionPoints[1][3], intersectionPoints[0][1], intersectionPoints[3][1]);
    intersectionPoints[1][2] = calculateIntersection(intersectionPoints[1][0], intersectionPoints[1][3], intersectionPoints[0][2], intersectionPoints[3][2]);
    intersectionPoints[2][1] = calculateIntersection(intersectionPoints[2][0], intersectionPoints[2][3], intersectionPoints[0][1], intersectionPoints[3][1]);
    intersectionPoints[2][2] = calculateIntersection(intersectionPoints[2][0], intersectionPoints[2][3], intersectionPoints[0][2], intersectionPoints[3][2]);

    for(int i =0 ; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            double centerX = (intersectionPoints[i][j].x() + intersectionPoints[i][j + 1].x() + intersectionPoints[i + 1][j].x() + intersectionPoints[i + 1][j + 1].x()) / 4;
            double centerY = (intersectionPoints[i][j].y() + intersectionPoints[i][j + 1].y() + intersectionPoints[i + 1][j].y() + intersectionPoints[i + 1][j + 1].y()) / 4;
            result[i][j] = QPoint(centerX,centerY);
        }
    }

    return result;
}

// 计算每个视角的圆的中心点(标记点)坐标
void MyLabel::calculateMarkPoints()
{
    if(m_points.size() == Config::U_D){
        m_marking = calculateSquareMarkPoints(m_points[Config::Top], m_points[Config::Right], m_points[Config::Left], m_points[Config::Down]);
    }else if(m_points.size() == Config::LF_BR){
        QVector<QVector<QPoint>> top = calculateSquareMarkPoints(m_points[Config::TopLeft], m_points[Config::TopRight], m_points[Config::MidLeft], m_points[Config::MidRight]);
        QVector<QVector<QPoint>> button = calculateSquareMarkPoints(m_points[Config::MidLeft], m_points[Config::MidRight], m_points[Config::DownLeft], m_points[Config::DownRight]);

        // 清除原有的标记点
        m_marking.clear();

        // 添加 top 的所有行
        for (const auto& row : top) {
            m_marking.append(row);
        }

        // 添加 button 的所有行
        for (const auto& row : button) {
            m_marking.append(row);
        }
    }
}


QVector<QPoint> MyLabel::getPoints()
{
    return m_points;
}

void MyLabel::setPoints(QVector<QPoint> points)
{
    m_points = points;
}

QVector<bool> MyLabel::getIsDraggingPoints()
{
    return m_isDraggingPoints;
}

void MyLabel::setIsDraggingPoints(QVector<bool> isDraggingPoints)
{
    m_isDraggingPoints = isDraggingPoints;
}

bool MyLabel::getIsDragging()
{
    return m_isDragging;
}

void MyLabel::setIsDragging(bool isDragging)
{
    m_isDragging = isDragging;
}

QVector<QVector<QPoint>> MyLabel::getMarking()
{
    return m_marking;
}

void MyLabel::setMarking(QVector<QVector<QPoint>>& markPoints)
{
    m_marking = markPoints;
}

using namespace Config;
void MyLabel::paintEvent(QPaintEvent *event)
{
    // 调用基类的 paintEvent 方法进行正常绘制
    QLabel::paintEvent(event);

    // 创建 QPainter 对象
    QPainter painter(this);
    // 设置画笔颜色为绿色，线宽为 ? 像素
    painter.setPen(QPen(Qt::green, LineWidth));
    // 绘制边框
    if(m_points.size() == U_D){
        painter.drawLine(m_points[Top], m_points[Left]);
        painter.drawLine(m_points[Top], m_points[Right]);
        painter.drawLine(m_points[Down], m_points[Left]);
        painter.drawLine(m_points[Down], m_points[Right]);
    }else if(m_points.size() == LF_BR){
        painter.drawLine(m_points[TopLeft], m_points[TopRight]);
        painter.drawLine(m_points[TopLeft], m_points[MidLeft]);
        painter.drawLine(m_points[TopRight], m_points[MidRight]);
        painter.drawLine(m_points[MidLeft], m_points[MidRight]);
        painter.drawLine(m_points[MidLeft], m_points[DownLeft]);
        painter.drawLine(m_points[MidRight], m_points[DownRight]);
        painter.drawLine(m_points[DownLeft], m_points[DownRight]);
    }

    // 设置画笔颜色、线宽、半径、用于绘制圆圈
    painter.setPen(QPen(Qt::red, CircleWidth));

    // 遍历 m_marking 中的每个点
    for(const auto& row : m_marking){
        for(const auto& point : row){
            // 以该点为圆心，x 像素为半径画圆
            painter.drawEllipse(point, CircleRadius, CircleRadius);
        }
    }
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint clickPos = event->pos();
        int threshold = 10; // 点击点与端点的最大距离阈值

        for(int i = 0; i < m_points.size(); ++i){
            if(isPressPoint(clickPos, m_points[i], m_isDraggingPoints[i], threshold)){
                break;
            }
        }
    }
}

void MyLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        QPoint newPos = event->pos();
        QRect labelRect = rect();

        if(labelRect.contains(newPos)){
            for(int i = 0; i < m_isDraggingPoints.size(); ++i){
                if(m_isDraggingPoints[i]){
                    m_points[i] = newPos;
                    break;
                }
            }
        }
        update(); // 触发重绘
        calculateMarkPoints();  // 重新计算标记点
    }
}

void MyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;

        for(int i = 0; i < m_isDraggingPoints.size(); ++i){
            m_isDraggingPoints[i] = false;
        }
    }
}
