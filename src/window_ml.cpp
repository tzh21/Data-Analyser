#include "window_ml.h"

#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <xgboost/c_api.h>

// #define safe_xgboost(call) { \
// int err = (call); \
//     if (err != 0) { \
//         fprintf(stderr, "%s:%d: error in %s: %s\\n", __FILE__, __LINE__, #call, XGBGetLastError()); \
//         exit(1); \
// } \
// }QChartView>

/**
 * @brief Construct a new Window_ML::Window_ML object
 * 
 * @param _diagnosis 样本的症状列表。0为良性，1为恶性。用于训练标签。
 * @param _feature_names 样本的特征名称列表。
 * @param _samples 样本的特征值列表。
 * @param parent 
 */
Window_ML::Window_ML(
    std::vector<int> &&_diagnosis,
    std::vector<std::string> &&_feature_names,
    std::vector<std::vector<float>> &&_samples,
    QWidget *parent):

    QMainWindow{parent},
    diagnosis(_diagnosis),
    feature_names(_feature_names),
    samples(_samples)
{
    // 布局

    setWindowTitle("训练与预测");
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(800, 600);

    auto central = new QWidget(this);
    setCentralWidget(central);
    auto layout_central = new QHBoxLayout(central);

    auto layout_ratio_and_table = new QVBoxLayout;
    layout_central->addLayout(layout_ratio_and_table);

    auto layout_analysis = new QVBoxLayout;
    layout_central->addLayout(layout_analysis);

    auto layout_settings = new QHBoxLayout;
    layout_ratio_and_table->addLayout(layout_settings);

    auto layout_table = new QHBoxLayout;
    layout_ratio_and_table->addLayout(layout_table);

    auto group_ratio = new QGroupBox("测试集:训练集");
    layout_settings->addWidget(group_ratio);

    auto group_max_depth = new QGroupBox("最大深度");
    layout_settings->addWidget(group_max_depth);

    auto group_iter = new QGroupBox("迭代次数");
    layout_settings->addWidget(group_iter);

    auto group_f1score = new QGroupBox("F1-score");
    layout_settings->addWidget(group_f1score);

    auto group_auc = new QGroupBox("AUC");
    layout_settings->addWidget(group_auc);

    auto layout_ratio = new QHBoxLayout;
    group_ratio->setLayout(layout_ratio);

    comb_ratio = new QComboBox;
    layout_ratio->addWidget(comb_ratio);
    comb_ratio->addItem("0.1");
    comb_ratio->addItem("0.2");
    comb_ratio->addItem("0.3");
    comb_ratio->addItem("0.5");

    auto button_ratio = new QPushButton("确定");
    layout_ratio->addWidget(button_ratio);
    connect(button_ratio, &QPushButton::clicked, this, &Window_ML::on_button_ratio_clicked);

    group_max_depth->setLayout(new QHBoxLayout);
    edit_max_depth = new QLineEdit;
    group_max_depth->layout()->addWidget(edit_max_depth);
    edit_max_depth->setText("3");

    group_iter->setLayout(new QHBoxLayout);
    edit_iter = new QLineEdit;
    group_iter->layout()->addWidget(edit_iter);
    edit_iter->setText("10");

    edit_f1score = new QLineEdit;
    group_f1score->setLayout(new QHBoxLayout);
    group_f1score->layout()->addWidget(edit_f1score);

    edit_auc = new QLineEdit;
    group_auc->setLayout(new QHBoxLayout);
    group_auc->layout()->addWidget(edit_auc);

    auto layout_table_train = new QVBoxLayout;
    layout_table->addLayout(layout_table_train);

    auto layout_table_test = new QVBoxLayout;
    layout_table->addLayout(layout_table_test);

    auto label_train = new QLabel("训练集");
    layout_table_train->addWidget(label_train);

    auto label_test = new QLabel("测试集");
    layout_table_test->addWidget(label_test);

    table_train = new QTableWidget;
    layout_table_train->addWidget(table_train);
    table_test = new QTableWidget;
    layout_table_test->addWidget(table_test);

    auto button_train = new QPushButton("开始训练");
    layout_analysis->addWidget(button_train);
    connect(button_train, &QPushButton::clicked, this, &Window_ML::on_button_train_clicked);

    auto button_test_result = new QPushButton("测试结果");
    layout_analysis->addWidget(button_test_result);
    connect(button_test_result, &QPushButton::clicked, this, &Window_ML::on_button_test_result_clicked);

    auto button_feature = new QPushButton("特征贡献度");
    layout_analysis->addWidget(button_feature);
    connect(button_feature, &QPushButton::clicked, this, &Window_ML::on_button_feature_clicked);
}

