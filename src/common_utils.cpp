#include "include/common_utils.h"

/**
 * @brief 美化散点图中的点。
 * 
 * @param scatter_series 需要美化的散点系列。 
 */
void beautify_scatter_series(QScatterSeries *scatter_series) {
    scatter_series->setOpacity(0.7);
    scatter_series->setMarkerSize(5);
    QPen pen = scatter_series->pen();
    pen.setWidth(0);
    scatter_series->setPen(pen);
}
