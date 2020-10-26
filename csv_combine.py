import sys

if len(sys.argv) == 1:
    print("Gimme file")
    exit(1)

lines = []
for file in sys.argv[1:]:
    with open(file, "r") as f:
        filename = file.split("/")[-1]
        header_line = f.readline().rstrip()
        header_parts = header_line.split(",", 1)
        header = "File Name,Objective Function Calls,Threads,Algorithm Name,Iteration," + header_parts[1]
        file_parts = filename.rsplit("_", 3)
        file_parts[-1] = file_parts[-1][0]
        file_parts[0] = file_parts[0].split(".")[0]
        line = f.readline().rstrip()
        while line:
            line_parts = line.split(",", 1)
            lines.append(",".join(file_parts[:-1]) + "," + line_parts[0] + "," + file_parts[-1] + "," + line_parts[1])
            line = f.readline().rstrip()

# print(header)
# for line in sorted(lines):
#     print(line)
#
# exit(0)

header_parts = header.split(",")
sorted_lines = lines
sorted_lines.sort()
results = []
full_header_parts = []
for i in range(0, len(sorted_lines), 3):
    sums = [0 for x in header_parts]
    mins = [float('inf') for x in header_parts]
    maxs = [0.0 for x in header_parts]
    failed = [False for x in header_parts]
    for j in range(3):
        line = sorted_lines[i + j]
        for k, part in enumerate(line.split(",")):
            try:
                sums[k] += float(part)
                mins[k] = min(mins[k], float(part))
                maxs[k] = max(maxs[k], float(part))
                if j == 2:
                    sums[k] /= 3
                    sums[k] = str(sums[k])
                    mins[k] = str(mins[k])
                    maxs[k] = str(maxs[k])
            except Exception as e:
                failed[k] = True
                sums[k] = part
    full_header_parts = []
    line = []
    for j in range(len(header_parts)):
        if failed[j]:
            full_header_parts.append(header_parts[j])
            line.append(sums[j])
        else:
            line.append(mins[j])
            line.append(sums[j])
            line.append(maxs[j])
            full_header_parts.append(header_parts[j] + " Min")
            full_header_parts.append(header_parts[j] + " Mean")
            full_header_parts.append(header_parts[j] + " Max")
    results.append(line)

print(",".join(full_header_parts))
for line in results:
    print(",".join(line))
