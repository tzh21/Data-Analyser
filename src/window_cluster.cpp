#include "window_cluster.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QValidator>

/**
 * @brief Construct a new Window_Cluster::Window_Cluster object
 * 
 * @param _table_widget 表格窗口指针。因为聚类窗口需要用到表中数据。
 * @param parent 
 */
Window_Cluster::Window_Cluster(Widget *_table_widget, QWidget *parent):
    QMainWindow{parent}, table_widget(_table_widget)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(200);

    auto central = new QWidget(this);
    setCentralWidget(central);
    auto layout_main = new QVBoxLayout(central);

    auto box_kmeans = new QGroupBox("K-means method");
    layout_main->addWidget(box_kmeans);

    auto layout_kmeans_button = new QVBoxLayout;
    box_kmeans->setLayout(layout_kmeans_button);

    auto layout_kmeans = new QFormLayout;
    layout_kmeans_button->addLayout(layout_kmeans);

    auto label_kmeans_k = new QLabel("K");
    auto edit_kmeans_k = new QLineEdit;
    layout_kmeans->addRow(label_kmeans_k, edit_kmeans_k);
    auto valid_k = new QIntValidator(1, INT_MAX, this);
    edit_kmeans_k->setValidator(valid_k);
    edit_kmeans_k->setText("4");
    connect(edit_kmeans_k, &QLineEdit::textEdited, this, &Window_Cluster::on_kmeans_k_edited);

    auto label_kmeans_iter = new QLabel("Iter");
    auto edit_kmeans_iter = new QLineEdit;
    layout_kmeans->addRow(label_kmeans_iter, edit_kmeans_iter);
    auto valid_iter = new QIntValidator(1, INT_MAX, this);
    edit_kmeans_iter->setValidator(valid_iter);
    edit_kmeans_iter->setText("100");
    connect(edit_kmeans_iter, &QLineEdit::textEdited, this, &Window_Cluster::on_kmeans_iter_edited);

    auto button_kmeans = new QPushButton("start K-means");
    layout_kmeans_button->addWidget(button_kmeans);

    auto box_dbscan = new QGroupBox("dbscan method");
    layout_main->addWidget(box_dbscan);

    auto layout_dbscan_main = new QVBoxLayout;
    box_dbscan->setLayout(layout_dbscan_main);

    auto layout_dbscan = new QFormLayout;
    layout_dbscan_main->addLayout(layout_dbscan);

    auto label_dbscan_epsilon = new QLabel("Epsilon");
    auto edit_dbscan_epsilon = new QLineEdit;
    layout_dbscan->addRow(label_dbscan_epsilon, edit_dbscan_epsilon);
    edit_dbscan_epsilon->setText("20");
    auto valid_epsilon = new QDoubleValidator(0.0, 1e6, 3, this);
    edit_dbscan_epsilon->setValidator(valid_epsilon);
    connect(edit_dbscan_epsilon, &QLineEdit::textEdited, this, &Window_Cluster::on_epsilon_edited);

    auto label_dbscan_minpts = new QLabel("Min Points");
    auto edit_dbscan_minpts = new QLineEdit;
    layout_dbscan->addRow(label_dbscan_minpts, edit_dbscan_minpts);
    edit_dbscan_minpts->setText("3");
    auto valid_minpts = new QIntValidator(1, INT_MAX, this);
    edit_dbscan_minpts->setValidator(valid_minpts);
    connect(edit_dbscan_minpts, &QLineEdit::textEdited, this, &Window_Cluster::on_minPts_edited);

    auto button_dbscan = new QPushButton("start dbscan");
    layout_dbscan_main->addWidget(button_dbscan);

    connect(button_kmeans, &QPushButton::clicked, table_widget, &Widget::on_cluster_kmeans_clicked);
    connect(button_dbscan, &QPushButton::clicked, table_widget, &Widget::on_cluster_dbscan_clicked);
}

/**
 * @brief Construct a new Window_Coloring::Window_Coloring object
 * 
 * @param table_widget 表格窗口指针。因为着色窗口需要用到表中数据。
 * @param parent 
 */
Window_Coloring::Window_Coloring(Widget *table_widget, QWidget *parent):
    QMainWindow(parent){

    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumWidth(200);

    auto central = new QWidget(this);
    setCentralWidget(central);

    auto layout_main = new QVBoxLayout(central);

    auto button_decoloring = new QPushButton("Decoloring");
    layout_main->addWidget(button_decoloring);
    connect(button_decoloring, &QPushButton::clicked, table_widget, &Widget::on_decoloring_clicked);

    auto button_kmeans = new QPushButton("K-means coloring");
    layout_main->addWidget(button_kmeans);
    connect(button_kmeans, &QPushButton::clicked, table_widget, &Widget::on_coloring_kmeans_clicked);

    auto button_dbscan = new QPushButton("dbscan coloring");
    layout_main->addWidget(button_dbscan);
    connect(button_dbscan, &QPushButton::clicked, table_widget, &Widget::on_coloring_dbscan_clicked);
}

void Window_Cluster::on_kmeans_k_edited(const QString &text){
    table_widget->map_cluster_groups[Cluster_method::kmeans] = text.toInt();
}

void Window_Cluster::on_kmeans_iter_edited(const QString &text){
    table_widget->kmeans_maxiter = text.toInt();
}

void Window_Cluster::on_epsilon_edited(const QString &text){
    table_widget->dbscan_epsilon = text.toFloat();
}

void Window_Cluster::on_minPts_edited(const QString &text){
    table_widget->dbscan_minPts = text.toInt();
}
