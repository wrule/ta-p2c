#include <math.h>
#include "pioneer.h"

#define RSI_LINE 0
#define K_LINE 1
#define D_LINE 2
#define DIFF_LINE 3
#define ATR_LINE 4

// 指标
void indicators(
  int rsi_length,
  int length,
  int k,
  int d,
  int atr
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

  // ATR指标生成
  const double atr_options[] = { atr };
  const double * atr_inputs[] = { High, Low, Close };
  const int atr_start = ti_atr_start(atr_options);
  double * atr_outputs[] = { &Indexs[ATR_LINE][atr_start] };
  ti_atr(Hist_Len, atr_inputs, atr_options, atr_outputs);
  if (Stable_Point < atr_start) {
    Stable_Point = atr_start;
  }
}


// 策略
void strategy(int cur) {

}

// 自定义报告输出
void custom_report(FILE * file, int index) {

}

// 测试器
void tester() {
  const int rsi_length = 10, length = 50, k = 5, d = 26, atr = 2;
  indicators(rsi_length, length, k, d, atr);
}

// 查找器
void finder() {

}

// 主函数
int main() {
  test();
  return 0;
}
