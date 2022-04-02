#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"

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

unsigned long Time[HIST_LENGTH];
double Open[HIST_LENGTH];
double High[HIST_LENGTH];
double Low[HIST_LENGTH];
double Close[HIST_LENGTH];
double Volume[HIST_LENGTH];
double * Indexs[1024];

void init_hist() {

}

void fill_ohlcv(
  int index,
  unsigned long time,
  double open,
  double high,
  double low,
  double close,
  double volume
) {
  Time[index] = time;
  Open[index] = open;
  High[index] = high;
  Low[index] = low;
  Close[index] = close;
  Volume[index] = volume;
}

void show_ohlcv(int index) {
  printf(
    "%lu %lf %lf %lf %lf %lf\n",
    Time[index],
    Open[index],
    High[index],
    Low[index],
    Close[index],
    Volume[index]
  );
}

void init_index(int index) {
  Indexs[index] = malloc(sizeof(double) * 21000);
}

TA_RetCode strategy() {
  TA_Integer outBeg;
  TA_Integer outNbElement;
  TA_RetCode retCode = TA_MA(
    0,
    21000 - 1,
    Close,
    3,
    TA_MAType_SMA,
    &outBeg,
    &outNbElement,
    Indexs[0]
  );
  retCode = TA_MA(
    0,
    21000 - 1,
    Close,
    44,
    TA_MAType_SMA,
    &outBeg,
    &outNbElement,
    Indexs[1]
  );
  return retCode;
}

double find() {
  init_index(0);
  init_index(1);
  printf("C >> 开始\n");
  time_t op = time(NULL);
  double sum = 0;
  for (int n2 = 0; n2 < 100; ++n2) {
    for (int n1 = 0; n1 < 1000; ++n1) {
      strategy();
      for (int current; current < 20000; ++current) {
        sum += (Low[current] - Open[current]);
      }
    }
  }
  printf("C >> 结束 结果 %lf 秒数 %ld\n", sum, time(NULL) - op);
  return sum;
}
