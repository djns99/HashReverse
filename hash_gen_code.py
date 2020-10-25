import math
import random
import subprocess
import sys
import binascii

import numpy as np
from pylfsr import LFSR

if len(sys.argv) != 3 or "help" in sys.argv:
    print("Usage: ", sys.argv[0], "<input bits> <output bits>")
    exit(1)

in_bits = int(sys.argv[1])
out_bits = int(sys.argv[2])
if in_bits % 8 != 0:
    print("Non-byte aligned input not supported")
    exit(1)
out_nibbles = math.ceil(out_bits / 4)
num_words = 2 ** in_bits
randomise = False
if in_bits > 16:
    num_words = 2 ** 20
    randomise = True

print(str(in_bits) + "," + str(out_bits) + "," + str(num_words))

def xor_num(num):
    assert num < 2 ** 256
    i = 128
    while i > 0:
        num ^= num >> i
        i = i // 2
    return num & 1


def run_lfsr(lfsr, num):
    init_state = []
    for bit in range(lfsr.fpoly[0]):
        init_state.append((num >> bit) & 1)
    lfsr.set(fpoly=lfsr.fpoly, state=np.array(init_state))
    for i in range(lfsr.fpoly[0]):
        lfsr.next()
    return (lfsr.state[0] & 1)

def crc(num):
    global in_bits
    return binascii.crc32(num.to_bytes(in_bits // 8, 'little')) & 1

poly_list = [[64, 63, 61, 60]]
if in_bits != 64:
    poly_list = LFSR().get_fpolyList(m=in_bits)

lfsr = LFSR(fpoly=poly_list[0])
for i in range(num_words):
    val = random.randint(0, 2 ** in_bits - 1) if randomise else i
    # print(str(val) + "," + str(run_lfsr(lfsr, val)))
    # print(str(val) + "," + str(xor_num(val)))
    print(str(val) + "," + str(crc(val)))
