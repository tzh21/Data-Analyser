#include "window_pca.h"
#include "include/needed_algo/pca.hpp"
#include "include/common_utils.h"

#include <QChart>
#include <QChartView>
#include <QScatterSeries>
#include <QValueAxis>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QWindow>
#include <QRandomGenerator>
#include <QSignalMapper>
#include <QLineEdit>

/**
 * @brief Construct a new Window_PCA2D::Window_PCA2D object
 * 
 * @param variants 变量的数据。
 * @param labels 每个样本的组别。
 * @param cnt_groups 组别的总数。
 * @param parent 
 */
Window_PCA2D::Window_PCA2D(
    const std::vector<std::vector<float>> &variants,
    const std::vector<int> &labels, // size: 2
    const size_t cnt_groups, // 2
    QWidget *parent):

    QWidget(parent){

    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(800, 600);

    if (variants.size() <= 0){
        QMessageBox::critical(this, "Error", "No variant selected.");
        return;
    }

    // 布局

    auto layout_main = new QVBoxLayout(this);

    auto layout_cluster = new QHBoxLayout;
    layout_main->addLayout(layout_cluster);

    auto layout_info = new QHBoxLayout;
    layout_main->addLayout(layout_info);

    auto group_idx = new QGroupBox("组别");
    layout_info->addWidget(group_idx);
    group_idx->setLayout(new QVBoxLayout);
    edit_group = new QLineEdit;
    group_idx->layout()->addWidget(edit_group);

    auto group_col = new QGroupBox("列序号");
    layout_info->addWidget(group_col);
    group_col->setLayout(new QVBoxLayout);
    edit_col = new QLineEdit;
    group_col->layout()->addWidget(edit_col);

    auto group_1st = new QGroupBox("第一主成分");
    layout_info->addWidget(group_1st);
    group_1st->setLayout(new QVBoxLayout);
    edit_1st = new QLineEdit;
    group_1st->layout()->addWidget(edit_1st);

    auto group_2nd = new QGroupBox("第二主成分");
    layout_info->addWidget(group_2nd);
    group_2nd->setLayout(new QVBoxLayout);
    edit_2nd = new QLineEdit;
    group_2nd->layout()->addWidget(edit_2nd);

    auto chart = new QChart;
    auto chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    layout_main->addWidget(chartView);

//    获取二维视图
    Eigen::MatrixXf samples_dim2 = pca(variants, 2);
    const size_t cnt_samples = samples_dim2.rows();

//    创建点集
    std::vector<QScatterSeries*> vec_series;
    auto signal_mapper = new QSignalMapper(this);
    for (size_t i = 0; i < cnt_groups; i ++){
        vec_series.push_back(new QScatterSeries);
        beautify_scatter_series(vec_series[i]);
        vec_series[i]->setName("Group " + QString::number(i));

        int r = QRandomGenerator::global()->bounded(20, 241);
        int g = QRandomGenerator::global()->bounded(20, 241);
        int b = QRandomGenerator::global()->bounded(20, 241);
        vec_series[i]->setColor(QColor(r, g, b));
        connect(vec_series[i], &QScatterSeries::hovered,
            signal_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        signal_mapper->setMapping(vec_series[i], i);
        connect(vec_series[i], &QScatterSeries::hovered,
            this, &Window_PCA2D::onPointHovered);
    }
    connect(signal_mapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt),
        this, &Window_PCA2D::on_group_hovered);

//    添加噪音点集（标签值为-1）
    auto series_noise = new QScatterSeries;
    beautify_scatter_series(series_noise);
    series_noise->setName("Noise");
    series_noise->setColor(QColor(0, 0, 0));

//    填充点集
    map_xy_col.clear();
    // map_point_col.clear();
    for (size_t i = 0; i < cnt_samples; i ++){
        int label = labels[i];
        if (label >= 0){
            vec_series[label]->append(samples_dim2(i, 0), samples_dim2(i, 1));
            std::string str = std::to_string(samples_dim2(i, 0)) + "," + std::to_string(samples_dim2(i, 1));
            map_xy_col.insert(std::make_pair(str, i));
        }
        else if (label == -1){
            series_noise->append(samples_dim2(i, 0), samples_dim2(i, 1));
            std::string str = std::to_string(samples_dim2(i, 0)) + "," + std::to_string(samples_dim2(i, 1));
            map_xy_col.insert(std::make_pair(str, -1));
        }
    }

    for (size_t i = 0; i < cnt_groups; i ++){
        chart->addSeries(vec_series[i]);
    }
    if (series_noise->count() != 0){
        chart->addSeries(series_noise);
    }

    // for (int i = 0; i < cnt_groups; i ++){
    //     const auto &series = dynamic_cast<QScatterSeries*>(chart->series()[i]);
    //     for (int j = 0; j < vec_series[i]->count(); j ++){
    //         const QPointF &point = series->at(j);
    //         map_point_col.insert(std::make_pair(&point, i));
    //     }
    // }

    chart->createDefaultAxes();
    auto axes = chart->axes();
    auto axisX = axes[0];
    auto axisY = axes[1];
    axisX->setTitleText("1st Component");
    axisY->setTitleText("2nd Component");
}