/**
 * @brief 划分训练集和测试集的按钮的槽函数。根据比例划分训练集和测试集，并绘制表格。
 */
void Window_ML::on_button_ratio_clicked()
{
    // 获取数据集和训练集的比例
    float ratio = comb_ratio->currentText().toFloat();
    int cnt_test = static_cast<int>(diagnosis.size() * ratio);
    // 检查测试集数量是否为0
    if (cnt_test <= 0){
        QMessageBox::critical(this, "错误", "测试集数量为0");
        return;
    }

    // 划分训练集和测试集
    std::vector<std::pair<int, int>> col_diagnosis;
    for (size_t i = 0; i < diagnosis.size(); ++i)
    {
        col_diagnosis.push_back(std::make_pair(i, diagnosis[i]));
    }
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::random_shuffle(col_diagnosis.begin(), col_diagnosis.end());
    std::vector<std::pair<int, int>> train;
    std::vector<std::pair<int, int>> test;
    idx_test.clear();
    for (size_t i = 0; i < cnt_test; i ++){
        test.push_back(col_diagnosis[i]);
        idx_test.push_back(col_diagnosis[i].first);
    }
    idx_train.clear();
    for (size_t i = cnt_test; i < col_diagnosis.size(); i ++){
        train.push_back(col_diagnosis[i]);
        idx_train.push_back(col_diagnosis[i].first);
    }

    // 绘制训练集表格
    table_train->clear();
    table_train->setRowCount(train.size());
    table_train->setColumnCount(2);
    table_train->setHorizontalHeaderLabels(QStringList() << "index" << "diagnosis");
    for (size_t row = 0; row < train.size(); ++row)
    {
        table_train->setItem(row, 0, new QTableWidgetItem(QString::number(train[row].first)));
        table_train->setItem(row, 1, new QTableWidgetItem(QString::number(train[row].second)));
    }
    // 绘制测试集表格
    table_test->clear();
    table_test->setRowCount(test.size());
    table_test->setColumnCount(3);
    table_test->setHorizontalHeaderLabels(QStringList() << "index" << "diagnosis" << "predict");
    for (size_t row = 0; row < test.size(); ++row)
    {
        table_test->setItem(row, 0, new QTableWidgetItem(QString::number(test[row].first)));
        table_test->setItem(row, 1, new QTableWidgetItem(QString::number(test[row].second)));
        table_test->setItem(row, 2, new QTableWidgetItem(QString::number(-1)));
    }
}

/**
 * @brief 训练按钮的槽函数。根据训练集进行训练，并预测已划分的数据集。
 */
