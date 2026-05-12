#!/usr/bin/env python3
"""
Plot hyperparameter tuning results.

Reads tuning_results.csv and creates a plot with subplots for each hyperparameter.

Usage:
    python plot_tuning.py log/tuning_results.csv
"""

import sys
import csv
from pathlib import Path
from collections import defaultdict

import matplotlib.pyplot as plt
import numpy as np


def load_results(csv_file):
    """Load tuning results from CSV."""
    results = defaultdict(lambda: {"values": [], "means": [], "stds": []})
    
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            param_name = row["param_name"]
            param_value = float(row["param_value"])
            mean_days = float(row["mean_days"])
            std_days = float(row["std_days"])
            
            results[param_name]["values"].append(param_value)
            results[param_name]["means"].append(mean_days)
            results[param_name]["stds"].append(std_days)
    
    # Sort each parameter by value
    for param_name in results:
        sorted_data = sorted(zip(results[param_name]["values"], 
                                results[param_name]["means"],
                                results[param_name]["stds"]))
        results[param_name]["values"] = [x[0] for x in sorted_data]
        results[param_name]["means"] = [x[1] for x in sorted_data]
        results[param_name]["stds"] = [x[2] for x in sorted_data]
    
    return results


def plot_results(results, output_file):
    """Create subplots for each hyperparameter."""
    num_params = len(results)
    if num_params == 0:
        print("No results to plot!")
        return
    
    # Arrange subplots
    ncols = 2
    nrows = (num_params + 1) // 2
    
    fig, axes = plt.subplots(nrows, ncols, figsize=(14, 4*nrows))
    
    # Convert to list of axes for consistent handling
    if isinstance(axes, np.ndarray):
        if axes.ndim == 1:
            axes = axes.tolist()
        else:
            axes = axes.flatten().tolist()
    elif not isinstance(axes, list):
        axes = [axes]
    
    for idx, (param_name, data) in enumerate(sorted(results.items())):
        ax = axes[idx]
        
        values = data["values"]
        means = data["means"]
        stds = data["stds"]
        
        # Plot mean ± std
        ax.errorbar(values, means, yerr=stds, fmt='o-', linewidth=2, 
                   markersize=8, capsize=5, capthick=2, label="Mean ± Std")
        
        # Format x-axis for integer values
        if all(v == int(v) for v in values):
            ax.set_xticks(values)
        
        ax.set_xlabel(param_name, fontsize=11, fontweight='bold')
        ax.set_ylabel("Best Days", fontsize=11, fontweight='bold')
        ax.set_title(f"Sensitivity to {param_name}", fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3)
        ax.legend()
    
    # Hide unused subplots
    for idx in range(num_params, len(axes)):
        axes[idx].set_visible(False)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Plot saved to: {output_file}")
    
    # Print summary
    print("\nHyperparameter Sensitivity Summary:")
    print("="*60)
    for param_name, data in sorted(results.items()):
        best_idx = data["means"].index(min(data["means"]))
        best_value = data["values"][best_idx]
        best_days = data["means"][best_idx]
        print(f"{param_name:<20}: Best at {best_value} → {best_days:.2f} days")
    print("="*60)


def main():
    if len(sys.argv) < 2:
        print("Usage: python plot_tuning.py <tuning_results.csv>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    if not Path(csv_file).exists():
        print(f"Error: File not found: {csv_file}")
        sys.exit(1)
    
    # Determine output file
    output_file = Path(csv_file).parent / "tuning_results.png"
    
    print(f"Reading: {csv_file}")
    results = load_results(csv_file)
    
    print(f"Parameters found: {', '.join(sorted(results.keys()))}")
    
    plot_results(results, str(output_file))


if __name__ == "__main__":
    main()
