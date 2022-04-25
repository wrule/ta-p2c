#include <math.h>
#include "pioneer.h"

#pragma region 高性能队列
#define X_QUEUE_SIZE 1024
#define X_QUEUE_ITEM_SIZE 3
double x_queue[X_QUEUE_SIZE][X_QUEUE_ITEM_SIZE] = { };
int x_queue_end = 0;
void x_queue_push(double bar_index, double high, double atr) {
  int index = x_queue_end % X_QUEUE_SIZE;
  x_queue[index][0] = bar_index;
  x_queue[index][1] = high;
  x_queue[index][2] = atr;
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
      "I:%lf H:%lf A:%lf\n",
      x_queue[index][0],
      x_queue[index][1],
      x_queue[index][2]
    );
  }
}
double x_queue_high(int cur, int x_num, int bar_num) {
  double max = DBL_MIN;
  double max_atr = 0.0;
  int i = 0;
  for (; i < x_num; ++i) {
    const int x_index = (x_queue_end - 1 - i) % X_QUEUE_SIZE;
    if (x_index < 0) {
      break;
    }
    if (cur - x_queue[x_index][0] > bar_num) {
      break;
    }
    if (x_queue[x_index][1] > max) {
      max = x_queue[x_index][1];
      max_atr = x_queue[x_index][2];
    }
  }
  if (i >= x_num) {
    return max + max_atr * 0.5;
  }
  return DBL_MAX;
}
#pragma endregion

#define FAST_LINE 0
#define SLOW_LINE 1
#define MACD_LINE 2
#define ATR_LINE 3
#define LEAVE_LINE 4

int Queue_Size = 0;
int Bar_Max = 0;

// 填充指标
void indicators(
  int fast,
  int slow,
  int size,
  int atr,
  int k_num
) {
  // MACD指标生成
  const double macd_options[] = { fast, slow, size };
  const double * macd_inputs[] = { Close };
  const int macd_start = ti_macd_start(macd_options);
  double * macd_outputs[] = {
    &Indexs[FAST_LINE][macd_start],
    &Indexs[SLOW_LINE][macd_start],
    &Indexs[MACD_LINE][macd_start]
  };
  ti_macd(Hist_Len, macd_inputs, macd_options, macd_outputs);
  Stable_Point = macd_start + 1;

  // ATR指标生成
  const double atr_options[] = { atr };
  const double * atr_inputs[] = { High, Low, Close };
  const int atr_start = ti_atr_start(atr_options);
  double * atr_outputs[] = { &Indexs[ATR_LINE][atr_start] };
  ti_atr(Hist_Len, atr_inputs, atr_options, atr_outputs);
  if (Stable_Point < atr_start) {
    Stable_Point = atr_start;
  }

  // 离场指标生成
  for (int i = k_num; i < Hist_Len; ++i) {
    double min = DBL_MAX;
    for (int h = i - k_num; h < i; ++h) {
      if (Low[h] < min) {
        min = Low[h];
      }
    }
    if (Low[i] < min) {
      if (Open[i] < min) {
        Indexs[LEAVE_LINE][i] = Open[i];
      } else {
        Indexs[LEAVE_LINE][i] = min;
      }
    } else {
      Indexs[LEAVE_LINE][i] = -1.0;
    }
  }
  if (Stable_Point < k_num) {
    Stable_Point = k_num;
  }
}

// 策略
void strategy(int cur) {
  // 记录金叉死叉
  if (
    (Indexs[MACD_LINE][cur] > 0 && Indexs[MACD_LINE][cur - 1] <= 0) ||
    (Indexs[MACD_LINE][cur] < 0 && Indexs[MACD_LINE][cur - 1] >= 0)
  ) {
    x_queue_push(cur, High[cur], Indexs[ATR_LINE][cur]);
  }
  // 入场
  const double high = x_queue_high(cur, Queue_Size, Bar_Max);
  if (High[cur] > high) {
    if (Open[cur] > high) {
      buy(Open[cur], cur);
    } else {
      buy(high, cur);
    }
    return;
  }
  // 离场
  if (
    Indexs[LEAVE_LINE][cur] > 0
  ) {
    sell(Indexs[LEAVE_LINE][cur], cur);
    return;
  }
}

// 自定义报告输出
void custom_report(FILE * file, int index) {
  fprintf(file, "\"FAST\": %lf, ", Indexs[FAST_LINE][index]);
  fprintf(file, "\"SLOW\": %lf, ", Indexs[SLOW_LINE][index]);
  fprintf(file, "\"MACD\": %lf, ", Indexs[MACD_LINE][index]);
  fprintf(file, "\"ATR\": %lf, ", Indexs[ATR_LINE][index]);
  fprintf(file, "\"LEAVE\": %lf, ", Indexs[LEAVE_LINE][index]);
}

// 测试器
void tester() {
  const int fast = 5, slow = 10, size = 25, atr = 4, k_num = 17;
  Queue_Size = 3;
  Bar_Max = 39;
  indicators(fast, slow, size, atr, k_num);
}

// 查找器
void finder() {
  double funds_max = DBL_MIN;
  for (int fast = 2; fast < 50; ++fast) {
    for (int slow = fast + 1; slow < 150; ++slow) {
      for (int size = 2; size < 200; ++size) {
        for (int atr = 3; atr < 4; ++atr) {
          for (int k_num = 34; k_num < 35; ++k_num) {
            for (int q_size = 2; q_size < 3; ++q_size) {
              Queue_Size = q_size;
              for (int bar_size = 51; bar_size < 52; ++bar_size) {
                Bar_Max = bar_size;
                x_queue_end = 0;
                indicators(fast, slow, size, atr, k_num);
                backing_test();
                if (Funds > funds_max) {
                  funds_max = Funds;
                  printf("%d %d %d\n", fast, slow, size);
                  print_state();
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @brief
 * 计算夏普率
 * @param size 计算尺度
 * @return double 夏普率
 */
double sharpe_ratio(int size) {
  const int usable_size = Hist_Len - Stable_Point;
  const int stage_num = (usable_size - size) / (size - 1) + 1;
  const int extra = (usable_size - size) % (size - 1);
  const int start_index = Stable_Point + extra;
  double return_rate = Funds / Indexs[Valuation_Index][start_index];
  // 阶段化盈利率
  const double stage_profit = pow(return_rate, 1.0 / stage_num) - 1;

  double * stage_profit_list = malloc(sizeof(double) * stage_num);
  for (int i = 0; i < stage_num; ++i) {
    const int op_index = start_index + i * (size - 1);
    const int ed_index = op_index + size - 1;
    stage_profit_list[i] = Indexs[Valuation_Index][ed_index] / Indexs[Valuation_Index][op_index] - 1;
  }

  double stage_profit_sum = 0.0;
  for (int i = 0; i < stage_num; ++i) {
    stage_profit_sum += stage_profit_list[i];
  }
  double stage_profit_avg = stage_profit_sum / stage_num;

  double variance_sum = 0.0;
  for (int i = 0; i < stage_num; ++i) {
    const double diff = stage_profit_list[i] - stage_profit_avg;
    variance_sum += diff * diff;
  }
  double variance = variance_sum / stage_num;

  double std = sqrt(variance);

  return stage_profit / std;
}

// 主函数
int main() {
  test();
  printf("夏普率: %lf\n", sharpe_ratio(7 * 24));
  return 0;
}
