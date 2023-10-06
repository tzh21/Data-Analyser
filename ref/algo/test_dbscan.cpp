#include "dbscan.hpp"

int main() {  // 测试数据
  std::vector<std::vector<float>> data = {
      {1.0, 2.0},   {1.5, 2.5},   {3.0, 4.0},   {3.5, 4.5},  {10.0, 11.0},
      {10.5, 11.5}, {11.0, 11.0}, {11.5, 11.5}, {20.0, 21.0}};
  float epsilon = 1.0;
  int minPts = 2;

  std::vector<int> cluster = dbscan(data, epsilon, minPts);

  for (int i = 0; i < cluster.size(); ++i) {
    std::cout << "Point " << i << " belongs to cluster " << cluster[i]
              << std::endl;
  }

  return 0;
}