#include <stdio.h>

void func(int const** p){
  p = new int*[10];
}

int main() {
  int const** p;
  func(p);
  return 0;
}