/**
 * @brief Construct a new Window_PCA3D::Window_PCA3D object
 * 
 * @param edits 组别、坐标等详细信息的显示框。因为需要在选中3D点时修改。
 * @param variants 变量的数据。
 * @param labels 每个样本的组别。
 * @param cnt_groups 组别的总数。
 */
Window_PCA3D::Window_PCA3D(
    std::vector<QLineEdit*> &edits,
    const std::vector<std::vector<float>> &variants,
    const std::vector<int> &labels,
    const size_t cnt_groups){

    edit_group = edits[0];
    edit_1st = edits[1];
    edit_2nd = edits[2];
    edit_3rd = edits[3];
    edit_col = edits[4];

    setMinimumSize(QSize(600, 600));
    activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
    setHorizontalAspectRatio(1);
    setAspectRatio(1);
    auto camera = scene()->activeCamera();
    camera->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);

    std::vector<QScatterDataProxy*> vec_proxy;
    auto noise_proxy = new QScatterDataProxy;
    for (size_t i = 0; i < cnt_groups; i ++){
        vec_proxy.push_back(new QScatterDataProxy);
    }

    const float item_size = 0.1;
    std::vector<QScatter3DSeries*> vec_series;
    auto signal_mapper = new QSignalMapper(this);
    auto noise_series = new QScatter3DSeries(noise_proxy);
    for (size_t i = 0; i < cnt_groups; i ++){
        vec_series.push_back(new QScatter3DSeries(vec_proxy[i]));
        vec_series[i]->setItemSize(item_size);
        vec_series[i]->setName("Group " + QString::number(i));

        int r = QRandomGenerator::global()->bounded(20, 241);
        int g = QRandomGenerator::global()->bounded(20, 241);
        int b = QRandomGenerator::global()->bounded(20, 241);
        vec_series[i]->setBaseColor(QColor(r, g, b));
        connect(vec_series[i], &QScatter3DSeries::selectedItemChanged,
            signal_mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        signal_mapper->setMapping(vec_series[i], i);
        connect(vec_series[i], &QScatter3DSeries::selectedItemChanged,
            this, &Window_PCA3D::on_item_selected);
    }
    connect(signal_mapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mappedInt),
        this, &Window_PCA3D::on_group_selected);
    noise_series->setItemSize(item_size);
    noise_series->setName("Noise");
    noise_series->setBaseColor(Qt::black);

    Eigen::MatrixXf samples_3d = pca(variants, 3);
    const size_t cnt_samples = samples_3d.rows(); // 5

    std::vector<QScatterDataArray> vec_array;
    QScatterDataArray array_noise;
    for (size_t i = 0; i < cnt_groups; i ++){
        vec_array.push_back(QScatterDataArray());
        vec_array.reserve(cnt_samples);
    }
    array_noise.reserve(cnt_samples);
    
    map_xyz_col.clear();
    for (size_t i = 0; i < cnt_samples; i ++){
        int label = labels[i];
        if (label >= 0) {
            vec_array[label] << QVector3D(samples_3d(i, 0), samples_3d(i, 1), samples_3d(i, 2));
            std::string str = std::to_string(samples_3d(i, 0)) + "," + std::to_string(samples_3d(i, 1)) + "," + std::to_string(samples_3d(i, 2));
            map_xyz_col.insert(std::make_pair(str, i));
        }
        else if (label == -1){
            array_noise << QVector3D(samples_3d(i, 0), samples_3d(i, 1), samples_3d(i, 2));
            std::string str = std::to_string(samples_3d(i, 0)) + "," + std::to_string(samples_3d(i, 1)) + "," + std::to_string(samples_3d(i, 2));
            map_xyz_col.insert(std::make_pair(str, -1));
        }
    }

    for (size_t i = 0; i < cnt_groups; i ++){
        vec_proxy[i]->addItems(vec_array[i]);
    }
    noise_proxy->addItems(array_noise);

    for (auto& i : vec_series){
        addSeries(i);
    }
    addSeries(noise_series);
}

