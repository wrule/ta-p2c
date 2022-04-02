#!/opt/homebrew/bin/python3
from ctypes import *

print('你好，世界')
hello = CDLL('hello.so')
print(hello.find())
