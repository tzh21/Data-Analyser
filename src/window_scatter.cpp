#include "window_scatter.h"
#include "include/common_utils.h"

#include <include/needed_algo/leastsquare.hpp>
#include <Eigen/Dense>

#include <QScatterSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QPushButton>
#include <QValidator>
#include <QLabel>
#include <QToolTip>
#include <QGroupBox>
#include <QFormLayout>

/**
 * @brief Construct a new Window_Scatter::Window_Scatter object
 * 
 * @param _vecX x轴数据。
 * @param _vecY y轴数据。
 * @param headerX x轴名称。
 * @param headerY y轴名称。
 * @param parent 
 */
Window_Scatter::Window_Scatter(const std::vector<float>& _vecX, const std::vector<float>& _vecY,
                               const QString &headerX, const QString &headerY,
                               QWidget *parent)
    :QMainWindow(parent), vecX(_vecX), vecY(_vecY), input(cnt_input)
{
    setAttribute(Qt::WA_DeleteOnClose);
//    布局

    resize(800, 600);

    if (parent != nullptr){
        resize(parent->size());
    }

    auto central = new QWidget(this);
    setCentralWidget(central);

    auto layout_central = new QVBoxLayout(central);

    auto layout_tool = new QHBoxLayout;
    layout_central->addLayout(layout_tool);

    layout_central->addWidget(chartView);

    auto group_degree = new QGroupBox("阶数");
    layout_tool->addWidget(group_degree);

    auto group_coor = new QGroupBox("坐标");
    layout_tool->addWidget(group_coor);

    auto layout_degree = new QFormLayout(group_degree);

    edit_degree = new QLineEdit("3");
    auto label_degree = new QLabel("阶数");
    layout_degree->addRow(label_degree, edit_degree);

    auto button_degree = new QPushButton("设置阶数");
    layout_degree->addWidget(button_degree);

    auto group_stat = new QGroupBox("统计量");
    layout_tool->addWidget(group_stat);

    auto layout_stat = new QFormLayout(group_stat);

    auto label_p = new QLabel("p-value");
    edit_p = new QLineEdit;
    layout_stat->addRow(label_p, edit_p);

    auto label_r = new QLabel("R squared");
    edit_r = new QLineEdit;
    layout_stat->addRow(label_r, edit_r);

    auto layout_coor = new QFormLayout(group_coor);

    auto label_x = new QLabel("x");
    edit_x = new QLineEdit;
    layout_coor->addRow(label_x, edit_x);
    auto label_y = new QLabel("y");
    edit_y = new QLineEdit;
    layout_coor->addRow(label_y, edit_y);

    connect(button_degree, &QPushButton::clicked,
           this, &Window_Scatter::on_button_degree_clicked);

//    计算并绘图

    Eigen::VectorXf coe;
    float p, R_squared;
    std::tie(coe, p, R_squared) = fitLeastSquareAndPR(vecX, vecY, inDegree);
    edit_p->setText(QString::number(p));
    edit_r->setText(QString::number(R_squared));

    chartView->setRenderHint(QPainter::Antialiasing);

//    散点图
    auto pointSeries = new QScatterSeries(this);
    pointSeries->setName("散点图");
    beautify_scatter_series(pointSeries);

    for (size_t i = 0; i < vecX.size() && i < vecY.size(); i++){
        pointSeries->append(vecX[i], vecY[i]);
    }
    chart->addSeries(pointSeries);

    connect(pointSeries, &QScatterSeries::hovered, this, &Window_Scatter::onPointHovered);

//    曲线图
    lineSeries = new QSplineSeries(this);
    lineSeries->setName("拟合曲线");

    float min_x = *std::min_element(vecX.begin(), vecX.end());
    float max_x = *std::max_element(vecX.begin(), vecX.end());
    const float step_len = (max_x - min_x) / (cnt_input - 1);
    for (size_t i = 0; i < cnt_input; i++) {
        input[i] = min_x + i * step_len;
    }

    for (size_t idx_input = 0; idx_input < input.size(); idx_input++){
        float x = input[idx_input];
        float y{0};
        for (int i = 0; i < coe.size(); i++) {
            y += coe(i) * std::pow(x, i);
        }
        lineSeries->append(x, y);
    }
    chart->addSeries(lineSeries);

    auto axisX = new QValueAxis(this);
    axisX->setTitleText(headerX);
    chart->addAxis(axisX, Qt::AlignBottom);
    pointSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisX);

    auto axisY = new QValueAxis(this);
    axisY->setTitleText(headerY);
    chart->addAxis(axisY, Qt::AlignLeft);
    pointSeries->attachAxis(axisY);
    lineSeries->attachAxis(axisY);
}

/**
 * @brief 显示拟合曲线。
 * 
 */
void Window_Scatter::on_button_degree_clicked(){
    bool valid = false;
    const int backup = inDegree;
    inDegree = edit_degree->text().toInt(&valid);

    if (!valid){
        inDegree = backup;
        qDebug() << "toInt() invalid";
        return;
    }

    Eigen::VectorXf coe;
    float p, R_squared;
    std::tie(coe, p, R_squared) = fitLeastSquareAndPR(vecX, vecY, inDegree);
    edit_p->setText(QString::number(p));
    edit_r->setText(QString::number(R_squared));

    lineSeries->clear();

    for (size_t idx_input = 0; idx_input < input.size(); idx_input++){
        float x = input[idx_input];
        float y{0};
        for (int i = 0; i < coe.size(); i++) {
            y += coe(i) * std::pow(x, i);
        }
        lineSeries->append(x, y);
    }
}

/**
 * @brief 鼠标悬停在点上方时显示坐标值。
 * 
 * @param point 鼠标悬停的点。
 * @param state 鼠标悬停状态。
 */
void Window_Scatter::onPointHovered(const QPointF &point, bool state) {
    if (state) {
        // 鼠标悬停在点上方，显示坐标值
        QPoint tooltipPos(point.x(), point.y() - 20);
//        QToolTip::showText(mapToGlobal(tooltipPos), QString("X: %1, Y: %2").arg(point.x()).arg(point.y()));
        edit_x->setText(QString::number(point.x()));
        edit_y->setText(QString::number(point.y()));
    }
    // else {
    //     edit_x->clear();
    //     edit_y->clear();
    // }
}
