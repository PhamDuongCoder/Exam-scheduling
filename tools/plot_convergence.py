#!/usr/bin/env python3
"""
Plot convergence of SA solver from CSV log file.

Usage:
    python plot_convergence.py <csv_file>

The CSV file should have columns: iteration, bestDays
Output PNG will be saved in the same directory with .png extension.
"""

import sys
import os
import csv
import matplotlib.pyplot as plt


def plot_convergence(csv_file):
    """Read CSV and plot convergence curve."""
    
    # Check if file exists
    if not os.path.exists(csv_file):
        print(f"Error: File '{csv_file}' not found.")
        sys.exit(1)
    
    # Read CSV
    iterations = []
    best_days = []
    
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                iterations.append(int(row['iteration']))
                best_days.append(int(row['bestDays']))
    except (KeyError, ValueError) as e:
        print(f"Error reading CSV: {e}")
        print("Expected columns: iteration, bestDays")
        sys.exit(1)
    
    if not iterations:
        print("No data found in CSV file.")
        sys.exit(1)
    
    # Create plot
    plt.figure(figsize=(10, 6))
    plt.plot(iterations, best_days, linewidth=2, marker='o', markersize=4, label='Best Days')
    
    plt.title('SA Solver Convergence', fontsize=14, fontweight='bold')
    plt.xlabel('Iteration', fontsize=12)
    plt.ylabel('Best Days', fontsize=12)
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # Save PNG
    png_file = csv_file.replace('.csv', '.png')
    plt.savefig(png_file, dpi=150, bbox_inches='tight')
    print(f"Plot saved to: {png_file}")
    
    # Also display basic statistics
    print(f"\nConvergence Statistics:")
    print(f"  Initial best: {best_days[0]} days")
    print(f"  Final best:   {best_days[-1]} days")
    print(f"  Improvement:  {best_days[0] - best_days[-1]} days")
    print(f"  Total iterations logged: {iterations[-1]}")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python plot_convergence.py <csv_file>")
        sys.exit(1)
    
    plot_convergence(sys.argv[1])
