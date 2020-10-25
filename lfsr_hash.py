#!/usr/bin/python3
import numpy as np
from pylfsr import LFSR
import sys

input_data = sys.stdin.buffer.read()
bits = len(input_data) * 8

cycle_len = bits
if len(sys.argv) == 2:
    cycle_len = int(sys.argv[1])

init_state = []
for byte in input_data:
    for i in range(8):
        init_state.append((byte >> i) & 1)

poly = LFSR().get_fpolyList(m=bits)[0]
L = LFSR(fpoly=poly, initstate=np.array(init_state))
L.runKCycle(cycle_len)

print(L.state[0])