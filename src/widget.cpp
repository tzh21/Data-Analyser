#include "widget.h"
#include "ui_widget.h"
#include "window_barchart.h"
#include "window_scatter.h"
#include "window_covariance.h"
#include "window_pca.h"
#include "window_cluster.h"
#include "window_ml.h"
#include "include/needed_algo/kmeans.hpp"
#include "include/needed_algo/dbscan.hpp"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <Eigen/Dense>
#include <QDialog>

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidget>
#include <QTableView>
#include <QMessageBox>
#include <QRandomGenerator>

std::vector<QColor> colors_set{
   QColor(255, 0, 0),     // 红色
   QColor(0, 255, 0),     // 绿色
   QColor(0, 0, 255),     // 蓝色
   QColor(255, 255, 0),   // 黄色
   QColor(255, 0, 255),   // 品红色
   QColor(0, 255, 255),   // 青色
   QColor(128, 0, 0),     // 深红色
   QColor(0, 128, 0),     // 深绿色
   QColor(0, 0, 128),     // 深蓝色
   QColor(128, 128, 0)
};

std::map<Cluster_method, QString> map_method_string{
    {Cluster_method::kmeans, "K-means_cluster"},
    {Cluster_method::dbscan, "dbscan_cluster"}
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
//    resize(Utils::window_size);
    resize(800, 600);

//    导入数据，显示表格
    ui->tableView->setModel(model);

//    打开文件
    open_table();
}

Widget::~Widget()
{
    delete ui;
}

/**
 * @brief 根据path_table打开csv文件，将数据显示在表格中。
 * 
 */
void Widget::open_table(){
    QFile file(path_table);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        model->clear();

        QTextStream in(&file);

        QString headerLine = in.readLine();
        QStringList headerFields = headerLine.split(",");
        model->setHorizontalHeaderLabels(headerFields);
        for (int64_t i = 0; i < headerFields.size(); i ++) {
            if (headerFields[i] == "diagnosis") {
                col_diagnosis_at = i;
            }
        }

        int row = 0;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(",");

            for (int col = 0; col < fields.size(); col++){
                QString& val = fields[col];
//                benign 良性 0
//                malignant 恶性 1
                if (val == "B"){
                    val = "0";
                }
                else if (val == "M"){
                    val = "1";
                }
                QStandardItem *item = new QStandardItem(fields[col]);
                model->setItem(row, col, item);
            }

            row++;
        }

        file.close();
    }
    else{
        qDebug() << "fail to open";
    }
}

/**
 * @brief 删除obj对象，并将指针置为nullptr。
 * 
 * @param obj 
 */
void Widget::delete_obj(QObject *obj){
    obj->deleteLater();
    obj = nullptr;
}

/**
 * @brief 获取选中的列的数据。不允许选中id列，否则会弹出错误提示框。
 * 
 * @param least_cols 最少需要选中的列数。
 * @return std::vector<std::vector<float>> 
 */
std::vector<std::vector<float>> Widget::samples_selected(size_t least_cols){
    // 获取选中的列
    QModelIndexList selectedColumns = ui->tableView->selectionModel()->selectedColumns();

    const size_t cnt_cols = selectedColumns.size();
    const size_t cnt_rows = model->rowCount();

    if (cnt_cols < least_cols){
        QMessageBox::critical(this, "Error", "Please select at least" + QString::number(least_cols) + "column.");
        return {};
    }

    std::vector<std::vector<float>> samples;

    for (size_t row = 0; row < cnt_rows; row ++){
        std::vector<float> variants;
        for (size_t j = 0; j < cnt_cols; j ++){
            if (model->horizontalHeaderItem(selectedColumns[j].column())->text() == "id"){
                QMessageBox::critical(this, "Error", "Please do not select id column.");
                return {};
            }
            QModelIndex index = model->index(row, selectedColumns[j].column());
            variants.push_back(model->data(index).toFloat());
        }
        samples.push_back(std::move(variants));
    }

    return samples;
}

/**
 * @brief 在表格的method对应列中添加labels作为聚类分组。
 * 
 * @param method 聚类方法
 * @param labels 该聚类方法的分组
 */
