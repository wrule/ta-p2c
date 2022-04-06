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
