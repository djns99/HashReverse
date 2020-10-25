import random
import subprocess
import sys

if len(sys.argv) != 4 or "help" in sys.argv:
    print("Usage: ", sys.argv[0], "<command> <input bits> <output bits>")
    exit(1)

in_bits = int(sys.argv[2])
out_bits = int(sys.argv[3])
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
for i in range(num_words):
    command = sys.argv[1]
    val = random.randint(0, 2 ** in_bits - 1) if randomise else i
    res = subprocess.run(command, shell=True, input=val.to_bytes(in_bits // 8, 'little'), stdout=subprocess.PIPE)
    hex_hash = res.stdout.decode("utf8")[:out_nibbles]
    upper_mask = (2 ** (out_bits % 4)) - 1
    if upper_mask == 0:
        upper_mask = 0xf
    adjusted_last = hex(int(hex_hash[-1], 16) & upper_mask)[2:]
    hex_hash = hex_hash[:-1] + adjusted_last

    print(str(val) + "," + hex_hash)
