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
  int i = 0;
  double max = DBL_MIN;
  double max_atr = 0.0;
  for (; i < x_num; ++i) {
    const int x_index = (x_queue_end - 1 - i) % X_QUEUE_SIZE;;
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
  if (i == x_num) {
    return max + max_atr * 0.5;
  }
  return DBL_MAX;
  // int start = x_queue_end - x_num;
  // if (start < 0) {
  //   start = 0;
  // }
  // double max = DBL_MIN;
  // double max_atr = 0;
  // for (int i = start; i < x_queue_end; ++i) {
  //   int index = i % X_QUEUE_SIZE;
  //   if (x_queue[index][1] > max) {
  //     max = x_queue[index][1];
  //     max_atr = x_queue[index][2];
  //   }
  // }
  // return max + max_atr * 0.5;
}
#pragma endregion

#define FAST_LINE 0
#define SLOW_LINE 1
#define MACD_LINE 2
#define ATR_LINE 3
#define LEAVE_LINE 4

int Queue_Size = 3;

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
  const double high = x_queue_high(cur, Queue_Size, 1e6);
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
    Indexs[LEAVE_LINE][cur] > 0
  ) {
    sell(Indexs[LEAVE_LINE][cur]);
    return;
  }
}

// 测试器
void tester() {
  const int fast = 5, slow = 10, size = 25, k_num = 17;
  indicators(fast, slow, size, 5, k_num);
  backing_test(1);
  save_valuation();
  print_state();
}

// 查找器
void finder() {

}

// 主函数
int main() {
  test();
  return 0;
}
