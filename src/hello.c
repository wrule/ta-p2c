#include <stdio.h>
#include <time.h>

#define HIST_LENGTH 1024000

typedef struct {
  unsigned long time;
  double open;
  double high;
  double low;
  double close;
  double volume;
  double indexs[11];
} OHLCV;

OHLCV hist[HIST_LENGTH] = { };

void fill_ohlcv(
  int index,
  unsigned long time,
  double open,
  double high,
  double low,
  double close,
  double volume
) {
  hist[index].time = time;
  hist[index].open = open;
  hist[index].high = high;
  hist[index].low = low;
  hist[index].close = close;
  hist[index].volume = volume;
}

void show_ohlcv(int index) {
  printf(
    "%lu %lf %lf %lf %lf %lf\n",
    hist[index].time,
    hist[index].open,
    hist[index].high,
    hist[index].low,
    hist[index].close,
    hist[index].volume
  );
}


int jimao(int a, int b) {
  int sum = a + b;
  printf("c语言调用: %d\n", sum);
  return sum;
}

int find() {
  printf("C >> 开始\n");
  time_t op = time(NULL);
  int sum = 0;
  for (int num1 = 0; num1 < 20000; ++num1) {
    for (int num2 = 0; num2 < 1000; ++num2) {
      for (int num3 = 0; num3 < 1000; ++num3) {
        sum += (num1 + num2 - num3);
      }
    }
  }
  printf("C >> 结束 秒数 %ld\n", time(NULL) - op);
  return sum;
}

int main() {
  printf("%d\n", find());
  printf("%ld\n", sizeof(OHLCV) * 60 * 60 * 24 * 365 * 10 / 1024 / 1024 / 1024);
  return 0;
}