void Window_ML::on_button_train_clicked(){
    if (samples.empty() || samples[0].empty()){
        QMessageBox::critical(this, "错误", "样本为空");
        return;
    }
    const int size_features = samples[0].size();
    const int size_samples = samples.size();
    const int size_train_samples = idx_train.size();
    const int size_test_samples = idx_test.size();
    const int size_diagnosis = diagnosis.size();
    if (size_diagnosis != size_samples || idx_test.size() == 0 || idx_train.size() == 0){
        QMessageBox::critical(this, "错误", "数据集和标签数量不一致");
        return;
    }

    // 将数据集和标签转换为xgboost的数据结构
    unsigned int label_train[size_train_samples];
    for (int i = 0; i < size_train_samples; i ++){
        label_train[i] = diagnosis[idx_train[i]];
    }
    float train[size_train_samples][size_features];
    for (int i = 0; i < size_train_samples; i ++){
        for (int j = 0; j < size_features; j ++){
            train[i][j] = samples[idx_train[i]][j];
        }
    }

    DMatrixHandle dtrain;

    XGDMatrixCreateFromMat((float *) train, size_train_samples, size_features, -1, &dtrain);

    XGDMatrixSetUIntInfo(dtrain, "label", label_train, size_train_samples); // set label parameter

    // Create booster
    BoosterHandle booster;
    XGBoosterCreate(&dtrain, 1, &booster);

    // 设置booster特征名称
    const char *fnames[size_features];
    for (int i = 0; i < size_features; i ++){
        fnames[i] = feature_names[i].c_str();
    }
    XGBoosterSetStrFeatureInfo(booster, "feature_name", fnames, size_features);

    // 检查参数
    bool is_valid = true;
    int max_depth = edit_max_depth->text().toInt(&is_valid);
    if (!is_valid){
        QMessageBox::critical(this, "错误", "最大深度不是整数");
        return;
    }
    std::string str_max_depth = std::to_string(max_depth);
    const char *c_str_max_depth = str_max_depth.c_str();
    int iter = edit_iter->text().toInt(&is_valid);
    if (!is_valid){
        QMessageBox::critical(this, "错误", "迭代次数不是整数");
        return;
    }

    // Set parameters
    XGBoosterSetParam(booster, "objective", "reg:squarederror");
    XGBoosterSetParam(booster, "max_depth", c_str_max_depth);
    XGBoosterSetParam(booster, "eta", "0.1");

    // Train for 10 iterations
    for (int round = 0; round < iter; round++) {
        XGBoosterUpdateOneIter(booster, round, dtrain);
    }

    // 获取特征贡献度
    char const config[] = 
        "{\"importance_type\": \"weight\"}";
    bst_ulong out_n_features;
    const char** out_features = new const char*[size_features];
    bst_ulong out_dim;
    const int max_n_features = 100;
    bst_ulong const* out_shape = new bst_ulong[max_n_features];
    float const* out_scores = new float[size_features];
    XGBoosterFeatureScore(
        booster, config,
        &out_n_features, &out_features,
        &out_dim, &out_shape,
        &out_scores);

    // 排序后存储特征贡献度
    feature_importance.clear();
    qDebug() << size_features;
    for (int i = 0; i < out_n_features; i ++){
        feature_importance.push_back(std::make_pair(std::string(out_features[i]), out_scores[i]));
    }
    std::sort(feature_importance.begin(), feature_importance.end(), [](const std::pair<std::string, float> &a, const std::pair<std::string, float> &b){
        return a.second < b.second;
    });

    // 预测测试集

    // 转换C数据结构
    unsigned int label_test[size_test_samples];
    for (int i = 0; i < size_test_samples; i ++){
        label_test[i] = diagnosis[idx_test[i]];
    }
    float test[size_test_samples][size_features];
    for (int i = 0; i < size_test_samples; i ++){
        for (int j = 0; j < size_features; j ++){
            test[i][j] = samples[idx_test[i]][j];
        }
    }
    DMatrixHandle dtest;
    XGDMatrixCreateFromMat((float *) test, size_test_samples, size_features, -1, &dtest);
    XGDMatrixSetUIntInfo(dtest, "label", label_test, size_test_samples);

    bst_ulong out_len;
    const float* out_result;
    XGBoosterPredict(booster, dtest, 0, 2, 0, &out_len, &out_result);

    // 将预测结果写入表格
    for (size_t row = 0; row < out_len; ++row)
    {
        if (!table_test->item(row, 2)){
            QMessageBox::critical(this, "错误", "测试集表格中有空值");
            return;
        }
        if (out_result[row] < 0.5){
            table_test->item(row, 2)->setText("0");
        }
        else {
            table_test->item(row, 2)->setText("1");
        }
    }

    // 统计混淆矩阵
    for (int row = 0; row < table_test->rowCount(); row ++){
        if (!table_test->item(row, 1) || !table_test->item(row, 2)){
            QMessageBox::critical(this, "错误", "测试集表格中有空值");
            return;
        }
        if (table_test->item(row, 1)->text() == "0"){
            if (table_test->item(row, 2)->text() == "0"){
                cnt_true_negative ++;
            }
            else {
                cnt_false_positive ++;
            }
        }
        else {
            if (table_test->item(row, 2)->text() == "0"){
                cnt_false_negative ++;
            }
            else {
                cnt_true_positive ++;
            }
        }
    }

    // 计算F1-score
    float precision = cnt_true_positive / (cnt_true_positive + cnt_false_positive);
    float recall = cnt_true_positive / (cnt_true_positive + cnt_false_negative);
    float f1_score = 2 * (precision * recall) / (precision + recall);
    edit_f1score->setText(QString::number(f1_score));

    // 计算AUC
    // 计算ROC曲线的数据点
    std::vector<float> true_positive_rates;
    std::vector<float> false_positive_rates;

    for (float threshold = 0.0; threshold <= 1.0; threshold += 0.01) {
        float true_positives = 0;
        float false_positives = 0;
        float false_negatives = 0;
        float true_negatives = 0;

        for (bst_ulong i = 0; i < out_len; i++) {
            float predicted_value = out_result[i];
            int true_label = label_test[i];

            if (predicted_value >= threshold && true_label == 1) {
                true_positives++;
            } else if (predicted_value >= threshold && true_label == 0) {
                false_positives++;
            } else if (predicted_value < threshold && true_label == 1) {
                false_negatives++;
            } else if (predicted_value < threshold && true_label == 0) {
                true_negatives++;
            }
        }

        // 预测阳性在所有阳性中所占的比例
        float true_positive_rate = true_positives / (true_positives + false_negatives);
        // 预测阳性在所有阴性中所占的比例
        float false_positive_rate = false_positives / (false_positives + true_negatives);

        true_positive_rates.push_back(true_positive_rate);
        false_positive_rates.push_back(false_positive_rate);
    }

    // 计算AUC，即ROC曲线下的面积
    float auc = 0.0;
    for (size_t i = 1; i < true_positive_rates.size(); i++) {
        float delta_fpr = false_positive_rates[i - 1] - false_positive_rates[i];
        auc += true_positive_rates[i - 1] * delta_fpr;
    }

    edit_auc->setText(QString::number(auc));

    // 释放内存
    XGBoosterFree(booster);
    XGDMatrixFree(dtrain);
    XGDMatrixFree(dtest);
    // delete[] out_features;
    // delete[] out_shape;
    // delete[] out_scores;
}

