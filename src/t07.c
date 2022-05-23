#include <math.h>
#include "pioneer.h"

#define FAST_LINE 0
#define SLOW_LINE 1
#define MACD_LINE 2
#define RSI_LINE 3

// 填充指标
void indicators(
  int fast,
  int slow,
  int size,
  int rsi_length
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
  // RSI指标生成
  const double rsi_options[] = { rsi_length };
  const double * rsi_inputs[] = { Close };
  const int rsi_start = ti_rsi_start(rsi_options);
  double * rsi_outputs[] = { &Indexs[RSI_LINE][rsi_start] };
  ti_rsi(Hist_Len, rsi_inputs, rsi_options, rsi_outputs);
  Stable_Point = macd_start > rsi_start ? macd_start : rsi_start;
}

double Prev_Price = 0.0;

// 策略
void strategy(int cur) {
  if (Assets == 0) {
    if (
      Indexs[FAST_LINE][cur] >= 0 &&
      Indexs[SLOW_LINE][cur] >= 0 &&
      Indexs[MACD_LINE][cur] < 0 &&
      Indexs[RSI_LINE][cur] < 10
    ) {
      buy(Close[cur], cur);
      Prev_Price = Close[cur];
    }
  } else {
    const double diff = Close[cur] - Prev_Price;
    const double diff_ratio = diff / Prev_Price;
    // 止盈
    if (diff > 0.02 || Indexs[RSI_LINE][cur] > 80) {
      sell(Close[cur], cur);
    }
    // 止损
    if (diff < 0.005) {
      sell(Close[cur], cur);
    }
  }
}

// 自定义报告输出
void custom_report(FILE * file, int index) {
  fprintf(file, "\"FAST\": %lf, ", Indexs[FAST_LINE][index]);
  fprintf(file, "\"SLOW\": %lf, ", Indexs[SLOW_LINE][index]);
  fprintf(file, "\"MACD\": %lf, ", Indexs[MACD_LINE][index]);
  fprintf(file, "\"RSI\": %lf, ", Indexs[RSI_LINE][index]);
}

// 测试器
void tester() {
  const int fast = 5, slow = 10, size = 25, rsi_length = 21;
  indicators(fast, slow, size, rsi_length);
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
              // Queue_Size = q_size;
              // for (int bar_size = 51; bar_size < 52; ++bar_size) {
              //   Bar_Max = bar_size;
              //   x_queue_end = 0;
              //   indicators(fast, slow, size, atr, k_num);
              //   backing_test();
              //   if (Funds > funds_max) {
              //     funds_max = Funds;
              //     printf("%d %d %d\n", fast, slow, size);
              //     print_state();
              //   }
              // }
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
  // printf("夏普率: %lf\n", sharpe_ratio(7 * 12));
  return 0;
}
