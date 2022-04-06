#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"
#include "indicators.h"

int HistLen = 0;

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
  double * rsi_outputs[] = { &Indexs[2][rsi_start] };
  ti_rsi(HistLen, rsi_inputs, rsi_options, rsi_outputs);
  const double stoch_options[] = { length, k, d };
  const double * stoch_inputs[] = {
    &Indexs[2][rsi_start],
    &Indexs[2][rsi_start],
    &Indexs[2][rsi_start]
  };
  const int stoch_start = ti_stoch_start(stoch_options) + rsi_start;
  double * stoch_outputs[] = { &Indexs[0][stoch_start], &Indexs[1][stoch_start] };
  ti_stoch(HistLen - rsi_start, stoch_inputs, stoch_options, stoch_outputs);
  StablePoint = stoch_start + 1;
  // for (int i = 0; i < 100; ++i) {
  //   printf("%d %lf %lf %lf\n", i, Close[i], Indexs[0][i], Indexs[1][i]);
  // }
}

void populate_indicators() {

}

// 初始资金
double init_funds = 100.0;
// 实时资金
double funds = 0.0;
// 实时资产
double assets = 0.0;
// 费率乘数
double fee = 0.9985;
// 上一次购买资金
double funds_buy = 0.0;
// 历史最大资金
double funds_max = 0.0;
// 盈利交易计数
int win_count = 0;
// 亏损交易计数
int loss_count = 0;

/**
 * @brief
 * 现货购买
 * @param price 购买价格
 * @return int 成功：0，失败：1
 */
int buy(double price) {
  if (assets == 0) {
    assets = funds / price * fee;
    funds_buy = funds;
    funds = 0;
    return 0;
  }
  return 1;
}

/**
 * @brief
 * 现货销售
 * @param price 销售价格
 * @return int 成功：0，失败：1
 */
int sell(double price) {
  if (funds == 0) {
    funds = assets * price * fee;
    if (funds >= funds_buy) {
      win_count++;
    } else {
      loss_count++;
    }
    assets = 0;
    return 0;
  }
  return 1;
}

// 临时
void strategy(int cur) {
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


void backing_test() {
  funds = init_funds;
  assets = 0;
  for (int cur = 0; cur < HistLen; ++cur) {
    if (cur == HistLen - 1) {
      sell(Close[cur]);
      break;
    }
    if (cur >= StablePoint) {
      strategy(cur);
    }
  }
}

void test_func() {
  strategy5(8, 49, 8, 27);
  backing_test();
  printf("%lf\n", funds);
}

void find() {
  // strategy(8, 44);
  // test_func();
  // return 0;
  printf("C >> 开始\n");
  time_t op = time(NULL);
  for (int rsi_length = 8; rsi_length < 200; ++rsi_length) {
    printf("# %d...\n", rsi_length);
    for (int length = 2; length < 200; ++length) {
      for (int k = 2; k < 100; ++k) {
        for (int d = 2; d < 100; ++d) {
          strategy5(rsi_length, length, k, d);
          backing_test();
          if (funds > funds_max) {
            funds_max = funds;
            printf("$ %lf %d %d %d %d\n", funds_max, rsi_length, length, k, d);
          }
        }
      }
    }
  }
  printf("C >> 结束 秒数 %ld\n", time(NULL) - op);
}
