/**
 * @file pioneer.h
 * @author jimao（1982775886@qq.com）
 * @brief 简易高性能量化回测框架
 * @version 0.0.1
 * @date 2022-04-22
 */
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include "indicators.h"

#pragma region 编译时参数
// 最大指标个数
#define INDEXS_SIZE 1024
// 初始资金
#define INIT_FUNDS 100
#pragma endregion

#pragma region 环境参数
// 历史时序数据长度
int Hist_Len = 0;
// 用户初始化指标个数
int Indexs_Size = 0;
// 费率乘数
double Fee = 0.9985;
// 购买列索引
int Buy_Index = -1;
// 出售列索引
int Sell_Index = -1;
// 估值列索引
int Valuation_Index = -1;
// 是否为报告模式
int Report_Mode = 1;
#pragma endregion

#pragma region 历史时序数据
unsigned long * Time;
double * Open;
double * High;
double * Low;
double * Close;
double * Volume;
double * Indexs[INDEXS_SIZE];
// 策略稳定点
int Stable_Point = 0;
#pragma endregion

#pragma region 回测状态
// 实时资金
double Funds = 0.0;
// 实时资产
double Assets = 0.0;
// 上一次购买资金
double Funds_Buy = 0.0;
// 盈利交易计数
int Win_Count = 0;
// 亏损交易计数
int Loss_Count = 0;
#pragma endregion

#pragma region 函数前向声明
void strategy(int cur);
void tester();
void finder();
void set_valuation(int cur, double price);
void reset_backing_test();
void custom_report(FILE * file, int index);
#pragma endregion

#pragma region 基础函数
/**
 * @brief
 * 初始化时序数据的存储空间
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
  for (int i = 0; i < Indexs_Size + 3; ++i) {
    Indexs[i] = malloc(sizeof(double) * Hist_Len);
  }
}

/**
 * @brief
 * 初始化指标数据的值为0
 */
void init_indexs_value() {
  for (int i = 0; i < Indexs_Size + 3; ++i) {
    memset(Indexs[i], 0, sizeof(double) * Hist_Len);
  }
}

/**
 * @brief
 * 初始化
 * @param hist_len 时序数据长度
 * @param indexs_size 指标数据个数
 */
void init(int hist_len, int indexs_size) {
  Hist_Len = hist_len;
  Indexs_Size = indexs_size;
  Buy_Index = Indexs_Size;
  Sell_Index = Indexs_Size + 1;
  Valuation_Index = Indexs_Size + 2;
  init_hist();
  init_indexs();
  init_indexs_value();
}

/**
 * @brief
 * 设置时序数据
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
 * 显示时序数据
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

#pragma region 辅助函数
/**
 * @brief
 * 用于回测状态重置
 */
void reset_backing_test() {
  Funds = INIT_FUNDS;
  Assets = 0.0;
  Funds_Buy = 0.0;
  Win_Count = 0;
  Loss_Count = 0;
}

/**
 * @brief
 * 设置估值
 * @param cur 当前蜡烛索引
 * @param price 当前价格
 */
void set_valuation(int cur, double price) {
  Indexs[Valuation_Index][cur] = Assets * price * Fee + Funds;
}

/**
 * @brief
 * 存储报告
 */
void save_report(void (* custom_report)(FILE * file, int index)) {
  printf("存储报告...\n");
  FILE * file = fopen("report.json", "w");
  fprintf(file, "[\n");
  for (int i = 0; i < Hist_Len; ++i) {
    fprintf(file, "  { ");
    fprintf(file, "\"time\": %ld, ", Time[i]);
    if (Indexs[Valuation_Index][i] > 0) {
      fprintf(file, "\"valuation\": %lf, ", Indexs[Valuation_Index][i]);
    }
    fprintf(file, "\"open\": %lf, ", Open[i]);
    fprintf(file, "\"high\": %lf, ", High[i]);
    fprintf(file, "\"low\": %lf, ", Low[i]);
    fprintf(file, "\"close\": %lf, ", Close[i]);
    if (custom_report != NULL) {
      custom_report(file, i);
    }
    if (Indexs[Buy_Index][i] > 0) {
      fprintf(file, "\"buy\": %lf, ", Indexs[Buy_Index][i]);
    }
    if (Indexs[Sell_Index][i] > 0) {
      fprintf(file, "\"sell\": %lf, ", Indexs[Sell_Index][i]);
    }
    fprintf(file, "\"volume\": %lf ", Volume[i]);
    fprintf(file, "}%s\n", i < Hist_Len - 1 ? "," : "");
  }
  fprintf(file, "]\n");
  fclose(file);
}

/**
 * @brief
 * 输出交易状态信息
 */
void print_state() {
  const double return_funds = Funds - INIT_FUNDS;
  const int total_count = Win_Count + Loss_Count;
  const double win_rate = 100.0 * Win_Count / total_count;
  printf(
    "$ 回报: %lf  交易次数: %d[%d:%d]  成功率: %.4lf%%\n",
    return_funds,
    total_count,
    Win_Count,
    Loss_Count,
    win_rate
  );
}

/**
 * @brief
 * 根据资金曲线计算夏普率
 * @param size 计算尺度
 */
void sharpe_index(int size) {
  for (int i = size - 1; i < Hist_Len; ++i) {

  }
}
#pragma endregion

#pragma region 交易函数
/**
 * @brief
 * 现货购买
 * @param price 购买价格
 * @return int 成功：0，失败：1
 */
int buy(double price, int cur) {
  if (Assets == 0) {
    Assets = Funds / price * Fee;
    Funds_Buy = Funds;
    Funds = 0;
    if (Report_Mode) {
      Indexs[Buy_Index][cur] = price;
    }
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
int sell(double price, int cur) {
  if (Funds == 0) {
    Funds = Assets * price * Fee;
    if (Funds >= Funds_Buy) {
      Win_Count++;
    } else {
      Loss_Count++;
    }
    Assets = 0;
    if (Report_Mode) {
      Indexs[Sell_Index][cur] = price;
    }
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
  for (int cur = 0; cur < Hist_Len; ++cur) {
    if (cur == Hist_Len - 1) {
      sell(Close[cur], cur);
      break;
    }
    if (cur >= Stable_Point) {
      strategy(cur);
    }
    if (Report_Mode) {
      set_valuation(cur, Close[cur]);
    }
  }
}

/**
 * @brief
 * 测试
 */
void test() {
  printf("回测开始...\n");
  Report_Mode = 1;
  init_indexs_value();
  tester();
  backing_test();
  printf("回测结束\n");
  save_report(custom_report);
  print_state();
}

/**
 * @brief
 * 查找
 */
void find() {
  Report_Mode = 0;
  printf("穷举器开始...\n");
  time_t op = time(NULL);
  finder();
  printf("穷举器完成 秒数 %ld\n", time(NULL) - op);
}
#pragma endregion
