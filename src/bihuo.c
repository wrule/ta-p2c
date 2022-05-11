#include <stdio.h>


double long_assets = 0.0;
double long_funds = 0.0;
double short_assets = 0.0;
double short_funds = 0.0;

double funds = 0.0;

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
}

void close_short(double price) {
  funds += -(short_assets * price - short_funds);
}

int main() {
  printf("你好，世界\n");
  return 0;
}
