#include <stdio.h>
#include <time.h>
#include "/opt/homebrew/opt/ta-lib/include/ta-lib/ta_libc.h"

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

TA_Real    closePrice[400];
TA_Real    out[400];
TA_Integer outBeg;
TA_Integer outNbElement;

double find() {
  TA_RetCode retCode = TA_MA(
    0,
    399,
    &closePrice[0],
    30,
    TA_MAType_SMA,
    &outBeg,
    &outNbElement,
    &out[0]
  );
  printf("C >> 开始\n");
  time_t op = time(NULL);
  double sum = 0;
  for (int n2 = 0; n2 < 1000000; ++n2) {
    for (int n1 = 0; n1 < 1000; ++n1) {
      for (int current; current < 20000; ++current) {
        sum += (hist[current].low - hist[current].open);
      }
    }
  }
  printf("C >> 结束 结果 %lf 秒数 %ld\n", sum, time(NULL) - op);
  return sum;
}
