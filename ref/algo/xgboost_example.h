#ifndef XGBOOST_EXAMPLE_H
#define XGBOOST_EXAMPLE_H

#include <cstdlib>
#include <xgboost/c_api.h>

#define safe_xgboost(call) { \
int err = (call); \
    if (err != 0) { \
        fprintf(stderr, "%s:%d: error in %s: %s\\n", __FILE__, __LINE__, #call, XGBGetLastError()); \
        exit(1); \
} \
}

void testXgboost() {
    // Load data from memory
    float train[4][2] = {{1,2}, {3,4}, {5,6}, {7,8}};
    float label[4] = {0,1, 0, 1};
    DMatrixHandle dtrain;
    safe_xgboost(XGDMatrixCreateFromMat((float *) train, 4, 2, -1, &dtrain));
    safe_xgboost(XGDMatrixSetFloatInfo(dtrain, "label", label, 4)); // set label parameter
    // Create booster
    BoosterHandle booster;
    safe_xgboost(XGBoosterCreate(&dtrain, 1, &booster));

    // Set parameters
    safe_xgboost(XGBoosterSetParam(booster, "objective", "reg:squarederror"));
    safe_xgboost(XGBoosterSetParam(booster, "max_depth", "3"));
    safe_xgboost(XGBoosterSetParam(booster, "eta", "0.1"));

    // Train for 10 iterations
    for (int iter = 0; iter < 2; iter++) {
        safe_xgboost(XGBoosterUpdateOneIter(booster, iter, dtrain));
    }

    // Use train data as test data
    DMatrixHandle dtest = dtrain;

    // Predict
    bst_ulong out_len;
    const float* out_result;
    safe_xgboost(XGBoosterPredict(booster, dtest, 4, 2, 0, &out_len, &out_result));

    // Print predictions
    for (int i = 0; i < out_len; i++) {
        printf("%f\n", out_result[i]);
    }

    safe_xgboost(XGBoosterPredict(booster, dtest, 4, 2, 1, &out_len, &out_result));

    // Print feature contributions
    for (int i = 0; i < out_len; i++) {
        printf("%f\n", out_result[i]);
    }

}
#endif // XGBOOST_EXAMPLE_H
