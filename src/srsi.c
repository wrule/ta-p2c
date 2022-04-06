#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 临时
void strategy(int cur) {
  if (
    Indexs[0][cur] > Indexs[1][cur] &&
    Indexs[0][cur - 1] <= Indexs[1][cur - 1]
  ) {
    buy(Close[cur]);
  }
  if (
    Indexs[0][cur] < Indexs[1][cur] &&
    Indexs[0][cur - 1] >= Indexs[1][cur - 1]
  ) {
    sell(Close[cur]);
  }
}

// 临时
void finder() {
  for (int rsi_length = 8; rsi_length < 200; ++rsi_length) {
    printf("# %d...\n", rsi_length);
    for (int length = 2; length < 200; ++length) {
      for (int k = 2; k < 100; ++k) {
        for (int d = 2; d < 100; ++d) {
          strategy5(rsi_length, length, k, d);
          backing_test();
          if (funds > funds_max) {
            funds_max = funds;
            printf(
              "$ %lf [%d %d %d %d] {%d %d:%d %lf}\n",
              funds_max,
              rsi_length, length, k, d,
              win_count + loss_count, win_count, loss_count, 100.0 * win_count / (win_count + loss_count)
            );
          }
        }
      }
    }
  }
}
