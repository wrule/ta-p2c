#!/opt/homebrew/bin/python3
from ctypes import *
import json
hello = CDLL('./pioneer.so')

def main():
  print('数据载入中...')
  hist = json.load(open('src/BTC_USDT-2h.json', 'r'))
  print('初始化环境...')
  hello.init(len(hist), 32)
  print('填充量价数据...')
  for index, item in enumerate(hist):
    hello.set_ohlcv(
      c_int(index),
      c_ulong(item[0]),
      c_double(item[1]),
      c_double(item[2]),
      c_double(item[3]),
      c_double(item[4]),
      c_double(item[5]),
    );
  # hello.find()
  hello.test()

main()