void Widget::add_cluster(Cluster_method method, std::vector<int> labels){
    const size_t cnt_total_cols = model->columnCount();
    size_t col_cluster = cnt_total_cols;
    const size_t cnt_total_rows = model->rowCount();
    //    寻找
    bool exist_cluster = false;
    for (int col = cnt_total_cols - 1; col >= 0; col --){
        if (model->horizontalHeaderItem(col)->text() == map_method_string[method]){
            exist_cluster = true;
            col_cluster = col;
            break;
        }
    }
    //    找不到则创建
    if (!exist_cluster){
        col_cluster = cnt_total_cols;
        auto newHeaderItem = new QStandardItem(map_method_string[method]);
        model->setHorizontalHeaderItem(col_cluster, newHeaderItem);
        for (size_t row = 0; row < cnt_total_rows; row ++){
            auto item = new QStandardItem("");
            model->setItem(row, col_cluster, item);
        }
    }

    for (size_t row = 0; row < cnt_total_rows; row ++){
        model->item(row, col_cluster)->setText(QString::number(labels[row]));
    }
}

/**
 * @brief 根据聚类方法对表格进行着色。
 * 
 * @param method 聚类方法
 */
void Widget::coloring_method(Cluster_method method){
    auto label = get_labels_of(method);
    int cnt_groups = map_cluster_groups[method];
    if (cnt_groups == 0){
        QMessageBox::critical(this, "Error", "Cluster data not found.");
        return;
    }

    const size_t cnt_total_rows = model->rowCount();
    const size_t cnt_total_cols = model->columnCount();
    const int col_cluster = map_cluster_col[method];

    //    取颜色
    std::vector<QColor> colors;
    for (int i = 0; i < cnt_groups; i ++){
        int r = QRandomGenerator::global()->bounded(240);
        int g = QRandomGenerator::global()->bounded(240);
        int b = QRandomGenerator::global()->bounded(240);
        colors.push_back(QColor(r, g, b));
    }

    //    着色
    for (size_t row = 0; row < cnt_total_rows; row ++){
        auto index = model->index(row, col_cluster);
        int label = model->data(index).toInt();
        for (size_t col = 0; col < cnt_total_cols; col ++){
            if (label == -1){
                model->item(row, col)->setBackground(QBrush(Qt::white));
            }
            else {
                model->item(row, col)->setBackground(QBrush(colors[label]));
            }
        }
    }
}

/**
 * @brief 获取某个聚类方法的分组，并且统计该方法的分组总数。如果没有找到聚类方法的分组，返回空vector，将分组所在的列设置为-1，将分组总数设置为0，
 * 
 * @param method 
 * @return std::vector<int> 
 */
std::vector<int> Widget::get_labels_of(Cluster_method method){
    const size_t cnt_total_rows = model->rowCount();
    size_t cnt_total_cols = model->columnCount();
    size_t col_cluster = cnt_total_cols;
    bool exist_cluster = false;
    for (int col = cnt_total_cols - 1; col >= 0; col --){
        if (model->horizontalHeaderItem(col)->text() == map_method_string[method]){
            col_cluster = col;
            exist_cluster = true;
            break;
        }
    }
    if (!exist_cluster){
        map_cluster_col[method] = -1;
        map_cluster_groups[method] = 0;
        return {};
    }
    map_cluster_col[method] = col_cluster;

//    获取每个样本的分组
//    记录组的总数
    int max_label = -1;
    std::vector<int> labels;
    for (size_t row = 0; row < cnt_total_rows; row ++){
        auto index = model->index(row, col_cluster);
        labels.push_back(model->data(index).toInt());
        if (labels.back() > max_label){
            max_label = labels.back();
        }
    }
    map_cluster_groups[method] = max_label + 1;
//    if (map_cluster_groups[method] == 0){
//        QMessageBox::critical(this, "Error", "没有找到该聚类分组数据。");
//    }

    return labels;
}

/**
 * @brief 方差和均值按钮的槽函数。
 * 
 */
