#ifndef PCA_HPP
#define PCA_HPP

#include "common.h"

Eigen::MatrixXf pca(const std::vector<std::vector<float>> &in, const int k)
{
    if (in.empty())
    {
        throw std::invalid_argument("in.empty()");
    }

    if (k <= 0)
    {
        throw std::invalid_argument("k <= 0");
    }

    size_t row = in.size();
    size_t col = in[0].size();

    Eigen::MatrixXf mat(row, col);
    for (size_t i = 0; i < row; i++)
    {
        if (in[i].size() != col)
        {
            throw std::invalid_argument("in[i].size() != col");
        }
        for (size_t j = 0; j < col; j++)
        {
            mat(i, j) = in[i][j];
        }
    }

    Eigen::VectorXf avg = mat.colwise().mean();
    Eigen::MatrixXf centered = mat.rowwise() - avg.transpose();

    Eigen::MatrixXf cov = centered.adjoint() * centered;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eig(cov);
    Eigen::MatrixXf eigenVectors = eig.eigenvectors();

    Eigen::MatrixXf result = centered * eigenVectors.rightCols(k);

    return result;
}

void testPCA()
{
    std::vector<std::vector<float>> highDimPoints = {
        {2.5f, 1.2f, 0.01f},
        {3.5f, 2.5f, 0.011f},
        {4.5f, 3.5f, 0.012f},
        {5.5f, 4.5f, 0.013f},
        {6.5f, 5.5f, 0.014f},
        {7.5f, 6.5f, 0.015f},
        {8.5f, 7.5f, 0.016f},
        {9.5f, 8.5f, 0.017f},
        {10.5f, 9.5f, 0.018f},
        {11.5f, 10.5f, 0.019f},
        {12.5f, 11.5f, 0.020f},
        {13.5f, 12.5f, 0.021f},
        {14.5f, 13.5f, 0.022f},
        {15.5f, 14.5f, 0.023f},
        {16.5f, 15.5f, 0.024f},
        {17.5f, 16.5f, 0.025f},
        {18.5f, 17.5f, 0.026f},
        {19.5f, 18.5f, 0.027f},
        {20.5f, 19.5f, 0.028f},
        {21.5f, 20.5f, 0.029f},
        {22.5f, 21.5f, 0.030f},
        {23.5f, 22.5f, 0.031f},
        {24.5f, 23.5f, 0.032f},
        {25.5f, 24.5f, 0.033f},
        {26.5f, 25.5f, 0.034f},
        {27.5f, 26.5f, 0.035f},
        {28.5f, 27.5f, 0.036f},
        {29.5f, 28.5f, 0.037f}
    };
    auto res = pca(highDimPoints, 2);
    std::cout << "res: \n" << res << std::endl;
}

#endif // PCA_HPP
