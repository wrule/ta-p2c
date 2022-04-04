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

void strategy1(int fast, int slow) {
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

void strategy2(int fast, int slow) {
  int ifast = fast <= slow ? fast : slow;
  int islow = fast <= slow ? slow : fast;
  const double * all_inputs[] = { Close };

  const double options_fast[] = { ifast };
  const int start_fast = ti_sma_start(options_fast);
  double * all_outputs_fast[] = { &Indexs[0][start_fast] };
  ti_sma(HistLen, all_inputs, options_fast, all_outputs_fast);

  const double options_slow[] = { islow };
  const int start_slow = ti_sma_start(options_slow);
  double * all_outputs_slow[] = { &Indexs[1][start_slow] };
  ti_sma(HistLen, all_inputs, options_slow, all_outputs_slow);

  StablePoint = start_slow + 1;
}

void strategy3(int fast, int slow) {
  int ifast = fast <= slow ? fast : slow;
  int islow = fast <= slow ? slow : fast;
  const double * all_inputs[] = { Close };

  const double options_fast[] = { ifast };
  const int start_fast = ti_rsi_start(options_fast);
  double * all_outputs_fast[] = { &Indexs[0][start_fast] };
  ti_rsi(HistLen, all_inputs, options_fast, all_outputs_fast);

  const double options_slow[] = { islow };
  const int start_slow = ti_rsi_start(options_slow);
  double * all_outputs_slow[] = { &Indexs[1][start_slow] };
  ti_rsi(HistLen, all_inputs, options_slow, all_outputs_slow);

  StablePoint = start_slow + 1;
}

void strategy4(int rsi, int ma) {
  const double * all_inputs[] = { Close };

  const double options_rsi[] = { rsi };
  const int start_rsi = ti_rsi_start(options_rsi);
  double * all_outputs_rsi[] = { &Indexs[0][start_rsi] };
  ti_rsi(HistLen, all_inputs, options_rsi, all_outputs_rsi);

  const double * all_inputs_ma[] = { &Indexs[0][start_rsi] };

  const double options_ma[] = { ma };
  const int start_ma = ti_sma_start(options_ma) + start_rsi;
  double * all_outputs_ma[] = { &Indexs[1][start_ma] };
  ti_sma(HistLen - start_rsi, all_inputs_ma, options_ma, all_outputs_ma);

  StablePoint = start_ma + 1;
}

void strategy5(
  int rsi_length,
  int length,
  int k,
  int d
) {
  const double rsi_options[] = { rsi_length };
  const double * rsi_inputs[] = { Close };
  const int rsi_start = ti_rsi_start(rsi_options);
  double * rsi_outputs[] = { &Indexs[0][rsi_start] };
  ti_rsi(HistLen, rsi_inputs, rsi_options, rsi_outputs);

  // const double stoch_options[] = { k, length, d };
  // const double * stoch_inputs[] = {
  //   &Indexs[0][rsi_start],
  //   &Indexs[0][rsi_start],
  //   &Indexs[0][rsi_start]
  // };
  // double * stoch_outputs[] = { Indexs[1], Indexs[2] };
  // ti_stoch(HistLen - rsi_start, stoch_inputs, stoch_options, stoch_outputs);

  for (int i = 0; i < 20; ++i) {
    printf("%d %lf %lf\n", i, Close[i], Indexs[0][i]);
  }
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
  strategy5(13, 49, 8, 27);
}

double find() {
  // strategy(8, 44);
  test_func();
  return 0;
  printf("C >> 开始\n");
  time_t op = time(NULL);
  double max = -1.0;
  int max_fast = -1;
  int max_slow = -1;
  for (int fast = 2; fast < 2000; ++fast) {
    for (int slow = 2; slow < 2000; ++slow) {
      strategy4(fast, slow);
      double result = backing_test();
      if (result > max) {
        max = result;
        max_fast = fast;
        max_slow = slow;
      }
    }
  }
  printf("C >> 结束 秒数 %ld\n", time(NULL) - op);
  printf("C >> 结果 %lf fast %d slow %d\n", max, max_fast, max_slow);
  return max;
}
