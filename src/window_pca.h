#ifndef WINDOW_PCA_H
#define WINDOW_PCA_H

#include "widget.h"

#include <QBoxLayout>
#include <QMainWindow>
#include <QChart>
#include <QChartView>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <Q3DScatter>
#include <QLineEdit>
#include <QVector3D>
#include <QPointF>

class Window_PCA2D : public QWidget
{
    Q_OBJECT
public:
    explicit Window_PCA2D(
        const std::vector<std::vector<float>> &variants,
        const std::vector<int> &labels,
        const size_t cnt_groups,
        QWidget *parent = nullptr);


signals:

private:
    int dim_reduced = 2;

    QLineEdit *edit_group;
    QLineEdit *edit_col;
    QLineEdit *edit_1st;
    QLineEdit *edit_2nd;

    std::map<std::string, int> map_xy_col;
    // std::map<QPointF, int> map_point_col;
    // std::map<const QPointF*, int> map_point_col;
    // std::map<const QPointF&, int> map_point_col;

    void on_group_hovered(int index);
    void onPointHovered(const QPointF &point, bool state);
};

class Window_PCA : public QMainWindow{
    Q_OBJECT
public:
    Window_PCA(
        Widget *_table_widget,
        std::vector<std::vector<float>> &&_variants,
        std::vector<int> &&_diagnosis,
        QWidget *parent = nullptr);

    std::vector<std::vector<QVector3D>> bm_groups;
    std::vector<std::vector<QVector3D>> cluster_groups;

    // void onPointHovered(int index);
    // void on_group_hovered(int index);

private:
    Widget *table_widget;

    Cluster_method cluster_method = Cluster_method::kmeans;

    const std::vector<std::vector<float>> variants;
    const std::vector<int> diagnosis;

    QPushButton *button_2d;
    QPushButton *button_3d;

    int selected_group = 0;

    void on_radio_kmeans_toggled(bool checked);
    void on_radio_dbscan_toggled(bool checked);

    void on_button_2dcluster_clicked();
    void on_button_3dcluster_clicked();

    void on_button_2d_clicked();
    void on_button_3d_clicked();
};

class Window_PCA3D : public Q3DScatter{
    Q_OBJECT
public:
    explicit Window_PCA3D(
        std::vector<QLineEdit*> &edits,
        const std::vector<std::vector<float>> &variants,
        const std::vector<int> &labels,
        const size_t cnt_groups);

private:
    int selected_group = 0;
    QLineEdit *edit_group;
    QLineEdit *edit_col;
    QLineEdit *edit_1st;
    QLineEdit *edit_2nd;
    QLineEdit *edit_3rd;

    std::map<std::string, int> map_xyz_col;

    void on_group_selected(int index);
    void on_item_selected(int index);
};

#endif // WINDOW_PCA_H
