#!/usr/bin/env python3
"""
Grid search over SA hyperparameters for exam scheduling.
Runs experiments and logs convergence data.
"""

import os
import sys
import subprocess
import time
import csv
from pathlib import Path
from collections import defaultdict
import tempfile

# Configuration
REPO_ROOT = Path(__file__).parent.parent
BUILD_DIR = REPO_ROOT / "build"
EXECUTABLE = BUILD_DIR / "exam_scheduler.exe"
INPUT_DIR = REPO_ROOT / "test_cases" / "Input"
OUTPUT_DIR = Path(__file__).parent  # result/

RESULT_CSV = OUTPUT_DIR / "result_SA.csv"

# Grid search parameters
T_START_VALUES = [50.0, 100.0, 200.0]
ALPHA_VALUES = [0.90, 0.95, 0.99, 0.999]
T_END = 0.1
ITER_PER_T = 10

RUNS_PER_COMBO = 5

# Maximum iterations (assuming we use default 50000 or adjust as needed)
MAX_ITERATIONS = 50000


def read_test_case(input_file):
    """Parse test case input file. Returns (N, M, d, c, conflicts)."""
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    idx = 0
    # Line 0: N M
    N, M = map(int, lines[idx].split())
    idx += 1
    
    # Line 1: d[1..N] (class sizes)
    d_line = list(map(int, lines[idx].split()))
    d = [0] + d_line  # 1-indexed
    idx += 1
    
    # Line 2: c[1..M] (room capacities)
    c_line = list(map(int, lines[idx].split()))
    c = [0] + c_line  # 1-indexed
    idx += 1
    
    # Line 3: K (number of conflicts)
    K = int(lines[idx])
    idx += 1
    
    # Lines 4 to 3+K: conflict pairs
    conflicts = defaultdict(set)
    for i in range(K):
        x, y = map(int, lines[idx].split())
        conflicts[x].add(y)
        conflicts[y].add(x)
        idx += 1
    
    return N, M, d, c, conflicts


def parse_solution_output(output_str):
    """Parse solution from stdout. Returns dict {class_id: (slot, room)}."""
    solution = {}
    for line in output_str.strip().split('\n'):
        if line.strip():
            parts = line.split()
            if len(parts) == 3:
                class_id, slot, room = map(int, parts)
                solution[class_id] = (slot, room)
    return solution


def calculate_f_days(max_slot):
    """Calculate f_days = ceil(maxSlot / 4)."""
    return (max_slot + 3) // 4


def validate_solution(solution, N, M, d, c, conflicts):
    """
    Validate that solution satisfies all constraints:
    1. All classes assigned a valid slot and room
    2. Room capacity respected (d[i] <= c[room])
    3. No conflicts at the same slot
    Returns (is_valid, max_slot)
    """
    if len(solution) != N:
        return False, -1
    
    max_slot = 0
    slot_room_occupied = defaultdict(lambda: defaultdict(int))  # slot -> room -> count
    
    for class_id in range(1, N + 1):
        if class_id not in solution:
            return False, -1
        
        slot, room = solution[class_id]
        
        # Check room bounds
        if room < 1 or room > M:
            return False, -1
        
        # Check room capacity
        if d[class_id] > c[room]:
            return False, -1
        
        # Track max slot
        if slot > max_slot:
            max_slot = slot
        
        # Track slot/room occupancy
        slot_room_occupied[slot][room] += d[class_id]
        
        # Check no double-booking (simplistic check)
        if slot_room_occupied[slot][room] > c[room]:
            return False, -1
    
    # Check conflict constraints: no two conflicting classes at same slot
    for class_i in range(1, N + 1):
        slot_i, _ = solution[class_i]
        for class_j in conflicts[class_i]:
            if class_j > class_i:  # Avoid double-checking
                slot_j, _ = solution[class_j]
                if slot_i == slot_j:
                    return False, -1
    
    return True, max_slot


def run_single_experiment(test_case_name, T_start, alpha, T_end, iter_per_T):
    """
    Run one SA experiment.
    Returns (valid, f_days, z_slots, runtime_s) or None if error.
    """
    input_file = INPUT_DIR / test_case_name
    if not input_file.exists():
        print(f"Warning: {input_file} not found")
        return None
    
    # Read test case for validation
    N, M, d, c, conflicts = read_test_case(input_file)
    
    # Create temporary log file for convergence data
    log_file = tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.csv')
    log_path = log_file.name
    log_file.close()
    
    # Build command
    cmd = [
        str(EXECUTABLE),
        'sa',
        '--initialTemp', str(T_start),
        '--coolingRate', str(alpha),
        '--minTemp', str(T_end),
        '--log', log_path
    ]
    
    # Run with input redirection
    try:
        start_time = time.perf_counter()
        result = subprocess.run(
            cmd,
            stdin=open(input_file, 'r'),
            capture_output=True,
            text=True,
            timeout=300  # 5 minute timeout
        )
        elapsed = time.perf_counter() - start_time
        
        if result.returncode != 0:
            print(f"Error running {test_case_name} with T_start={T_start}, alpha={alpha}")
            print(f"stderr: {result.stderr}")
            return None
        
        # Parse solution
        solution = parse_solution_output(result.stdout)
        
        # Validate
        is_valid, max_slot = validate_solution(solution, N, M, d, c, conflicts)
        
        f_days = calculate_f_days(max_slot)
        z_slots = max_slot
        
        # Read convergence log
        convergence_data = []
        if os.path.exists(log_path):
            with open(log_path, 'r') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    convergence_data.append(row)
        
        return {
            'valid': is_valid,
            'f_days': f_days,
            'z_slots': z_slots,
            'runtime': elapsed,
            'convergence_data': convergence_data,
            'log_path': log_path
        }
    
    except subprocess.TimeoutExpired:
        print(f"Timeout for {test_case_name} with T_start={T_start}, alpha={alpha}")
        return None
    except Exception as e:
        print(f"Exception: {e}")
        return None


