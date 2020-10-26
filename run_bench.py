import subprocess
import sys

if len(sys.argv) != 2:
    print("Oops")
    exit(1)

quick_benchmarks = [
    ("random_8.csv", "random_8.csv", 8),
    ("random_16.csv", "random_16.csv", 8),
    ("xor_8.csv", "xor_8.csv", 8),
    ("xor_16.csv", "xor_16.csv", 32),
    ("xor_64.csv", "xor_64_test.csv", 128),
    ("crc_64.csv", "crc_64_test.csv", 128),
    ("lfsr_64.csv", "lfsr_64_test.csv", 128),
    ("sha2_16.csv", "sha2_16.csv", 256),
]

full_benchmarks = [
    ("random_64_24.csv", "random_64_24_test.csv", 32),
]

pop_sizes = [10, 100, 10000]
local_search = 0
threads = [2, 6, 12]
partial_samples = 1024
evals = 1000000
runs = 3

out_dir = sys.argv[1]
command = "./cmake-build-release/bench"

pop_size = 10000
thread = 12
for quick_bench in quick_benchmarks:
    for i in range(runs):
        with open(out_dir + "/{}_{}_{}_{}.csv".format(quick_bench[0], pop_size, thread, i), "w+") as f:
            subprocess.run([command, quick_bench[0], quick_bench[1], str(thread), str(quick_bench[2]),
                            str(partial_samples), str(evals), str(pop_size), str(local_search), str(i)], stdout=f)

for full_bench in full_benchmarks:
    for pop_size in pop_sizes:
        for thread in threads:
            for i in range(runs):
                with open(out_dir + "/{}_{}_{}_{}.csv".format(full_bench[0], pop_size, thread, i), "w+") as f:
                    subprocess.run([command, full_bench[0], full_bench[1], str(thread), str(full_bench[2]),
                                    str(partial_samples), str(evals), str(pop_size), str(local_search), str(i)], stdout=f)