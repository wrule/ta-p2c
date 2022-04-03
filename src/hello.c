#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"
#include "indicators.h"

int HistLen = 0;

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
  Time = malloc(sizeof(unsigned long) * HistLen);
  Open = malloc(sizeof(double) * HistLen);
  High = malloc(sizeof(double) * HistLen);
  Low = malloc(sizeof(double) * HistLen);
  Close = malloc(sizeof(double) * HistLen);
  Volume = malloc(sizeof(double) * HistLen);
}

void init_indexs(int size) {
  for (int i = 0; i < size; ++i) {
    Indexs[i] = malloc(sizeof(double) * HistLen);
  }
}

void init(int len, int index_size) {
  HistLen = len;
  init_hist();
  init_indexs(index_size);
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

int StablePoint = 0;

void strategy(int fast, int slow) {
  int ifast = fast <= slow ? fast : slow;
  int islow = fast <= slow ? slow : fast;
  StablePoint = islow;
  TA_Integer data_op;
  TA_Integer data_size;
  TA_MA(
    0,
    HistLen - 1,
    Close,
    ifast,
    TA_MAType_SMA,
    &data_op,
    &data_size,
    &Indexs[0][ifast - 1]
  );
  TA_MA(
    0,
    HistLen - 1,
    Close,
    islow,
    TA_MAType_SMA,
    &data_op,
    &data_size,
    &Indexs[1][islow - 1]
  );
  // for (int i = 0; i < 10; ++i) {
  //   printf("%d %lf %lf\n", i, Close[i], Indexs[0][i]);
  // }
  // return retCode;
}

double funds = 100.0;
double assets = 0.0;
double fee = 0.999;

int buy(double price) {
  if (assets == 0) {
    assets = funds / price * fee;
    funds = 0;
    return 0;
  }
  return 1;
}

int sell(double price) {
  if (funds == 0) {
    funds = assets * price * fee;
    assets = 0;
    return 0;
  }
  return 1;
}

double backing_test() {
  funds = 100.0;
  assets = 0;
  for (int cur = 0; cur < HistLen; ++cur) {
    if (cur == HistLen - 1) {
      sell(Close[cur]);
      break;
    }
    if (cur >= StablePoint) {
      if (
        Indexs[0][cur] > Indexs[1][cur] &&
        Indexs[0][cur - 1] <= Indexs[1][cur - 1]
      ) {
        buy(Close[cur]);
      }
      if (
        Indexs[0][cur] < Indexs[1][cur] &&
        Indexs[0][cur - 1] >= Indexs[1][cur - 1]
      ) {
        sell(Close[cur]);
      }
    }
  }
  return funds;
}

void test_func() {
  printf("你好，世界\n");
  /* Order data from oldest to newest (index 0 is oldest) */
  const double data_in[] = {5,8,12,11,9,8,7,10,11,13};
  const int input_length = sizeof(data_in) / sizeof(double); /* 10 in this example */
  const double options[] = {3};
  /* Find start size for given options. */
  const int start = ti_sma_start(options);
  printf("%d\n", start);

  /* Output length is input length minus start size. */
  const int output_length = input_length - start;

  double *data_out = malloc(output_length * sizeof(double));
  assert(data_out != 0);

  const double *all_inputs[] = {data_in};
  double *all_outputs[] = {data_out};

  int error = ti_sma(input_length, all_inputs, options, all_outputs);
  assert(error == TI_OKAY);

  for (int i = 0; i < output_length; ++i) {
    printf("%d %lf\n", i, data_out[i]);
  }
}

double find() {
  // strategy(8, 44);
  // test_func();
  // return 0;
  printf("C >> 开始\n");
  time_t op = time(NULL);
  double max = -1.0;
  for (int fast = 2; fast < 2000; ++fast) {
    for (int slow = fast + 1; slow <= 2000; ++slow) {
      strategy(fast, slow);
      double result = backing_test();
      if (result > max) {
        max = result;
      }
    }
  }
  printf("C >> 结束 结果 %lf 秒数 %ld\n", max, time(NULL) - op);
  return max;
}
