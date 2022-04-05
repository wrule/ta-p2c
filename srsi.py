#!/opt/homebrew/bin/python3
import pandas as pd
import pandas_ta as ta

df = pd.read_json("src/BTC_USDT-2h.json")
df.index = pd.to_datetime(df[0], unit = 'ms')
df.index.names = ['time']
df.rename(
  columns = {
    0: 'offset',
    1: 'open',
    2: 'high',
    3: 'low',
    4: 'close',
    5: 'volume',
  },
  inplace = True,
)
df['offset'] = range(len(df.index))
df.ta.stochrsi(
  length = 49,
  rsi_length = 8,
  k = 8,
  d = 27,
  append = True,
)
df = df[['offset', 'close', 'STOCHRSIk_49_8_8_27', 'STOCHRSId_49_8_8_27']]
print(df.head(20))