void Widget::on_button_variance_clicked()
{
    int selectedColumnCount = ui->tableView
                                  ->selectionModel()
                                  ->selectedColumns().count();

    if (selectedColumnCount != 1) {
        QMessageBox::critical(this, "Error", "Please select only 1 column.");
        return;
    }

    int selectedColumn = ui->tableView->currentIndex().column();

    if (model->horizontalHeaderItem(selectedColumn)->text() == "id"){
        QMessageBox::critical(this, "Error", "Please do not select id column.");
        return;
    }

    if (selectedColumn >= 0){
        Eigen::VectorXd columnData(model->rowCount());

        for (int row = 0; row < model->rowCount(); row++){
            QModelIndex index = model->index(row, selectedColumn);
            QVariant data = model->data(index);
            columnData[row] = data.toDouble();
        }

        double mean = columnData.mean();
        double variance = (columnData.array() - mean).array().square().mean();

        QDialog window(this);
//        window.setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout layout(&window);
        window.setLayout(&layout);
        QLabel label_mean("均值", &window);
        layout.addWidget(&label_mean);
        QLineEdit edit_mean(QString::number(mean), &window);
        layout.addWidget(&edit_mean);
        QLabel label_variance("方差", &window);
        layout.addWidget(&label_variance);
        QLineEdit edit_variance(QString::number(variance), &window);
        layout.addWidget(&edit_variance);
        window.exec();
    }
}

/**
 * @brief 直方图和正态分布密度曲线按钮的槽函数。
 * 
 */
void Widget::on_button_barchart_clicked()
{
//    检查选中列数

    auto selectedColumns = ui->tableView->selectionModel()->selectedColumns();

    if (selectedColumns.size() != 1) {
        QMessageBox::critical(this, "Error", "Please select only 1 column.");
        return;
    }

    if (model->horizontalHeaderItem(selectedColumns[0].column())->text() == "id"){
        QMessageBox::critical(this, "Error", "Please do not select id column.");
        return;
    }

    const size_t cnt_total_rows = model->rowCount();
    const size_t col_selected = selectedColumns[0].column();
    QList<float> columnData;
    bool is_discrete = false;
    if (model->horizontalHeaderItem(col_selected)->text() == "diagnosis"){
        is_discrete = true;
    }
    for (size_t row = 0; row < cnt_total_rows; row ++){
        auto index = model->index(row, col_selected);
        columnData.append(model->data(index).toFloat());
    }

    //    打开新窗口
    auto window_bar = new Window_Barchart(is_discrete, columnData, this);
    window_bar->show();
}

/**
 * @brief 散点图按钮的槽函数。
 * 
 */
void Widget::on_button_scatter_clicked()
{
    // 获取选中的单元格模型
    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();

    // 获取选中的列
    QModelIndexList selectedColumns = selectionModel->selectedColumns();

    // 检查是否选中了两列
    if (selectedColumns.count() != 2) {
        QMessageBox::critical(this, "Error", "Please select only 2 columns.");
        return;
    }

    // 创建两个vector<float>来存储选中列的数据
    std::vector<float> dataX;
    std::vector<float> dataY;

    const int colX = selectedColumns[0].column();
    const int colY = selectedColumns[1].column();

    if (colX == colY) {
        QMessageBox::critical(this, "Error", "Please select 2 different columns.");
        return;
    }

    if (model->horizontalHeaderItem(colX)->text() == "id" ||
        model->horizontalHeaderItem(colY)->text() == "id") {
        QMessageBox::critical(this, "Error", "Please do not select id column.");
        return;
    }

    if (model->horizontalHeaderItem(colX)->text() == "diagnosis" ||
        model->horizontalHeaderItem(colY)->text() == "diagnosis") {
        QMessageBox::critical(this, "Error", "Please do not select diagnosis column.");
        return;
    }

    const auto header = ui->tableView->horizontalHeader();
    const QString headerX = header->model()->headerData(colX, Qt::Horizontal).toString();
    const QString headerY = header->model()->headerData(colY, Qt::Horizontal).toString();

    auto get_data = [&](const int col){
        for (int row = 0; row < model->rowCount(); row++){
            QModelIndex index = model->index(row, col);
            QVariant data = model->data(index);

            bool conversionOk;
            float value = data.toFloat(&conversionOk);
            if (conversionOk) {
                if (index.column() == colX) {
                    dataX.push_back(value);
                } else if (index.column() == colY) {
                    dataY.push_back(value);
                }
            }
        }
    };

    get_data(colX);
    get_data(colY);

//    打开新窗口
    auto window_scatter = new Window_Scatter(dataX, dataY, headerX, headerY, this);
    window_scatter->show();
}

