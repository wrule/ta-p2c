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
double x_queue_high(int size) {
  int start = x_queue_end - size;
  if (start < 0) {
    start = 0;
  }
  double max = DBL_MIN;
  double max_atr = 0;
  for (int i = start; i < x_queue_end; ++i) {
    int index = i % X_QUEUE_SIZE;
    if (x_queue[index][1] > max) {
      max = x_queue[index][1];
      max_atr = x_queue[index][2];
    }
  }
  return max + max_atr * 0.5;
}
#pragma endregion

/**
 * @brief
 * 用于回测的状态重置
 */
void reset_backing_test() {
  Funds = Init_Funds;
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
  Indexs[Valuation_Index][cur] = Assets * price + Funds;
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
  ti_macd(Hist_Len, macd_inputs, macd_options, macd_outputs);

  Stable_Point = macd_start + 1;

  // ATR指标生成
  const double atr_options[] = { 5 };
  const double * atr_inputs[] = { High, Low, Close };
  const int atr_start = ti_atr_start(atr_options);
  double * atr_outputs[] = { &Indexs[4][atr_start] };
  ti_atr(Hist_Len, atr_inputs, atr_options, atr_outputs);

  if (Stable_Point < atr_start) {
    Stable_Point = atr_start;
  }

  // 离场指标生成
  for (int i = 0; i < k_num; ++i) {
    Indexs[3][i] = -1.0;
  }
  for (int i = k_num; i < Hist_Len; ++i) {
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

  if (Stable_Point < k_num) {
    Stable_Point = k_num;
  }

  // printf("%d\n", Stable_Point);
  // for (int i = 0; i < 100; ++i) {
  //   printf("%d %lf %lf %lf\n", i, Indexs[2][i], Indexs[4][i], Indexs[3][i]);
  // }
}

// 策略
void strategy(int cur) {
  // 记录金叉死叉
  if (
    (Indexs[2][cur] > 0 && Indexs[2][cur - 1] <= 0) ||
    (Indexs[2][cur] < 0 && Indexs[2][cur - 1] >= 0)
  ) {
    x_queue_push(cur, High[cur], Indexs[4][cur]);
  }
  // 入场
  const double high = x_queue_high(3);
  if (High[cur] > high) {
    if (Open[cur] > high) {
      buy(Open[cur]);
    } else {
      buy(high);
    }
    return;
  }
  // 离场
  if (
    Indexs[3][cur] > 0
  ) {
    sell(Indexs[3][cur]);
    return;
  }
}
// 查找器
void finder() {
  double max_rate = 0.0;
  for (int fast = 8; fast < 100; ++fast) {
    printf("# %d...\n", fast);
    for (int slow = fast + 1; slow < 100; ++slow) {
      for (int size = 2; size < 100; ++size) {
        for (int k_num = 2; k_num < 100; ++k_num) {
          indicators(fast, slow, size, k_num);
          backing_test(0);
          const double cur_rate = 100.0 * Win_Count / (Win_Count + Loss_Count);
          if (cur_rate > max_rate) {
            max_rate = cur_rate;
            printf(
              "$ %lf [%d %d %d %d] {%d %d:%d %lf}\n",
              Funds,
              fast, slow, size, k_num,
              Win_Count + Loss_Count, Win_Count, Loss_Count, 100.0 * Win_Count / (Win_Count + Loss_Count)
            );
          }
        }
      }
    }
  }
}

void test() {
  const int fast = 5;
  const int slow = 10;
  const int size = 25;
  const int k_num = 17;
  indicators(fast, slow, size, k_num);
  backing_test(1);
  printf(
    "$ %lf [%d %d %d %d] {%d %d:%d %lf}\n",
    Funds,
    fast, slow, size, k_num,
    Win_Count + Loss_Count, Win_Count, Loss_Count, 100.0 * Win_Count / (Win_Count + Loss_Count)
  );
  save_valuation();
}

void save_valuation() {
  printf("存储估值曲线数据...\n");
  FILE * file = fopen("valuation.json", "w");
  fprintf(file, "[\n");
  for (int i = 0; i < Hist_Len; ++i) {
    fprintf(
      file,
      "  { \"type\": \"valuation\", \"x\": %lu, \"y\": %lf }%s\n",
      Time[i],
      Indexs[Valuation_Index][i],
      i < Hist_Len - 1 ? "," : ""
    );
  }
  fprintf(file, "]\n");
  fclose(file);
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
