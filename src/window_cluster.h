#ifndef WINDOW_CLUSTER_H
#define WINDOW_CLUSTER_H

#include "widget.h"

#include <QMainWindow>

class Window_Cluster : public QMainWindow
{
    Q_OBJECT
public:
    Widget *table_widget;

    explicit Window_Cluster(Widget *table_widget, QWidget *parent = nullptr);

    void on_kmeans_k_edited(const QString& text);

    void on_kmeans_iter_edited(const QString &text);

    void on_epsilon_edited(const QString &text);

    void on_minPts_edited(const QString &text);

signals:

};

class Window_Coloring : public QMainWindow
{
    Q_OBJECT
public:
    Window_Coloring(Widget *table_widget, QWidget *parent = nullptr);
};

#endif // WINDOW_CLUSTER_H