/**
 * @brief Construct a new Window_PCA::Window_PCA object
 * 
 * @param _table_widget 表格的指针。因为需要获取聚类的标签。
 * @param _variants 变量的数据。
 * @param _diagnosis 每个样本的诊断结果。
 * @param parent 
 */
Window_PCA::Window_PCA(
    Widget *_table_widget,
    std::vector<std::vector<float>> &&_variants,
    std::vector<int> &&_diagnosis,
    QWidget *parent):
    QMainWindow(parent),
    table_widget(_table_widget),
    variants(_variants),
    diagnosis(_diagnosis){

    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(200);

    auto central = new QWidget;
    setCentralWidget(central);

    auto layout_main = new QHBoxLayout(central);

    auto box_cluster = new QGroupBox("聚类算法");
    layout_main->addWidget(box_cluster);

    auto box_chart = new QGroupBox("生成图像");
    layout_main->addWidget(box_chart);

    auto layout_cluster = new QVBoxLayout(box_cluster);

    auto group_cluster = new QButtonGroup(box_cluster);

    auto radio_kmeans = new QRadioButton("K-means");
    layout_cluster->addWidget(radio_kmeans);
    group_cluster->addButton(radio_kmeans);
    connect(radio_kmeans, &QRadioButton::toggled, this, &Window_PCA::on_radio_kmeans_toggled);
    radio_kmeans->toggle();

    auto radio_dbscan = new QRadioButton("dbscan");
    layout_cluster->addWidget(radio_dbscan);
    group_cluster->addButton(radio_dbscan);
    connect(radio_dbscan, &QRadioButton::toggled, this, &Window_PCA::on_radio_dbscan_toggled);

    auto layout_chart = new QVBoxLayout(box_chart);

    button_2d = new QPushButton("2D 降维（仅标记BM）");
    layout_chart->addWidget(button_2d);
    auto button_2d_cluster = new QPushButton("2D 降维（包含聚类）");
    layout_chart->addWidget(button_2d_cluster);
    button_3d = new QPushButton("3D 降维（仅标记BM）", central);
    layout_chart->addWidget(button_3d);
    auto button_3d_cluster = new QPushButton("3D 降维（包含聚类）");
    layout_chart->addWidget(button_3d_cluster);

    connect(button_2d, &QPushButton::clicked, this, &Window_PCA::on_button_2d_clicked);
    connect(button_2d_cluster, &QPushButton::clicked, this, &Window_PCA::on_button_2dcluster_clicked);
    connect(button_3d, &QPushButton::clicked, this, &Window_PCA::on_button_3d_clicked);
    connect(button_3d_cluster, &QPushButton::clicked, this, &Window_PCA::on_button_3dcluster_clicked);
}

/**
 * @brief 2D降维按钮的槽函数。仅标记BM。
 * 
 */
void Window_PCA::on_button_2d_clicked(){
    if (variants[0].size() < 2) {
        QMessageBox::critical(this, "Error", "Please select more than 2 columns.");
        return;
    }

    auto window_2d = new QMainWindow(this);
    auto widget_2d = new Window_PCA2D(variants, diagnosis, 2, window_2d);
    window_2d->setCentralWidget(widget_2d);
    window_2d->show();
}

/**
 * @brief 显示K-means聚类算法的分组。
 * 
 */
void Window_PCA::on_radio_kmeans_toggled(bool checked){
    if (checked){
        cluster_method = Cluster_method::kmeans;
    }
}

/**
 * @brief 显示dbscan聚类算法的分组。
 * 
 * @param checked 
 */
void Window_PCA::on_radio_dbscan_toggled(bool checked){
    if (checked){
        cluster_method = Cluster_method::dbscan;
    }
}

/**
 * @brief 3D降维按钮的槽函数。仅标记BM。
 * 
 */
