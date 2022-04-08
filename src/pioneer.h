#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "/usr/local/include/ta-lib/ta_libc.h"
#include "indicators.h"

#define INDEXS_SIZE 1024

int Hist_Len = 0;
int Indexs_Size = 0;
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
void set_valuation(int cur, double price, int index);
void save_valuation();
