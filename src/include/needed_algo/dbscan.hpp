#include <iostream>
#include <vector>

#include "Eigen/Dense"

using namespace Eigen;

// 定义点的类型
typedef std::vector<float> Point;

// 计算两个点之间的欧氏距离
float calculateDistance(const Point& p1, const Point& p2) {
  float distance = 0.0;
  for (size_t i = 0; i < p1.size(); ++i) {
    float diff = p1[i] - p2[i];
    distance += diff * diff;
  }
  return std::sqrt(distance);
}

// 获取给定点的邻居
std::vector<int> getNeighbors(const std::vector<Point>& dataset, int pointIdx,
                              float epsilon) {
  std::vector<int> neighbors;
  for (size_t i = 0; i < dataset.size(); ++i) {
    if (calculateDistance(dataset[pointIdx], dataset[i]) <= epsilon) {
      neighbors.push_back(i);
    }
  }
  return neighbors;
}

// 执行DBSCAN算法
void dbscanRecursive(const std::vector<Point>& dataset, int pointIdx,
                     float epsilon, int minPts, std::vector<int>& cluster,
                     std::vector<int>& visited) {
  visited[pointIdx] = 1;
  std::vector<int> neighbors = getNeighbors(dataset, pointIdx, epsilon);

  if (neighbors.size() >= static_cast<size_t>(minPts)) {
    for (size_t i = 0; i < neighbors.size(); ++i) {
      int neighborIdx = neighbors[i];
      if (!visited[neighborIdx]) {
        dbscanRecursive(dataset, neighborIdx, epsilon, minPts, cluster,
                        visited);
      }
    }
  }

  cluster.push_back(pointIdx);
}

// DBSCAN主函数
std::vector<int> dbscan(const std::vector<std::vector<float>>& in,
                        const float epsilon, const int minPts) {
  // 将输入数据vector<vector<float>>转换为vector<Point>类型
  int numPoints = in.size();
  std::vector<Point> dataset(numPoints);
  for (int i = 0; i < numPoints; ++i) {
    dataset[i] = in[i];
  }

  // cluster为分组标签，-1表示噪声点
  std::vector<int> cluster(numPoints, -1);
  std::vector<int> visited(numPoints, 0);
  int clusterIdx = 0;

  for (int i = 0; i < numPoints; ++i) {
    if (visited[i]) {
      continue;
    }

    std::vector<int> neighbors = getNeighbors(dataset, i, epsilon);

    if (neighbors.size() < static_cast<size_t>(minPts)) {
      visited[i] = 1;  // 标记为噪声点
    } else {
      std::vector<int> newCluster;
      dbscanRecursive(dataset, i, epsilon, minPts, newCluster, visited);

      for (size_t j = 0; j < newCluster.size(); ++j) {
        cluster[newCluster[j]] = clusterIdx;
      }

      clusterIdx++;
    }
  }

  return cluster;
}