void Window_PCA::on_button_3d_clicked(){
    if (variants[0].size() < 3){
        QMessageBox::critical(this, "Error", "Please select more than 3 columns.");
        return;
    }

    auto window = new QMainWindow;
    window->setMinimumSize(600, 600);
    auto central = new QWidget(window);
    central->setMinimumSize(600, 600);
    auto layout_central = new QVBoxLayout(central);

    auto layout_info = new QHBoxLayout;
    layout_central->addLayout(layout_info);

    auto layout_scatters = new QHBoxLayout;
    layout_central->addLayout(layout_scatters);

    auto layout_bm = new QHBoxLayout;
    layout_info->addLayout(layout_bm);

    auto set_layout = [](QHBoxLayout *layout) -> std::vector<QLineEdit*> {
        auto group_idx = new QGroupBox("组别");
        layout->addWidget(group_idx);
        group_idx->setLayout(new QVBoxLayout);
        auto edit_group3d = new QLineEdit;
        group_idx->layout()->addWidget(edit_group3d);

        auto group_col = new QGroupBox("列序号");
        layout->addWidget(group_col);
        group_col->setLayout(new QVBoxLayout);
        auto edit_col3d = new QLineEdit;
        group_col->layout()->addWidget(edit_col3d);

        auto group_1st = new QGroupBox("第一主成分");
        layout->addWidget(group_1st);
        group_1st->setLayout(new QVBoxLayout);
        auto edit_1st3d = new QLineEdit;
        group_1st->layout()->addWidget(edit_1st3d);

        auto group_2nd = new QGroupBox("第二主成分");
        layout->addWidget(group_2nd);
        group_2nd->setLayout(new QVBoxLayout);
        auto edit_2nd3d = new QLineEdit;
        group_2nd->layout()->addWidget(edit_2nd3d);

        auto group_3rd = new QGroupBox("第三主成分");
        layout->addWidget(group_3rd);
        group_3rd->setLayout(new QVBoxLayout);
        auto edit_3rd3d = new QLineEdit;
        group_3rd->layout()->addWidget(edit_3rd3d);

        return {edit_group3d, edit_1st3d, edit_2nd3d, edit_3rd3d, edit_col3d};
    };

    auto edits_bm = set_layout(layout_bm);

    auto scatter_bm = new Window_PCA3D(edits_bm, variants, diagnosis, 2);

    auto container_bm = QWidget::createWindowContainer(scatter_bm);
    container_bm->setMinimumSize(600, 600);
    layout_scatters->addWidget(container_bm);
    window->show();
}

/**
 * @brief 2D降维按钮的槽函数。包含聚类。
 * 
 */
void Window_PCA::on_button_2dcluster_clicked(){
    if (variants[0].size() < 2) {
        QMessageBox::critical(this, "Error", "Please select more than 2 columns.");
        return;
    }

    auto widget_bm = new Window_PCA2D(variants, diagnosis, 2);

//    获取该聚类的标签
    std::vector<int> labels = table_widget->get_labels_of(cluster_method);
    size_t cnt_groups = table_widget->map_cluster_groups[cluster_method];

//    若在表中找到分组，size为数据规模
//    若在表中没找到分组，size为0，此时报错终止
    if (labels.size() == 0){
        QMessageBox::critical(this, "Error", "No column called K-means_cluster was found.");
        return;
    }

    auto widget_cluster = new Window_PCA2D(variants, labels, cnt_groups);

    auto window = new QMainWindow;
    auto central = new QWidget(window);
    window->setCentralWidget(central);
    auto layout_central = new QHBoxLayout(central);
    layout_central->addWidget(widget_bm);
    layout_central->addWidget(widget_cluster);
    window->show();
//    connect(window, &QObject::destroyed,[&window](){
//        window->deleteLater();
//        window = nullptr;
//    });
}

/**
 * @brief 3D降维按钮的槽函数。包含聚类。
 * 
 */
