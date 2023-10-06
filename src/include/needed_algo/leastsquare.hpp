#ifndef EXAMPLES_HPP
#define EXAMPLES_HPP

#include "common.h"

std::tuple<Eigen::VectorXf, float, float>
fitLeastSquareAndPR(const std::vector<float> &inX, const std::vector<float> &inY, const int inDegree)
{
    if (inX.size() != inY.size())
    {
        throw std::invalid_argument("inX.size() != inY.size()");
    }

    if (inDegree <= 0)
    {
        throw std::invalid_argument("inDegree <= 0");
    }

    if (inX.empty())
    {
        throw std::invalid_argument("inX.empty()");
    }

    Eigen::MatrixXf A(inX.size(), inDegree + 1);
    Eigen::VectorXf b(inY.size());

    for (size_t i = 0; i < inX.size(); i++)
    {
        for (int j = 0; j < inDegree + 1; j++)
        {
            A(i, j) = float(pow(inX[i], j));
        }
        b(i) = inY[i];
    }

    Eigen::VectorXf x = A.colPivHouseholderQr().solve(b);

    float pValue = 0;
    for (size_t i = 0; i < inX.size(); i++)
    {
        float y = 0;
        for (int j = 0; j < inDegree + 1; j++)
        {
            y += x(j) * float(pow(inX[i], j));
        }
        pValue += float(pow(y - inY[i], 2));
    }

    float r2Value = 0;
    float avg = b.mean();
    for (size_t i = 0; i < inX.size(); i++)
    {
        r2Value += float(pow(b(i) - avg, 2));
    }
    r2Value = 1 - pValue / r2Value;

    return { x, pValue, r2Value };
}

void testLesatSquare()
{
    std::vector<float> x = {1, 2, 3, 4, 5};
    std::vector<float> y = {1, 4, 9, 16, 25};

    auto res1 = fitLeastSquareAndPR(x, y, 1);
    std::cout << "res1: \n" << std::get<0>(res1) << std::endl;
    std::cout << "p = " << std::get<1>(res1) << std::endl;
    std::cout << "r2 = " << std::get<2>(res1) << std::endl;
    std::cout << std::endl;

    auto res2 = fitLeastSquareAndPR(x, y, 2);
    std::cout << "res2: \n" << std::get<0>(res2) << std::endl;
    std::cout << "p = " << std::get<1>(res2) << std::endl;
    std::cout << "r2 = " << std::get<2>(res2) << std::endl;
    std::cout << std::endl;

    auto res3 = fitLeastSquareAndPR(x, y, 3);
    std::cout << "res3: \n" << std::get<0>(res3) << std::endl;
    std::cout << "p = " << std::get<1>(res3) << std::endl;
    std::cout << "r2 = " << std::get<2>(res3) << std::endl;
    std::cout << std::endl;
}
#endif // EXAMPLES_HPP
