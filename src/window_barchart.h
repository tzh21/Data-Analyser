#ifndef WINDOW_BARCHART_H
#define WINDOW_BARCHART_H

#include "widget.h"

#include <QMainWindow>
#include <QChartView>
#include <QBarSeries>
#include <QLineSeries>
#include <QSplineSeries>

class Window_Barchart : public QMainWindow
{
    Q_OBJECT
public:
//    explicit Window_Barchart(Widget *parent = nullptr);
    explicit Window_Barchart(bool is_discrete, const QList<float> &data, Widget *parent = nullptr);
//    explicit Window_Barchart(QWidget *parent = nullptr);

signals:

private:
    QChart *chart{new QChart};
    QChartView *chartView{new QChartView(chart)};

    QBarSeries *barSeries{new QBarSeries()};
//    QLineSeries *lineSeries{new QLineSeries()};
    QSplineSeries *lineSeries{new QSplineSeries()};

    void on_check_bar(int state);

    void on_check_line(int state);
};

#endif // WINDOW_BARCHART_H
