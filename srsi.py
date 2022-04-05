#!/opt/homebrew/bin/python3
import pandas as pd
import pandas_ta as ta

df = pd.read_json("src/BTC_USDT-2h.json")
df.rename(
  columns = {
    0: 'time',
    1: 'open',
    2: 'high',
    3: 'low',
    4: 'close',
    5: 'volume',
  },
  inplace = True,
)
df.index = pd.to_datetime(df['time'], unit = 'ms')
print(df)
