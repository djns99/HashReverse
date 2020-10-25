#!/usr/bin/python3
import sys

res = 0
file = sys.stdin.buffer.read()
for byte in file:
    res ^= byte
res ^= res >> 4
res ^= res >> 2
res ^= res >> 1
print(res & 1)