/**
 * @brief 打开文件按钮的槽函数。
 * 
 */
void Widget::on_button_open_clicked()
{
    QString new_path = QFileDialog::getOpenFileName(this, "Open CSV File", "", "CSV Files (*.csv)");
//    qDebug() << new_path;

    if (!new_path.isEmpty() && new_path.endsWith(".csv", Qt::CaseInsensitive)) {
        path_table = new_path;
        open_table();
    }
    else{
        QMessageBox::critical(this, "Error", "Fail to open CSV file.");
        return;
    }
}

/**
 * @brief 协方差和相关系数矩阵按钮的槽函数。
 * 
 */
void Widget::on_button_covariance_clicked()
{
    // 获取选中的单元格模型
    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();

    // 获取选中的列
    QModelIndexList selectedColumns = selectionModel->selectedColumns();

    if (selectedColumns.count() < 2) {
        QMessageBox::critical(this, "Error", "Please select at least 2 columns.");
        return;
    }

    const size_t cnt_col = selectedColumns.count();
    const size_t cnt_row = model->rowCount();

    QStringList headers_selected;
    // 获取选中列的表头项并添加到header_selected
    for (QModelIndex index : selectedColumns) {
        int col = index.column();
        if (model->horizontalHeaderItem(col)->text() == "id"){
            QMessageBox::critical(this, "Error", "Please do not select id column.");
            return;
        }
        auto headerItem = model->horizontalHeaderItem(col);
        if (headerItem) {
            headers_selected.append(headerItem->text());
        }
    }

    std::vector<std::vector<float>> cells(cnt_col, std::vector<float>(cnt_row));
    for (size_t i = 0; i < cnt_col; i ++){
        for (size_t row = 0; row < cnt_row; row ++){
            size_t col = selectedColumns[i].column();
            QModelIndex index = model->index(row, col);
            if (i < cells.size() && row < cells[0].size()) {
                cells[i][row] = model->data(index).toFloat();
            }
            else{
                QMessageBox::critical(this, "Error", "index out of range");
            }
        }
    }

    auto window_covar = new Window_Covariance(cells, headers_selected, this);
    window_covar->show();
//    connect(window_covar, &QObject::destroyed, this, &Widget::delete_obj);
}

/**
 * @brief PCA按钮的槽函数。
 * 
 */
void Widget::on_button_pca_clicked()
{
    // 获取选中的单元格模型
    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();

    // 获取选中的列
    QModelIndexList selectedColumns = selectionModel->selectedColumns();

    if (selectedColumns.count() < 2) {
        QMessageBox::critical(this, "Error", "Please select at least 2 columns.");
        return;
    }

    const size_t cnt_col = selectedColumns.count();
    const size_t cnt_row = model->rowCount();

    std::vector<std::vector<float>> samples;
    for (size_t row = 0; row < cnt_row; row ++){
        std::vector<float> variants;
        for (size_t i = 0; i < cnt_col; i ++){
            size_t col = selectedColumns[i].column();
            if (model->horizontalHeaderItem(col)->text() == "id"){
                QMessageBox::critical(this, "Error", "Please do not select id column.");
                return;
            }
            QModelIndex index = model->index(row, col);
//            cells[row][i] = model->data(index).toFloat();
            variants.push_back(model->data(index).toFloat());
        }
        samples.push_back(std::move(variants));
    }

//    BM信息
    std::vector<int> diagnosis;
    for (size_t row = 0; row < cnt_row; row ++) {
        QModelIndex index = model->index(row, col_diagnosis_at);
        diagnosis.push_back(model->data(index).toInt());
    }

    auto widget_pca = new Window_PCA(this, std::move(samples), std::move(diagnosis), this);
    widget_pca->show();
//    connect(widget_pca, &QObject::destroyed, this, &Widget::delete_obj);
}

