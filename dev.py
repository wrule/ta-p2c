#!/usr/bin/python3
from ctypes import *
import json
hello = CDLL('/home/gu/github/ta-p2c/hello.so')

hist = json.load(open('src/ETH_USDT-2h.json', 'r'))

# hist = list(filter(lambda item: item[0] >= 1609459200000, all_hist))

print(hist[0][0])

hello.init(len(hist), 3)

for index, item in enumerate(hist):
  hello.fill_ohlcv(
    c_int(index),
    c_ulong(item[0]),
    c_double(item[1]),
    c_double(item[2]),
    c_double(item[3]),
    c_double(item[4]),
    c_double(item[5]),
  );

hello.find()
