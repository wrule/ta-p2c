#include <math.h>
#include "pioneer.h"

#define RSI_LINE 0
#define K_LINE 1
#define D_LINE 2
#define DIFF_LINE 3

// 指标
void indicators(
  int rsi_length,
  int length,
  int k,
  int d
) {
  // RSI指标计算
  const double rsi_options[] = { rsi_length };
  const double * rsi_inputs[] = { Close };
  const int rsi_start = ti_rsi_start(rsi_options);
  double * rsi_outputs[] = { &Indexs[RSI_LINE][rsi_start] };
  ti_rsi(Hist_Len, rsi_inputs, rsi_options, rsi_outputs);

  // STOCH指标计算
  const double stoch_options[] = { length, k, d };
  const double * stoch_inputs[] = {
    &Indexs[RSI_LINE][rsi_start],
    &Indexs[RSI_LINE][rsi_start],
    &Indexs[RSI_LINE][rsi_start]
  };
  const int stoch_start = ti_stoch_start(stoch_options) + rsi_start;
  double * stoch_outputs[] = { &Indexs[K_LINE][stoch_start], &Indexs[D_LINE][stoch_start] };
  ti_stoch(Hist_Len - rsi_start, stoch_inputs, stoch_options, stoch_outputs);

  // KD差值计算
  for (int i = stoch_start; i < Hist_Len; ++i) {
    Indexs[DIFF_LINE][i] = Indexs[K_LINE][i] - Indexs[D_LINE][i];
  }

  // 设置稳定点
  Stable_Point = stoch_start + 1;
}

int win_count = 0;
int fail_count = 0;

// 策略
void strategy(int cur) {
  if (Assets == 0) {
    if (Indexs[DIFF_LINE][cur] > 0 && Indexs[DIFF_LINE][cur - 1] <= 0) {
      buy(Close[cur], cur);
    }
  } else {
    const double open_ratio = (Assets * Open[cur] - Funds_Buy) / Funds_Buy;
    const double high_ratio = (Assets * High[cur] - Funds_Buy) / Funds_Buy;
    const double low_ratio = (Assets * Low[cur] - Funds_Buy) / Funds_Buy;
    if (open_ratio >= 0.00510) {
      win_count++;
    } else if (open_ratio <= -0.0045) {
      fail_count++;
    } else if (high_ratio >= 0.00510) {
      win_count++;
    } else if (low_ratio <= -0.0045) {
      fail_count++;
    } else {
      return;
    }
    Funds = 100;
    Assets = 0;
    Funds_Buy = 0;
  }
}

// 自定义报告输出
void custom_report(FILE * file, int index) {

}

// 测试器
void tester() {
  const int rsi_length = 27, length = 49, k = 8, d = 8, atr = 2;
  indicators(rsi_length, length, k, d);
}

// 查找器
void finder() {

}

// 主函数
int main() {
  test();
  printf("%d %d 胜率: %lf\n", win_count, fail_count, (double)win_count / (win_count + fail_count));
  return 0;
}
