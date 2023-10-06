#ifndef COVARIANCE_HPP
#define COVARIANCE_HPP

#include "common.h"
#include "rowfeature.hpp"

Eigen::MatrixXf getCovariance(const std::vector<std::vector<float>> &inMat)
{
    if (inMat.empty())
    {
        throw std::invalid_argument("inMat.empty()");
    }

    size_t row = inMat.size();
    size_t col = inMat[0].size();

    Eigen::MatrixXf mat(row, col);
    for (size_t i = 0; i < row; i++)
    {
        if (inMat[i].size() != col)
        {
            throw std::invalid_argument("inMat[i].size() != col");
        }
        for (size_t j = 0; j < col; j++)
        {
            mat(i, j) = inMat[i][j];
        }
    }
    mat.transposeInPlace();

    Eigen::MatrixXf centered = mat.rowwise() - mat.colwise().mean();
    Eigen::MatrixXf cov = (centered.adjoint() * centered) / double(mat.rows() - 1);

    return cov;
}

Eigen::MatrixXf getPearsonCorr(const Eigen::MatrixXf &cov, const std::vector<float> &vars)
{
    if (vars.empty())
    {
        throw std::invalid_argument("vars.empty()");
    }

    size_t row = cov.rows();
    size_t col = cov.cols();

    if (row != col)
    {
        throw std::invalid_argument("row != col");
    }

    if (row != vars.size())
    {
        throw std::invalid_argument("row != vars.size()");
    }

    Eigen::MatrixXf relativity(row, col);
    for (size_t i = 0; i < row; i++)
    {
        relativity(i, i) = 1;
        for (size_t j = i + 1; j < col; j++)
        {
            relativity(i, j) = cov(i, j) / sqrt(vars[i] * vars[j]);
            relativity(j, i) = relativity(i, j);
        }
    }
    return relativity;
}

void testCovariance()
{
    std::vector<std::vector<float>> mat = {
        {1.2f, 2.3f, 3.4f, 8.8f},
        {4.5f, 5.6f, 6.7f, 7.2f}
    };

    auto cov = getCovariance(mat);
    std::cout << "cov: \n" << cov << std::endl;

    std::vector<float> vars;
    for (auto vec : mat)
    {
        auto avgVar = getAvgVar(vec);
        vars.push_back(std::get<1>(avgVar));
    }
    auto rel = getPearsonCorr(cov, vars);
    std::cout << "pearson corr: \n" << rel << std::endl;
}


#endif // COVARIANCE_HPP