void Window_PCA::on_button_3dcluster_clicked(){
    if (variants[0].size() < 3) {
        QMessageBox::critical(this, "Error", "Please select more than 3 columns.");
        return;
    }

    //    获取该聚类的标签
    std::vector<int> labels = table_widget->get_labels_of(cluster_method);
    size_t cnt_groups = table_widget->map_cluster_groups[cluster_method];

    auto window = new QMainWindow;
    window->setMinimumSize(1200, 600);
    auto central = new QWidget(window);
    central->setMinimumSize(1200, 600);
    auto layout_central = new QVBoxLayout(central);

    auto layout_info = new QHBoxLayout;
    layout_central->addLayout(layout_info);

    auto layout_scatters = new QHBoxLayout;
    layout_central->addLayout(layout_scatters);

    auto layout_bm = new QHBoxLayout;
    layout_info->addLayout(layout_bm);

    auto layout_cluster = new QHBoxLayout;
    layout_info->addLayout(layout_cluster);

    auto set_layout = [](QHBoxLayout *layout) -> std::vector<QLineEdit*> {
        auto group_idx = new QGroupBox("组别");
        layout->addWidget(group_idx);
        group_idx->setLayout(new QVBoxLayout);
        auto edit_group3d = new QLineEdit;
        group_idx->layout()->addWidget(edit_group3d);

        auto group_col = new QGroupBox("列序号");
        layout->addWidget(group_col);
        group_col->setLayout(new QVBoxLayout);
        auto edit_col3d = new QLineEdit;
        group_col->layout()->addWidget(edit_col3d);

        auto group_1st = new QGroupBox("第一主成分");
        layout->addWidget(group_1st);
        group_1st->setLayout(new QVBoxLayout);
        auto edit_1st3d = new QLineEdit;
        group_1st->layout()->addWidget(edit_1st3d);

        auto group_2nd = new QGroupBox("第二主成分");
        layout->addWidget(group_2nd);
        group_2nd->setLayout(new QVBoxLayout);
        auto edit_2nd3d = new QLineEdit;
        group_2nd->layout()->addWidget(edit_2nd3d);

        auto group_3rd = new QGroupBox("第三主成分");
        layout->addWidget(group_3rd);
        group_3rd->setLayout(new QVBoxLayout);
        auto edit_3rd3d = new QLineEdit;
        group_3rd->layout()->addWidget(edit_3rd3d);

        return {edit_group3d, edit_1st3d, edit_2nd3d, edit_3rd3d, edit_col3d};
    };

    auto edits_bm = set_layout(layout_bm);
    auto edits_cluster = set_layout(layout_cluster);

    auto scatter_bm = new Window_PCA3D(edits_bm, variants, diagnosis, 2);
    auto scatter_cluster = new Window_PCA3D(edits_cluster, variants, labels, cnt_groups);

    auto container_bm = QWidget::createWindowContainer(scatter_bm);
    container_bm->setMinimumSize(600, 600);
    layout_scatters->addWidget(container_bm);
    auto container_cluster = QWidget::createWindowContainer(scatter_cluster);
    container_cluster->setMinimumSize(600, 600);
    layout_scatters->addWidget(container_cluster);
    window->show();
}

/**
 * @brief 2D图中在点上悬停时，显示组别。
 * 
 * @param index 
 */
void Window_PCA2D::on_group_hovered(int index){
    edit_group->setText(QString::number(index));
}

/**
 * @brief 2D图中在点上悬停时，显示列序号和坐标。
 * 
 * @param point 
 * @param state 
 */
void Window_PCA2D::onPointHovered(const QPointF &point, bool state){
    if (state){
        std::string str = std::to_string(point.x()) + "," + std::to_string(point.y());
        edit_col->setText(QString::number(map_xy_col[str]));
        edit_1st->setText(QString::number(point.x()));
        edit_2nd->setText(QString::number(point.y()));
    }
}

/**
 * @brief 3D图中点击点时，显示组别。
 * 
 * @param index 
 */
void Window_PCA3D::on_group_selected(int index){
    if (index < 0){
        return;
    }
    selected_group = index;
    edit_group->setText(QString::number(index));
}

/**
 * @brief 3D图中点击点时，显示列序号和坐标。
 * 
 * @param index 
 */
void Window_PCA3D::on_item_selected(int index){
    if (index < 0){
        return;
    }
    float x = seriesList().at(selected_group)->dataProxy()->array()->at(index).x();
    float y = seriesList().at(selected_group)->dataProxy()->array()->at(index).y();
    float z = seriesList().at(selected_group)->dataProxy()->array()->at(index).z();
    std::string str = std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    edit_col->setText(QString::number(map_xyz_col[str]));
    edit_1st->setText(QString::number(x));
    edit_2nd->setText(QString::number(y));
    edit_3rd->setText(QString::number(z));
}