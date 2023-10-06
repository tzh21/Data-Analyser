#ifndef WINDOW_ML_H
#define WINDOW_ML_H

#include <QMainWindow>
#include <QComboBox>
#include <QTableWidget>
#include <QLineEdit>

class Window_ML : public QMainWindow
{
    Q_OBJECT
public:
    explicit Window_ML(
        std::vector<int> &&_diagnosis,
        std::vector<std::string> &&_feature_names,
        std::vector<std::vector<float>> &&_samples,
        QWidget *parent = nullptr);

signals:

private:
    const std::vector<int> diagnosis;
    const std::vector<std::string> feature_names;
    const std::vector<std::vector<float>> samples;

    std::vector<int> idx_train;
    std::vector<int> idx_test;
    std::vector<std::pair<std::string, float>> feature_importance;

    float cnt_true_positive = 0;
    float cnt_true_negative = 0;
    float cnt_false_positive = 0;
    float cnt_false_negative = 0;

    QComboBox *comb_ratio;

    QLineEdit *edit_max_depth;
    QLineEdit *edit_iter;
    QLineEdit *edit_f1score;
    QLineEdit *edit_auc;

    QTableWidget *table_train;
    QTableWidget *table_test;

    void on_button_ratio_clicked();
    void on_button_train_clicked();
    void on_button_test_result_clicked();
    void on_button_feature_clicked();
};

#endif // WINDOW_ML_H
