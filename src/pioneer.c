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
  int fast,
  int slow,
  int size,
  int k_num
) {
  const double macd_options[] = { fast, slow, size };
  const double * macd_inputs[] = { Close };
  const int macd_start = ti_macd_start(macd_options);
  double * macd_outputs[] = {
    &Indexs[0][macd_start],
    &Indexs[1][macd_start],
    &Indexs[2][macd_start]
  };
  ti_macd(HistLen, macd_inputs, macd_options, macd_outputs);
  StablePoint = macd_start + 1;

  for (int i = 0; i < k_num; ++i) {
    Indexs[3][i] = -1.0;
  }
  for (int i = k_num; i < HistLen; ++i) {
    double min = DBL_MAX;
    for (int h = i - k_num; h < i; ++h) {
      if (Low[h] < min) {
        min = Low[h];
      }
    }
    if (Low[i] < min) {
      if (Open[i] < min) {
        Indexs[3][i] = Open[i];
      } else {
        Indexs[3][i] = min;
      }
    } else {
      Indexs[3][i] = -1.0;
    }
  }

  printf("%d\n", StablePoint);
  for (int i = 0; i < 50; ++i) {
    printf("%d %lf %lf %lf %lf\n", i, Close[i], Indexs[2][i], Low[i], Indexs[3][i]);
  }
}
// 策略
void strategy(int cur) {
  if (
    Indexs[2][cur] > 0 &&
    Indexs[2][cur - 1] <= 0
  ) {
    buy(Close[cur]);
    return;
  }
  if (
    Indexs[2][cur] < 0 &&
    Indexs[2][cur - 1] >= 0
  ) {
    sell(Close[cur]);
    return;
  }
}
// 查找器
void finder() {
  for (int fast = 2; fast < 200; ++fast) {
    printf("# %d...\n", fast);
    for (int slow = 2; slow < 200; ++slow) {
      for (int size = 2; size < 200; ++size) {
        for (int k_num = 2; k_num < 100; ++k_num) {
          indicators(fast, slow, size, k_num);
          backing_test();
          if (funds > funds_max) {
            funds_max = funds;
            printf(
              "$ %lf [%d %d %d] {%d %d:%d %lf}\n",
              funds,
              fast, slow, size,
              win_count + loss_count, win_count, loss_count, 100.0 * win_count / (win_count + loss_count)
            );
          }
        }
      }
    }
  }
}

void test() {
  printf("你好，世界\n");
  indicators(7, 21, 12, 20);
  backing_test();
  printf(
    "$ %lf [%d %d %d] {%d %d:%d %lf}\n",
    funds,
    7, 21, 12,
    win_count + loss_count, win_count, loss_count, 100.0 * win_count / (win_count + loss_count)
  );
}
