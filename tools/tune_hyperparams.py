#!/usr/bin/env python3
"""
Hyperparameter tuning script for SA solver.

Sweeps through different hyperparameter values, runs the solver T times,
and collects statistics (mean, std, avg runtime).

Usage:
    python tune_hyperparams.py --testcase test_cases/Input/TC_02 [--sweep param1 param2 ...]
    
    If no sweep params specified, runs a quick test on one hyperparameter.
"""

import sys
import os
import subprocess
import argparse
import csv
import time
import statistics
from pathlib import Path


# Baseline hyperparameters
BASELINE = {
    "initialTemp": 100.0,
    "minTemp": 0.01,
    "coolingRate": 0.9999,
    "maxIterations": 50000,
    "horizonExtension": 4,
    "swapProbability": 0.3,
    "highSlotBias": 0.5,
}

# Hyperparameter sweep ranges
SWEEP = {
    "coolingRate": [0.999, 0.9995, 0.9999, 0.99995],
    "initialTemp": [10, 50, 100, 500],
    "swapProbability": [0.1, 0.3, 0.5, 0.7],
    "highSlotBias": [0.0, 0.3, 0.5, 0.7],
    "horizonExtension": [2, 4, 8],
}


def run_solver(exe_path, input_file, params, timeout=120):
    """
    Run the solver with given hyperparameters.
    
    Returns (best_days, runtime_seconds) or (None, None) on error.
    best_days is extracted from output as ceil(max_slot / 4).
    """
    # Build command
    cmd = [exe_path, "sa"]
    for param_name, param_value in params.items():
        cmd.append(f"--{param_name}")
        cmd.append(str(param_value))
    
    try:
        start_time = time.time()
        
        # Run solver with input piped from file
        with open(input_file, 'r') as f:
            result = subprocess.run(
                cmd,
                stdin=f,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=timeout
            )
        
        elapsed = time.time() - start_time
        
        if result.returncode != 0:
            print(f"  Error: solver returned {result.returncode}")
            if result.stderr:
                print(f"  stderr: {result.stderr[:200]}")
            return None, None
        
        # Parse output to extract max slot
        max_slot = 0
        for line in result.stdout.strip().split('\n'):
            parts = line.split()
            if len(parts) >= 3:
                try:
                    slot = int(parts[1])
                    max_slot = max(max_slot, slot)
                except ValueError:
                    pass
        
        # Calculate days
        best_days = (max_slot + 3) // 4 if max_slot > 0 else 0
        
        return best_days, elapsed
    
    except subprocess.TimeoutExpired:
        print(f"  Timeout after {timeout}s")
        return None, None
    except Exception as e:
        print(f"  Error: {e}")
        return None, None


def tune_hyperparams(exe_path, input_file, test_count=5, params_to_sweep=None):
    """
    Sweep through hyperparameters and collect results.
    
    Returns list of (param_name, param_value, mean_days, std_days, avg_time_s) tuples.
    """
    results = []
    
    # Determine which parameters to sweep
    if params_to_sweep:
        sweep_dict = {k: SWEEP[k] for k in params_to_sweep if k in SWEEP}
    else:
        sweep_dict = SWEEP
    
    total_sweeps = sum(len(vals) for vals in sweep_dict.values())
    sweep_count = 0
    
    for param_name, param_values in sweep_dict.items():
        for param_value in param_values:
            sweep_count += 1
            print(f"\n[{sweep_count}/{total_sweeps}] Tuning {param_name} = {param_value}")
            print(f"  Running {test_count} trials...")
            
            # Create params dict with this value overriding baseline
            params = BASELINE.copy()
            params[param_name] = param_value
            
            # Run multiple times
            days_list = []
            time_list = []
            
            for trial in range(test_count):
                best_days, runtime = run_solver(exe_path, input_file, params)
                if best_days is not None:
                    days_list.append(best_days)
                    time_list.append(runtime)
                    print(f"    Trial {trial+1}: {best_days} days, {runtime:.2f}s")
                else:
                    print(f"    Trial {trial+1}: FAILED")
            
            if days_list:
                mean_days = statistics.mean(days_list)
                std_days = statistics.stdev(days_list) if len(days_list) > 1 else 0
                avg_time = statistics.mean(time_list)
                
                print(f"  Result: mean={mean_days:.1f}±{std_days:.2f} days, avg_time={avg_time:.2f}s")
                results.append((param_name, param_value, mean_days, std_days, avg_time))
            else:
                print(f"  Result: ALL FAILED")
    
    return results


def save_results(results, output_csv):
    """Save results to CSV file."""
    os.makedirs(os.path.dirname(output_csv) or ".", exist_ok=True)
    
    with open(output_csv, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["param_name", "param_value", "mean_days", "std_days", "avg_time_s"])
        for param_name, param_value, mean_days, std_days, avg_time in results:
            writer.writerow([param_name, param_value, f"{mean_days:.2f}", 
                           f"{std_days:.4f}", f"{avg_time:.3f}"])
    
    print(f"\nResults saved to: {output_csv}")


def print_results_table(results):
    """Print results as a formatted table."""
    print("\n" + "="*70)
    print(f"{'Parameter':<20} {'Value':<15} {'Mean Days':<15} {'Std Dev':<10}")
    print("="*70)
    
    for param_name, param_value, mean_days, std_days, _ in results:
        print(f"{param_name:<20} {str(param_value):<15} {mean_days:<15.2f} {std_days:<10.4f}")
    
    print("="*70)


def main():
    parser = argparse.ArgumentParser(
        description="Hyperparameter tuning for SA solver"
    )
    parser.add_argument("--testcase", required=True,
                       help="Path to test case input file")
    parser.add_argument("--tests", type=int, default=5,
                       help="Number of runs per hyperparameter (default: 5)")
    parser.add_argument("--sweep", nargs='+', 
                       help="Which hyperparameters to sweep (default: all). "
                            "Options: coolingRate, initialTemp, swapProbability, highSlotBias, horizonExtension")
    parser.add_argument("--exe", default="build/exam_scheduler.exe",
                       help="Path to exam_scheduler executable")
    parser.add_argument("--output", default="log/tuning_results.csv",
                       help="Output CSV file")
    
    args = parser.parse_args()
    
    # Verify inputs
    if not os.path.exists(args.testcase):
        print(f"Error: Test case file not found: {args.testcase}")
        sys.exit(1)
    
    if not os.path.exists(args.exe):
        print(f"Error: Executable not found: {args.exe}")
        sys.exit(1)
    
    print(f"Test case: {args.testcase}")
    print(f"Executable: {args.exe}")
    print(f"Runs per config: {args.tests}")
    if args.sweep:
        print(f"Sweeping: {args.sweep}")
    
    # Run tuning
    results = tune_hyperparams(args.exe, args.testcase, args.tests, args.sweep)
    
    if results:
        print_results_table(results)
        save_results(results, args.output)
        print(f"\nTo plot results, run:")
        print(f"  python tools/plot_tuning.py {args.output}")
    else:
        print("\nNo results collected!")
        sys.exit(1)


if __name__ == "__main__":
    main()
