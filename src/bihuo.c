#include <stdio.h>


double long_assets = 0.0;
double long_funds = 0.0;
double short_assets = 0.0;
double short_funds = 0.0;

double funds = 100.0;

void open_long(double amount, double price) {
  long_assets += amount;
  const double use_funds = amount * price;
  long_funds += use_funds;
}

void open_short(double amount, double price) {
  short_assets += amount;
  const double use_funds = amount * price;
  short_funds += use_funds;
}

void close_long(double price) {
  funds += long_assets * price - long_funds;
  long_assets = 0.0;
  long_funds = 0.0;
}

void close_short(double price) {
  funds += -(short_assets * price - short_funds);
  short_assets = 0.0;
  short_funds = 0.0;
}

int main() {
  printf("你好，世界\n");
  open_long(100, 10);
  close_long(11);
  printf("%lf\n", funds);
  open_short(100, 10);
  close_short(8);
  printf("%lf\n", funds);
  return 0;
}
