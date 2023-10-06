#include <iostream>
#include "leastsquare.hpp"
#include "rowfeature.hpp"
#include "covariance.hpp"
#include "kmeans.hpp"
#include "pca.hpp"
#include "xgboost_example.h"

int main()
{
    testAvgVar();
    testLesatSquare();
    testCovariance();
    testCluster();
    testPCA();
    testXgboost();

    return 0;
}
