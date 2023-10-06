#ifndef WINDOW_SCATTER_H
#define WINDOW_SCATTER_H

#include <QMainWindow>
#include <QChart>
#include <QChartView>
#include <QLineEdit>
#include <QSplineSeries>

class Window_Scatter : public QMainWindow
{
    Q_OBJECT
public:
//    Window_Scatter() = default;
    explicit Window_Scatter(const std::vector<float>& vecX, const std::vector<float>& vecY,
                            const QString &headerX, const QString &headerY,
                            QWidget *parent = nullptr);

signals:

private:
    const std::vector<float> vecX;
    const std::vector<float> vecY;
    const size_t cnt_input = 10;
    std::vector<float> input;

    int inDegree = 3;

    QChart *chart{new QChart};
    QChartView *chartView{new QChartView(chart)};

    QLineEdit *edit_degree = nullptr;
    QLineEdit *edit_p = nullptr;
    QLineEdit *edit_r = nullptr;
    QLineEdit *edit_x = nullptr;
    QLineEdit *edit_y = nullptr;

    QSplineSeries *lineSeries = nullptr;

    void on_button_degree_clicked();
    void onPointHovered(const QPointF &point, bool state);
};

#endif // WINDOW_SCATTER_H
