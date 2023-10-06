#include "window_covariance.h"

#include <include/needed_algo/covariance.hpp>
#include <QBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGradientStops>
#include <QLabel>

/**
 * @brief Construct a new Window_Covariance::Window_Covariance object
 * 
 * @param variants 变量的数据。
 * @param headers 变量的名称。
 * @param parent 
 */
Window_Covariance::Window_Covariance(
    const std::vector<std::vector<float>> &variants,
    const QStringList &headers,
    QWidget *parent):
    QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
//    布局

//    resize(800, 800);
//    setMinimumSize(800, 600);

    auto central = new QWidget(this);
    setCentralWidget(central);

    auto layout_main = new QVBoxLayout(central);
    central->setLayout(layout_main);

    auto group = new QButtonGroup(central);
    auto layout_radios = new QHBoxLayout();
    layout_main->addLayout(layout_radios);

    auto radio_cov = new QRadioButton("Covariance", central);
    layout_radios->addWidget(radio_cov);
    group->addButton(radio_cov);
    auto radio_corr = new QRadioButton("Correlation", central);
    layout_radios->addWidget(radio_corr);
    group->addButton(radio_corr);

    table = new QTableWidget(central);
    layout_main->addWidget(table);
    table->setMinimumSize(800, 600);

    auto colorBar = new ColorBarWidget(central);
    layout_main->addWidget(colorBar);

    const Eigen::MatrixXf eigen_cov = getCovariance(variants);

    matrixSize = eigen_cov.rows();

    for (size_t i = 0; i < matrixSize; ++i) {
        std::vector<float> row;
        for (size_t j = 0; j < matrixSize; ++j) {
            row.push_back(eigen_cov(i, j));
        }
        cov.push_back(row);
    }

    for (size_t i = 0; i < matrixSize; i ++) {
        var.push_back(cov[i][i]);
    }

    const Eigen::MatrixXf eigen_corr = getPearsonCorr(eigen_cov, var);

    for (size_t i = 0; i < matrixSize; ++i) {
        std::vector<float> row;
        for (size_t j = 0; j < matrixSize; ++j) {
            row.push_back(eigen_corr(i, j));
        }
        corr.push_back(row);
    }

    // 设置表格的行数和列数
    table->setRowCount(matrixSize);
    table->setColumnCount(matrixSize);

    table->setHorizontalHeaderLabels(headers);
    table->setVerticalHeaderLabels(headers);

    // 设置单元格的大小，使其成为正方形
    for (size_t i = 0; i < matrixSize; ++i) {
        table->setRowHeight(i, cellSize); // 设置行高
        table->setColumnWidth(i, cellSize); // 设置列宽
    }

    connect(radio_cov, &QRadioButton::toggled,
            this, &Window_Covariance::on_cov_toggled);

    connect(radio_corr, &QRadioButton::toggled,
            this, &Window_Covariance::on_corr_toggled);

    // 遍历协方差矩阵并添加数据到单元格
    for (size_t row = 0; row < matrixSize; ++row) {
        for (size_t col = 0; col < matrixSize; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem("N/A");

            item->setTextAlignment(Qt::AlignCenter);

            double correlation = corr[row][col];
            // 将相关系数映射到0-1的范围
            double normalizedCorrelation = (correlation + 1.0) / 2.0;
            QColor color = mapValueToColor(normalizedCorrelation);
            QBrush brush(color);
            item->setBackground(brush);

            table->setItem(row, col, item);
        }
    }

    radio_corr->toggle();
}

/**
 * @brief 将值映射到颜色。
 * 
 * @param value 需要映射的值。
 * @return QColor 映射后的颜色。
 */
QColor Window_Covariance::mapValueToColor(double value) {
    //    从255 255 255 到 110 12 24
    int red = static_cast<int>(255.0 - value * 145.0); // 从0到255
    int green = static_cast<int>(255.0 - value * 243);
    int blue = static_cast<int>(255.0 - value * 231);

    // 创建并返回对应的颜色
    return QColor(red, green, blue);
}

/**
 * @brief 显示协方差矩阵。
 * 
 * @param checked
 */
void Window_Covariance::on_cov_toggled(bool checked){
    if (checked == true){
        // 遍历协方差矩阵并添加数据到单元格
        for (size_t row = 0; row < matrixSize; ++row) {
            for (size_t col = 0; col < matrixSize; ++col) {
                table->item(row, col)->setText(QString::number(cov[row][col]));
            }
        }
    }
}

/**
 * @brief 显示相关系数矩阵。
 * 
 * @param checked 
 */
void Window_Covariance::on_corr_toggled(bool checked){
    if (checked == true){
        // 遍历协方差矩阵并添加数据到单元格
        for (size_t row = 0; row < matrixSize; ++row) {
            for (size_t col = 0; col < matrixSize; ++col) {
                table->item(row, col)->setText(QString::number(corr[row][col]));
            }
        }
    }
}
