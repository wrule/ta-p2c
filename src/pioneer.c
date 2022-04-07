#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"
#include "indicators.h"

int x_queue_end;

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
  x_queue_end = 0;
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
  // MACD指标生成
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

  // ATR指标生成
  const double atr_options[] = { 5 };
  const double * atr_inputs[] = { High, Low, Close };
  const int atr_start = ti_atr_start(atr_options);
  double * atr_outputs[] = { &Indexs[4][atr_start] };
  ti_atr(HistLen, atr_inputs, atr_options, atr_outputs);

  if (StablePoint < atr_start) {
    StablePoint = atr_start;
  }

  // 离场指标生成
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

  if (StablePoint < k_num) {
    StablePoint = k_num;
  }

  // printf("%d\n", StablePoint);
  // for (int i = 0; i < 100; ++i) {
  //   printf("%d %lf %lf %lf\n", i, Indexs[2][i], Indexs[4][i], Indexs[3][i]);
  // }
}

#define X_QUEUE_SIZE 10240
double x_queue[X_QUEUE_SIZE][4] = { };
int x_queue_end = 0;
void x_queue_push(double bar, double high, double low, double atr) {
  int index = x_queue_end % X_QUEUE_SIZE;
  x_queue[index][0] = bar;
  x_queue[index][1] = high;
  x_queue[index][2] = low;
  x_queue[index][3] = atr;
  x_queue_end++;
}
void x_queue_show_tail(int size) {
  int start = x_queue_end - size;
  if (start < 0) {
    start = 0;
  }
  for (int i = start; i < x_queue_end; ++i) {
    int index = i % X_QUEUE_SIZE;
    printf(
      "%lf %lf %lf %lf\n",
      x_queue[index][0],
      x_queue[index][1],
      x_queue[index][2],
      x_queue[index][3]
    );
  }
}
double x_queue_high(int size) {
  int start = x_queue_end - size;
  if (start < 0) {
    start = 0;
  }
  double max = DBL_MIN;
  double atr = 0;
  for (int i = start; i < x_queue_end; ++i) {
    int index = i % X_QUEUE_SIZE;
    if (x_queue[index][1] > max) {
      max = x_queue[index][1];
      atr = x_queue[index][3];
    }
  }
  return max;
}

// 策略
void strategy(int cur) {
  // 记录金叉
  if (
    Indexs[2][cur] > 0 &&
    Indexs[2][cur - 1] <= 0
  ) {
    buy(Close[cur]);
    // if (x_queue_end >= 3) {
    //   const double high = x_queue_high(3);
    //   if (Close[cur] > high) {
    //     buy(Close[cur]);
    //   }
    // }
    // x_queue_push(cur, High[cur], Low[cur], Indexs[4][cur]);
    return;
  }
  // 记录死叉
  // if (
  //   Indexs[2][cur] < 0 &&
  //   Indexs[2][cur - 1] >= 0
  // ) {
  //   sell(Close[cur]);
  //   // x_queue_push(cur, High[cur], Low[cur], Indexs[4][cur]);
  //   return;
  // }
  if (
    Indexs[3][cur] > 0
  ) {
    sell(Indexs[3][cur]);
    return;
  }
}
// 查找器
void finder() {
  for (int fast = 5; fast < 50; ++fast) {
    printf("# %d...\n", fast);
    for (int slow = fast + 1; slow < 100; ++slow) {
      for (int size = 2; size < 100; ++size) {
        for (int k_num = 2; k_num < 100; ++k_num) {
          indicators(fast, slow, size, k_num);
          backing_test();
          if (funds > funds_max) {
            funds_max = funds;
            printf(
              "$ %lf [%d %d %d %d] {%d %d:%d %lf}\n",
              funds,
              fast, slow, size, k_num,
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