/**
 * @brief 测试结果按钮的槽函数。绘制测试结果的混淆矩阵。
 */
void Window_ML::on_button_test_result_clicked(){
    // 绘制混淆矩阵
    auto window_confusion = new QMainWindow;
    window_confusion->setAttribute(Qt::WA_DeleteOnClose);
    window_confusion->setWindowTitle("混淆矩阵");
    window_confusion->setMinimumSize(400, 400);

    auto table_confusion = new QTableWidget(window_confusion);
    window_confusion->setCentralWidget(table_confusion);

    table_confusion->setRowCount(2);
    table_confusion->setColumnCount(2);

    table_confusion->setHorizontalHeaderLabels(QStringList() << "实际良性" << "实际恶性");
    table_confusion->setVerticalHeaderLabels(QStringList() << "预测良性" << "预测恶性");

    const int item_size = 100;    
    table_confusion->setRowHeight(0, item_size);
    table_confusion->setRowHeight(1, item_size);
    table_confusion->setColumnWidth(0, item_size);
    table_confusion->setColumnWidth(1, item_size);
    table_confusion->setItem(0, 0, new QTableWidgetItem(QString::number(cnt_true_negative)));
    table_confusion->setItem(0, 1, new QTableWidgetItem(QString::number(cnt_false_positive)));
    table_confusion->setItem(1, 0, new QTableWidgetItem(QString::number(cnt_false_negative)));
    table_confusion->setItem(1, 1, new QTableWidgetItem(QString::number(cnt_true_positive)));

    window_confusion->show();
}

/**
 * @brief 特征贡献度按钮的槽函数。绘制特征贡献度的条形图。
 */
void Window_ML::on_button_feature_clicked(){       
    auto window = new QMainWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowTitle("特征贡献度");

    if (feature_importance.empty()){
        QMessageBox::critical(this, "错误", "无特征贡献度");
        return;
    }

    auto chart = new QChart;
    auto chartView = new QChartView;
    chartView->setChart(chart);
    window->setCentralWidget(chartView);

    auto series = new QHorizontalBarSeries;
    chart->addSeries(series);

    auto barSet = new QBarSet("贡献度", this);
    for (auto &feature : feature_importance){
        *barSet << feature.second;
    }
    series->append(barSet);

    auto axis_feature = new QBarCategoryAxis(this);
    axis_feature->setTitleText("特征");
    for (auto &feature : feature_importance){
        axis_feature->append(QString::fromStdString(feature.first));
    }
    chart->addAxis(axis_feature, Qt::AlignLeft);
    series->attachAxis(axis_feature);

    auto axis_value = new QValueAxis(this);
    axis_value->setTitleText("贡献度");
    // 设置范围
    float max_value = feature_importance.back().second;
    axis_value->setRange(0, max_value * 1.25);
    chart->addAxis(axis_value, Qt::AlignBottom);
    series->attachAxis(axis_value);

    window->show();
}