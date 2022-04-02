#include <stdio.h>

int jimao(int a, int b) {
  int sum = a + b;
  printf("c语言调用: %d\n", sum);
  return sum;
}

int find() {
  printf("开始\n");
  int sum = 0;
  for (int num1 = 0; num1 < 1000; ++num1) {
    for (int num2 = 0; num2 < 1000; ++num2) {
      for (int num3 = 0; num3 < 1000; ++num3) {
        sum += (num1 + num2 - num3);
      }
    }
  }
  printf("结束\n");
  return sum;
}

int main() {
  printf("%d\n", find());
  return 0;
}
