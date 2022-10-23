# !/bin/python3

import sys

def parse_file(quantum, run_count):
    benchmark_result = {}
    
    for rc in range(1,run_count+1):
        with open(f"rr_benchmark/quantum_{quantum}/run_{rc}.txt") as f:
            lines = list(f)
            benchmark_line = 0
            total_lines = len(lines)
            for i,l in enumerate(lines):
                if "=Benchmark=" in l:
                    benchmark_line = i
            ##############################
            for ci in range(benchmark_line + 1, total_lines, 2):
                command = lines[ci][:-1]
                result = lines[ci+1]
                if not command in benchmark_result.keys():
                    benchmark_result[command] = {"response_time": [], "burst_time": [], "turnaround_time": [], "waiting_time": []}
                
                components = list(map(lambda l: l.strip(), result.split(", ")))
                for comp in components:
                    comp_split = list(map(lambda c: c.strip(), comp.split(":")))
                    benchmark_result[command][comp_split[0]].append(int(comp_split[1]))

    return benchmark_result

results = {}
for quantum in range(10, 101,10):
    benchmark_result = parse_file(quantum, 10)
    for command in benchmark_result.keys():
        if not command in results:
            results[command] = {"response_time": {}, "burst_time": {}, "turnaround_time": {}, "waiting_time": {}}
        
        for metric in ["response_time", "burst_time", "turnaround_time", "waiting_time"]:
            results[command][metric][quantum] = benchmark_result[command][metric]
            
print(results[sys.argv[1]][sys.argv[2]])
