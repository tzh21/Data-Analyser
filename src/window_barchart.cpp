#include "window_barchart.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QBarSet>
#include <QValueAxis>
#include <QBarCategoryAxis>

/**
 * @brief Construct a new Window_Barchart::Window_Barchart object
 * 
 * @param is_discrete 选取列的数值是否离散。若离散则采取不同的直方图分组策略，且不绘制正态分布密度曲线。
 * @param columnData 列的数据。
 * @param parent 
 */
Window_Barchart::Window_Barchart(bool is_discrete, const QList<float> &columnData, Widget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
//    布局

//    resize(800, 600);

    auto central = new QWidget();
    setCentralWidget(central);

    auto layout_main = new QVBoxLayout(central);
    central->setLayout(layout_main);

    auto layout_checkbox = new QHBoxLayout();
    layout_main->addLayout(layout_checkbox);

    auto check_barchart = new QCheckBox("直方图", this);
    auto check_linechart = new QCheckBox("折线图", this);
    layout_checkbox->addWidget(check_barchart);
    layout_checkbox->addWidget(check_linechart);

    connect(check_barchart, &QCheckBox::stateChanged,
            this, &Window_Barchart::on_check_bar);
    connect(check_linechart, &QCheckBox::stateChanged,
            this, &Window_Barchart::on_check_line);

//    绘图

//    绘制频次直方图

    layout_main->addWidget(chartView);
    chartView->setMinimumSize(1200, 600);

//    将数据等距分成8组
//    区分离散
    const int cnt_set = is_discrete ? 2 : 8;
    const float minValue = is_discrete ? 0 : *std::min_element(columnData.begin(), columnData.end());
    const float maxValue = is_discrete ? 1 : *std::max_element(columnData.begin(), columnData.end());
    const float binWidth = (maxValue - minValue) / cnt_set;

//    统计每个组的频次
    QVector<int> frequencies(cnt_set, 0);
    if (is_discrete) {
        for (float value : columnData) {
            int ivalue = static_cast<int>(value);
            frequencies[ivalue]++;
        }
    }
    else{
        for (float value : columnData) {
            int bin = static_cast<int>((value - minValue) / binWidth);
            if (bin >= 0 && bin < frequencies.size()) {
                frequencies[bin]++;
            }
        }
    }


//    直方图添加到图中
    chart->addSeries(barSeries);
    auto barSet = new QBarSet("Bar", this);

//    将频次添加到直方图中
    for (int i = 0; i < frequencies.size(); ++i) {
        *barSet << frequencies[i];
    }
    barSeries->append(barSet);

//    创建横轴
//    区分离散
    auto axisX = new QBarCategoryAxis(this);
    axisX->setTitleText("Range");
    if (is_discrete){
        axisX->append("Benign");
        axisX->append("Malignant");
    }
    else{
        for (int i = 0; i < frequencies.size(); ++i) {
            axisX->append(
                QString::asprintf("%.*g", 4, i * binWidth + minValue) +
                "-" +
                QString::asprintf("%.*g", 4, (i + 1) * binWidth + minValue)
            );
        }
    }
    chart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

//    创建纵轴
//    区分离散
    auto axisY = new QValueAxis(this);
    axisY->setTitleText("Frequency");
    axisY->setRange(0, columnData.size());
    chart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

//    绘制折线图

//    添加折线图
    lineSeries->setName("Line");
    chart->addSeries(lineSeries);

//    添加数据点，连接直方图的顶部
//    区分离散

//    计算正态分布参数

//    样本均值
    float mean = 0;
    float sum = 0;
    for (auto i : columnData){
        sum += i;
    }
    mean = sum / columnData.size();

//    样本方差
    float stddev = 0;
    float sum_squared = 0;
    for (auto i : columnData){
        sum_squared += pow(i - mean, 2);
    }
    stddev = sqrt(sum_squared / static_cast<float>(columnData.size()));

//    在最小到最大的范围内，等距离取6个点曲线，并添加到lineSeries上面
    auto pdf_normal = [mean, stddev](float x)->float{
        return 1.0 / (stddev * sqrt(2 * 3.1415)) * exp(-1 / (2 * pow(stddev, 2)) * pow(x - mean, 2));
    };
    if (!is_discrete){
        for (int i = 0; i < 9; i ++){
            float x = minValue + i * binWidth;
            float y = pdf_normal(x);
            lineSeries->append(x, y);
        }
    }
    const float max_density = pdf_normal(mean);

//    将分布曲线与坐标轴关联
//    横轴范围：最小到最大值
//    纵轴范围：均值处分布取最大值，最小值为0
    auto axisX_dist = new QValueAxis(this);
    axisX_dist->setTitleText("value");
    axisX_dist->setRange(minValue, maxValue);
    chart->addAxis(axisX_dist, Qt::AlignTop);
    auto axisY_dist = new QValueAxis(this);
    axisY_dist->setTitleText("Density");
    axisY_dist->setRange(0, 1.25 * max_density);
    chart->addAxis(axisY_dist, Qt::AlignRight);
    lineSeries->attachAxis(axisX_dist);
    lineSeries->attachAxis(axisY_dist);

//      勾选设置可见
    check_barchart->setChecked(true);
    check_linechart->setChecked(true);
}

/**
 * @brief 选择显示直方图。
 * 
 * @param state 为Qt::Checked时显示，否则不显示。
 */
void Window_Barchart::on_check_bar(int state){
    if (state == Qt::Checked){
        barSeries->setVisible(true);
    }
    else{
        barSeries->setVisible(false);
    }
}

/**
 * @brief 选择显示折线图。
 * 
 * @param state 为Qt::Checked时显示，否则不显示。
 */
void Window_Barchart::on_check_line(int state){
    if (state == Qt::Checked){
        lineSeries->setVisible(true);
    }
    else{
        lineSeries->setVisible(false);
    }
}
