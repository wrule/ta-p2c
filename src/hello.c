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

unsigned long * Time;
double * Open;
double * High;
double * Low;
double * Close;
double * Volume;
double * Indexs[1024];

void init_hist() {
  Time = malloc(sizeof(unsigned long) * HIST_LENGTH);
  Open = malloc(sizeof(double) * HIST_LENGTH);
  High = malloc(sizeof(double) * HIST_LENGTH);
  Low = malloc(sizeof(double) * HIST_LENGTH);
  Close = malloc(sizeof(double) * HIST_LENGTH);
  Volume = malloc(sizeof(double) * HIST_LENGTH);
}

void init_indexs(int size) {
  for (int i = 0; i < size; ++i) {
    Indexs[i] = malloc(sizeof(double) * HIST_LENGTH);
  }
}

void init() {
  init_hist();
  init_indexs(2);
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
