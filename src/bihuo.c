#include <stdio.h>


double long_assets = 0.0;
double long_funds = 0.0;
double short_assets = 0.0;
double short_funds = 0.0;

double funds = 100.0;

void long_open(double price, double asset) {
  long_assets += asset;
  const double use_funds = asset * price;
  long_funds += use_funds;
}

void long_close(double price) {
  funds += long_assets * price - long_funds;
  long_assets = 0.0;
  long_funds = 0.0;
}

double long_price() {
  return long_funds / long_assets;
}


int main() {
  printf("你好，世界\n");
  long_open(100, 10);
  printf("%lf\n", funds);
  return 0;
}