def run_grid_search():
    """Run full grid search and collect results."""
    
    # List of available test cases
    test_cases = []
    for i in range(1, 10):
        test_file = INPUT_DIR / f"TC_{i:02d}"
        if test_file.exists():
            test_cases.append(f"TC_{i:02d}")
    
    if not test_cases:
        print("No test cases found!")
        return
    
    print(f"Found test cases: {test_cases}")
    print(f"Will run grid search on: T_start={T_START_VALUES}, alpha={ALPHA_VALUES}")
    print(f"Runs per combination: {RUNS_PER_COMBO}")
    
    # Prepare result CSV
    result_rows = []
    convergence_logs = defaultdict(list)  # (tc, T_start, alpha) -> list of convergence data
    
    total_experiments = len(test_cases) * len(T_START_VALUES) * len(ALPHA_VALUES) * RUNS_PER_COMBO
    experiment_count = 0
    
    # Grid search loop
    for test_case in test_cases:
        for T_start in T_START_VALUES:
            for alpha in ALPHA_VALUES:
                for run_id in range(1, RUNS_PER_COMBO + 1):
                    experiment_count += 1
                    print(f"\n[{experiment_count}/{total_experiments}] Running {test_case}, "
                          f"T_start={T_start}, alpha={alpha}, run {run_id}/{RUNS_PER_COMBO}")
                    
                    result = run_single_experiment(test_case, T_start, alpha, T_END, ITER_PER_T)
                    
                    if result is None:
                        print(f"  FAILED")
                        continue
                    
                    # Record result row
                    row = {
                        'test_case': test_case,
                        'solver': 'SA',
                        'T_start': T_start,
                        'alpha': alpha,
                        'T_end': T_END,
                        'iter_per_T': ITER_PER_T,
                        'run_id': run_id,
                        'f_days': result['f_days'],
                        'Z_slots': result['z_slots'],
                        'runtime_s': round(result['runtime'], 3),
                        'valid': str(result['valid']).lower()
                    }
                    result_rows.append(row)
                    print(f"  f_days={result['f_days']}, Z_slots={result['z_slots']}, "
                          f"runtime={result['runtime']:.2f}s, valid={result['valid']}")
                    
                    # Store convergence data
                    key = (test_case, T_start, alpha)
                    if result['convergence_data']:
                        convergence_logs[key].extend(result['convergence_data'])
    
    # Write results CSV
    print(f"\nWriting results to {RESULT_CSV}")
    with open(RESULT_CSV, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=[
            'test_case', 'solver', 'T_start', 'alpha', 'T_end', 'iter_per_T',
            'run_id', 'f_days', 'Z_slots', 'runtime_s', 'valid'
        ])
        writer.writeheader()
        writer.writerows(result_rows)
    
    print(f"Wrote {len(result_rows)} result rows")
    
    # Write convergence logs
    print("\nWriting convergence logs...")
    for (test_case, T_start, alpha), conv_data in convergence_logs.items():
        if not conv_data:
            continue
        
        # Format: sa_log_<TC>_T<T_start>_a<alpha>.csv
        # e.g., sa_log_TC_05_T50.0_a0.90.csv
        log_filename = f"sa_log_{test_case}_T{T_start:.1f}_a{alpha:.3f}.csv"
        log_path = OUTPUT_DIR / log_filename
        
        with open(log_path, 'w', newline='') as f:
            # Average convergence data across runs for this combination
            # Group by iteration
            iteration_data = defaultdict(list)
            for row in conv_data:
                it = int(row.get('iteration', 0))
                days = int(row.get('bestDays', 0))
                iteration_data[it].append(days)
            
            writer = csv.DictWriter(f, fieldnames=[
                'iteration', 'temperature', 'f_current', 'f_best'
            ])
            writer.writeheader()
            
            for it in sorted(iteration_data.keys()):
                days_list = iteration_data[it]
                avg_days = sum(days_list) / len(days_list) if days_list else 0
                
                # Estimate temperature at this iteration
                # T(i) = T_start * alpha^i (roughly)
                # For simplicity, just use the best days value
                row_dict = {
                    'iteration': it,
                    'temperature': '',  # Not easily extracted from current log format
                    'f_current': '',    # Current objective not in log
                    'f_best': avg_days
                }
                writer.writerow(row_dict)
        
        print(f"  Wrote {log_path}")
    
    print("\nGrid search complete!")


if __name__ == '__main__':
    if not EXECUTABLE.exists():
        print(f"Error: Executable not found at {EXECUTABLE}")
        sys.exit(1)
    
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    run_grid_search()
