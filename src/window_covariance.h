#ifndef WINDOW_COVARIANCE_H
#define WINDOW_COVARIANCE_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPainter>

class Window_Covariance : public QMainWindow
{
    Q_OBJECT
public:
//    explicit Window_Covariance(QWidget *parent = nullptr);
    explicit Window_Covariance(const std::vector<std::vector<float>> &variants,
                               const QStringList &headers,
                               QWidget *parent = nullptr);

signals:

private:
    friend class ColorBarWidget;

    const int cellSize = 100;

    size_t matrixSize = 0;

    QTableWidget *table = nullptr;

    std::vector<std::vector<float>> cov;
    std::vector<std::vector<float>> corr;
    std::vector<float> var;

    static QColor mapValueToColor(double value);

    void on_cov_toggled(bool checked);
    void on_corr_toggled(bool checked);
};

/**
 * @brief 颜色条部件
 * 
 */
class ColorBarWidget : public QWidget {
public:
    ColorBarWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(300, 60);
    }

    void paintEvent(QPaintEvent *) override {
        // 创建一个QPainter对象来进行绘制
        QPainter painter(this);

        int barWidth = 400;
        int barHeight = 20;
        int barX = (width() - barWidth) / 2;
        int barY = (height() - barHeight) / 2;
        QRect barRect(barX, barY, barWidth, barHeight);

        // 创建线性渐变
        QLinearGradient gradient(barRect.topLeft(), barRect.topRight());

        // 计算color bar的颜色段数
        int numSegments = 4;

        // 添加颜色段到渐变中
        for (int i = 0; i < numSegments; ++i) {
            // 计算当前颜色段的起始位置和结束位置
            qreal startStop = static_cast<qreal>(i) / numSegments;
            QColor color = Window_Covariance::mapValueToColor(startStop);
            gradient.setColorAt(startStop, color);

            // 绘制刻度标签
            qreal cov = (startStop - 0.5) * 2;
            QString labelText = QString::number(cov, 'f', 2); // 将浮点数格式化为字符串
            QRectF labelRect(barRect.left() + startStop * barWidth - 15, barRect.top() - 20, 30, 20);
            painter.drawText(labelRect, Qt::AlignCenter, labelText);
        }
        QString labelText = QString::number(1.0, 'f', 2); // 将浮点数格式化为字符串
        QRectF labelRect(barRect.left() + 1.0 * barWidth - 15, barRect.top() - 20, 30, 20);
        painter.drawText(labelRect, Qt::AlignCenter, labelText);

        // 创建一个矩形，用渐变填充
        QRectF gradientRect(barRect);
        painter.fillRect(gradientRect, gradient);
    }
};

#endif // WINDOW_COVARIANCE_H
