#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"
#include "indicators.h"

#define INDEXS_SIZE 1024

int Hist_Len = 0;
int Indexs_Size = 0;
int Valuation_Index = 0;
unsigned long * Time;
double * Open;
double * High;
double * Low;
double * Close;
double * Volume;
double * Indexs[INDEXS_SIZE];
int Stable_Point = 0;
// 初始资金
double Init_Funds = 100.0;
// 实时资金
double Funds = 0.0;
// 实时资产
double Assets = 0.0;
// 费率乘数
double Fee = 0.9985;
// 上一次购买资金
double Funds_Buy = 0.0;
// 历史最大资金
double Funds_Max = 0.0;
// 盈利交易计数
int Win_Count = 0;
// 亏损交易计数
int Loss_Count = 0;

void strategy(int cur);
void finder();
void set_valuation(int cur, double price);
void save_valuation();
void reset_backing_test();

#pragma region 基础函数
/**
 * @brief
 * 初始化OHLCV数据的存储空间
 */
void init_hist() {
  Time = malloc(sizeof(unsigned long) * Hist_Len);
  Open = malloc(sizeof(double) * Hist_Len);
  High = malloc(sizeof(double) * Hist_Len);
  Low = malloc(sizeof(double) * Hist_Len);
  Close = malloc(sizeof(double) * Hist_Len);
  Volume = malloc(sizeof(double) * Hist_Len);
}

/**
 * @brief
 * 初始化指标数据的存储空间
 */
void init_indexs() {
  for (int i = 0; i < Indexs_Size; ++i) {
    Indexs[i] = malloc(sizeof(double) * Hist_Len);
  }
}

/**
 * @brief
 * 初始化
 * @param hist_len OHLCV数据长度
 * @param indexs_size 指标数据个数
 */
void init(int hist_len, int indexs_size) {
  Hist_Len = hist_len;
  Indexs_Size = indexs_size;
  Valuation_Index = Indexs_Size - 1;
  init_hist();
  init_indexs();
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
    "T: %lu O: %lf H: %lf L: %lf C: %lf V: %lf\n",
    Time[index],
    Open[index],
    High[index],
    Low[index],
    Close[index],
    Volume[index]
  );
}
#pragma endregion

#pragma region 交易函数
/**
 * @brief
 * 现货购买
 * @param price 购买价格
 * @return int 成功：0，失败：1
 */
int buy(double price) {
  if (Assets == 0) {
    Assets = Funds / price * Fee;
    Funds_Buy = Funds;
    Funds = 0;
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
  if (Funds == 0) {
    Funds = Assets * price * Fee;
    if (Funds >= Funds_Buy) {
      Win_Count++;
    } else {
      Loss_Count++;
    }
    Assets = 0;
    return 0;
  }
  return 1;
}

/**
 * @brief
 * 回测
 */
void backing_test(int valuation) {
  reset_backing_test();
  for (int cur = 0; cur < Hist_Len; ++cur) {
    if (cur == Hist_Len - 1) {
      sell(Close[cur]);
      break;
    }
    if (cur >= Stable_Point) {
      strategy(cur);
    }
    if (valuation) {
      set_valuation(cur, Close[cur]);
    }
  }
}

/**
 * @brief
 * 查找
 */
void find() {
  printf("Finder开始...\n");
  time_t op = time(NULL);
  finder();
  printf("Finder完成 秒数 %ld\n", time(NULL) - op);
}
#pragma endregion