/**
 * @brief 聚类按钮的槽函数。
 * 
 */
void Widget::on_button_cluster_clicked()
{
    auto window_choice = new Window_Cluster(this, this);
    window_choice->show();
}

/**
 * @brief K-means聚类按钮的槽函数。
 * 
 */
void Widget::on_cluster_kmeans_clicked(){
    auto samples = samples_selected(1);
    if (samples.empty()){
        return;
    }

    Eigen::MatrixXf centers;
    std::vector<int> labels;
    std::tie(centers, labels) = clusterKMeans(samples, map_cluster_groups[Cluster_method::kmeans], kmeans_maxiter);

    add_cluster(Cluster_method::kmeans, labels);
}

/**
 * @brief 着色按钮的槽函数。
 * 
 */
void Widget::on_button_coloring_clicked()
{
    auto window_coloring = new Window_Coloring(this);
    window_coloring->show();
//    connect(window_coloring, &QObject::destroyed, this, &Widget::delete_obj);
}

/**
 * @brief 根据kmeans分组结果进行着色。
 * 
 */
void Widget::on_coloring_kmeans_clicked(){
    coloring_method(Cluster_method::kmeans);
}

/**
 * @brief 取消表格着色。
 * 
 */
void Widget::on_decoloring_clicked(){
    const size_t cnt_total_rows = model->rowCount();
    const size_t cnt_total_cols = model->columnCount();
    for (size_t row = 0; row < cnt_total_rows; row ++){
        for (size_t col = 0; col < cnt_total_cols; col ++){
            model->item(row, col)->setBackground(QBrush(Qt::white));
        }
    }
}

/**
 * @brief dbscan聚类按钮的槽函数。
 * 
 */
void Widget::on_cluster_dbscan_clicked(){
    auto samples = samples_selected(1);

    qDebug() << dbscan_epsilon << dbscan_minPts;
    auto labels = dbscan(samples, dbscan_epsilon, dbscan_minPts);

    add_cluster(Cluster_method::dbscan, labels);
}

/**
 * @brief 根据dbscan分组结果进行着色。
 * 
 */
void Widget::on_coloring_dbscan_clicked(){
    coloring_method(Cluster_method::dbscan);
}

/**
 * @brief 机器学习按钮的槽函数。
 * 
 */
void Widget::on_button_ml_clicked()
{
    // 获取症状数据
    std::vector<int> diagnosis;
    size_t col_diagnosis = 0;
    for (size_t col = 0; col < model->columnCount(); col ++){
        if (model->horizontalHeaderItem(col)->text() == "diagnosis"){
            col_diagnosis = col;
            break;
        }
    }
    for (size_t row = 0; row < model->rowCount(); row ++){
        auto index = model->index(row, col_diagnosis);
        diagnosis.push_back(model->data(index).toInt());
    }

    // 获取特征数据
    std::vector<std::vector<float>> samples;
    for (size_t row = 0; row < model->rowCount(); row ++){
        std::vector<float> variants;
        for (size_t col = 0; col < model->columnCount(); col ++){
            if (col == col_diagnosis){
                continue;
            }
            if (model->horizontalHeaderItem(col)->text() == "id"){
                continue;
            }
            auto index = model->index(row, col);
            variants.push_back(model->data(index).toFloat());
        }
        samples.push_back(std::move(variants));
    }

    // 获取特征名称
    std::vector<std::string> feature_names;
    // std::vector<QString> feature_names;
    for (size_t col = 0; col < model->columnCount(); col ++){
        if (model->horizontalHeaderItem(col)->text() == "id"){
            continue;
        }
        if (model->horizontalHeaderItem(col)->text() == "diagnosis"){
            continue;
        }
        feature_names.push_back(model->horizontalHeaderItem(col)->text().toStdString());
    }

    auto window_ml = new Window_ML(std::move(diagnosis), std::move(feature_names), std::move(samples));
    window_ml->show();
}

