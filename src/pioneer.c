#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
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
int StablePoint = 0;
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

void strategy(int cur);
void finder();

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

/**
 * @brief
 * 设置OHLCV数据
 * @param index 索引
 * @param time 时间
 * @param open 开盘价
 * @param high 最高价
 * @param low 最低价
 * @param close 收盘价
 * @param volume 成交量
 */
void set_ohlcv(
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

/**
 * @brief
 * 显示OHLCV数据
 * @param index 数据索引
 */
void show_ohlcv(int index) {
  printf(
    "Time: %lu Open: %lf High: %lf Low: %lf Close: %lf Volume: %lf\n",
    Time[index],
    Open[index],
    High[index],
    Low[index],
    Close[index],
    Volume[index]
  );
}

/**
 * @brief
 * 用于回测的状态重置
 */
void reset_backing_test() {
  funds = init_funds;
  assets = 0.0;
  funds_buy = 0.0;
  win_count = 0;
  loss_count = 0;
}

/**
 * @brief
 * 用于查找器的状态重置
 */
void reset_finder() {
  reset_backing_test();
  funds_max = 0.0;
}

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

/**
 * @brief
 * 回测
 */
void backing_test() {
  reset_backing_test();
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

/**
 * @brief
 * 查找
 */
void find() {
  reset_finder();
  printf("Finder开始...\n");
  time_t op = time(NULL);
  finder();
  printf("Finder完成 秒数 %ld\n", time(NULL) - op);
}


// 用户代码 ----------------------------------------------------------------
// 指标
void indicators(
  int rsi_length,
  int length,
  int k,
  int d,
  int k_num,
  int cross_num
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

  for (int i = k_num; i < HistLen; ++i) {
    double min = DBL_MAX;
    for (int h = i - k_num; h < i; ++h) {
      if (Low[h] < min) {
        min = Low[h];
      }
    }
    if (Low[i] < min) {
      if (High[i] < min) {
        Indexs[3][i] = High[i];
      } else {
        Indexs[3][i] = min;
      }
    } else {
      Indexs[3][i] = -1.0;
    }
  }

  if (StablePoint < k_num) {
    StablePoint = k_num;
  }

  // for (int i = 0; i < 30; ++i) {
  //   printf("%d %lf %lf\n", i, Low[i], Indexs[3][i]);
  // }
}
// 策略
void strategy(int cur) {
  if (
    Indexs[0][cur] > Indexs[1][cur] &&
    Indexs[0][cur - 1] <= Indexs[1][cur - 1]
  ) {
    buy(Close[cur]);
    return;
  }
  if (
    // Indexs[0][cur] < Indexs[1][cur] &&
    // Indexs[0][cur - 1] >= Indexs[1][cur - 1]
    Indexs[3][cur] > 0
  ) {
    sell(Indexs[3][cur]);
    return;
  }
}
// 查找器
void finder() {
  for (int rsi_length = 2; rsi_length < 16; ++rsi_length) {
    printf("# %d...\n", rsi_length);
    for (int length = 30; length < 70; ++length) {
      for (int k = 2; k < 20; ++k) {
        for (int d = 10; d < 40; ++d) {
          for (int k_num = 2; k_num < 50; ++k_num) {
            indicators(rsi_length, length, k, d, k_num, 2);
            backing_test();
            if (funds > funds_max) {
              funds_max = funds;
              printf(
                "$ %lf [%d %d %d %d %d] {%d %d:%d %lf}\n",
                funds_max,
                rsi_length, length, k, d, k_num,
                win_count + loss_count, win_count, loss_count, 100.0 * win_count / (win_count + loss_count)
              );
            }
          }
        }
      }
    }
  }
}

void test() {
  int rsi_length = 8, length = 49, k = 8, d = 27;
  indicators(rsi_length, length, k, d, 25, 2);
  backing_test();
  printf(
    "$ %lf [%d %d %d %d] {%d %d:%d %lf}\n",
    funds,
    rsi_length, length, k, d,
    win_count + loss_count, win_count, loss_count, 100.0 * win_count / (win_count + loss_count)
  );
}
