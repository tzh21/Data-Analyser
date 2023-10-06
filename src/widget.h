#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStandardItem>
#include <QTableView>
#include <QMainWindow>
#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

enum class Cluster_method{
    kmeans,
    dbscan
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 聚类方法到设定的聚类数的映射
    std::map<Cluster_method, int> map_cluster_groups{{Cluster_method::kmeans, 4},
                                                     {Cluster_method::dbscan, 8}};

    // 聚类方法到设定的聚类颜色的映射
    std::map<Cluster_method, int> map_cluster_col{{Cluster_method::kmeans, 10},
                                                     {Cluster_method::dbscan, 11}};

    // K-means聚类的最大迭代次数
    int kmeans_maxiter = 100;
    // dbscan聚类的参数
    float dbscan_epsilon = 15;
    int dbscan_minPts = 3;

    std::vector<int> get_labels_of(Cluster_method method = Cluster_method::kmeans);

    void on_decoloring_clicked();

    void on_cluster_kmeans_clicked();

    void on_coloring_kmeans_clicked();

    void on_cluster_dbscan_clicked();

    void on_coloring_dbscan_clicked();

private slots:
    void on_button_variance_clicked();

    void on_button_barchart_clicked();

    void on_button_scatter_clicked();

    void on_button_open_clicked();

    void on_button_covariance_clicked();

    void on_button_pca_clicked();

    void on_button_cluster_clicked();

    void on_button_coloring_clicked();

    void on_button_ml_clicked();

private:
    // 诊断结果所在列
    int col_diagnosis_at = 1;

    Ui::Widget *ui;

    // 数据表的路径
    QString path_table = ":/resource/breast-cancer.csv";

    // 数据表模型
    QStandardItemModel *model{new QStandardItemModel(this)};

//    tableView set in ui file
//    QTableView *view{new QTableView(this)};

    // 数据表图表
    QChart* chart{new QChart};

    void open_table();

    void delete_obj(QObject *obj);

    void add_cluster(Cluster_method method, std::vector<int> labels);

    void coloring_method(Cluster_method method);

    std::vector<std::vector<float>> samples_selected(size_t least_cols = 1);
};
#endif // WIDGET_H
