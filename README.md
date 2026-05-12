# Exam Scheduling Solver - Refactored CMake Project

## Project Structure

```
Exam scheduling/
├── CMakeLists.txt              # CMake build configuration
├── include/
│   ├── problem.h               # Problem struct: input data and I/O
│   ├── solver.h                # Abstract Solver base class
│   ├── greedy_solver.h         # Greedy solver implementation
│   └── sa_solver.h             # Simulated Annealing solver implementation
├── src/
│   ├── main.cpp                # Entry point: reads input, runs solver, outputs result
│   ├── problem.cpp             # Problem I/O implementation
│   ├── greedy_solver.cpp       # GreedySolver implementation
│   ├── sa_solver.cpp           # SASolver implementation
│   └── SA.cpp                  # Original monolithic file (for reference)
├── test_cases/
│   ├── Input/
│   │   ├── TC_01 ... TC_05
│   └── Output/
│       ├── TC_01 ... TC_05
└── build/                      # CMake build directory (generated)
    └── exam_scheduler.exe      # Compiled executable
```

## Architecture

### Core Components

1. **Problem struct** (`include/problem.h`, `src/problem.cpp`)
   - Stores all input data: N classes, M rooms, conflicts
   - Handles input parsing via `readInput()`
   - Handles output formatting via `writeSolution()`
   - Utility: `calculateDays()` static method

2. **Solver abstract base class** (`include/solver.h`)
   - Pure virtual `solve()` method
   - Pure virtual `getName()` method
   - Extensible design for new solvers (e.g., genetic algorithm, ACO)

3. **GreedySolver** (`include/greedy_solver.h`, `src/greedy_solver.cpp`)
   - Constructs solution by processing classes in decreasing order of conflicts
   - Assigns each class to first available non-conflicting slot+room pair
   - Fast initialization for SA and standalone use

4. **SASolver** (`include/sa_solver.h`, `src/sa_solver.cpp`)
   - Configurable hyperparameters via `SASolver::Params` struct:
     - `initialTemp`: starting temperature (default: 100.0)
     - `minTemp`: stopping temperature (default: 0.01)
     - `coolingRate`: temperature decay per iteration (default: 0.9999)
     - `maxIterations`: iteration limit (default: 1,000,000)
     - `horizonExtension`: search horizon extension (default: 4 slots)
   - Starts with greedy initialization
   - Iteratively improves via random moves and acceptance probability

5. **main.cpp** (`src/main.cpp`)
   - Reads problem from stdin
   - CLI argument selects solver: `greedy` or `sa` (default: `sa`)
   - Writes solution to stdout

## Building

```bash
cd "c:\Users\LENOVO\Documents\GitHub\Exam scheduling"
cmake -B build
cmake --build build
```

The executable is created at: `build/exam_scheduler.exe`

## Usage

### Run with Greedy Solver (fast)
```powershell
Get-Content test_cases\Input\TC_01 | .\build\exam_scheduler.exe greedy
```

### Run with Simulated Annealing (slower but better quality)
```powershell
Get-Content test_cases\Input\TC_01 | .\build\exam_scheduler.exe sa
```
Or simply:
```powershell
Get-Content test_cases\Input\TC_01 | .\build\exam_scheduler.exe
```

## Input/Output Format

### Input Format

```
Line 1: N M
Line 2: d1 d2 ... dN
Line 3: c1 c2 ... cM
Line 4: K
Line 5 to 4+K: i j (conflict pairs)
```

**Description:**
- **N**: Number of classes (1-indexed)
- **M**: Number of rooms (1-indexed)
- **d[i]**: Number of students in class i (i = 1 to N)
- **c[j]**: Capacity of room j (j = 1 to M)
- **K**: Number of conflict pairs
- **Conflict pairs**: Classes that have common students (cannot be in same time slot)

### Output Format

```
1 s[1] r[1]
2 s[2] r[2]
...
N s[N] r[N]
```

Each line i contains:
- Class ID (1-indexed)
- Time slot s[i] (1-indexed, consecutive across days)
- Room r[i] (1-indexed)

To add a new solver (e.g., Genetic Algorithm):

1. Create header `include/ga_solver.h`:
```cpp
#ifndef GA_SOLVER_H
#define GA_SOLVER_H
#include "solver.h"

class GASolver : public Solver {
public:
    int solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) override;
    std::string getName() const override { return "Genetic Algorithm"; }
};

#endif
```

2. Create implementation `src/ga_solver.cpp` with your algorithm

3. Update `CMakeLists.txt` to include the new source file

4. Update `main.cpp` to register the new solver in the CLI

## Convergence Analysis

SA solver can optionally log convergence data to a CSV file to visualize how the best solution improves over iterations.

### Logging Convergence Data

Run the solver with the `--log` option:

```bash
Get-Content test_cases\Input\TC_01 | .\build\exam_scheduler.exe sa --log log\convergence.csv
```

This creates a CSV file with columns: `iteration`, `bestDays`

### Plotting Convergence

Use the Python plotting script to visualize the convergence curve:

```bash
python tools\plot_convergence.py log\convergence.csv
```

This generates a PNG file in the same directory showing:
- X-axis: Iteration number
- Y-axis: Best days found
- Line plot with markers for each logged iteration
- Grid for easier reading
- Convergence statistics (initial, final, improvement)

### Automated Workflow

Use the PowerShell convenience script to run the solver and generate a plot:

```powershell
powershell -ExecutionPolicy Bypass .\tools\run_and_plot.ps1 -TestCase TC_01
```

Options:
- `-TestCase <name>`: Test case to run (default: TC_01)
- `-LogDir <dir>`: Log directory (default: log)
- `-LogFile <path>`: Custom log file path

This will:
1. Run SA solver on the test case with logging
2. Automatically generate and save a convergence plot
3. Display convergence statistics

## Test Results

All test cases were run with SA solver (50,000 iterations, logging every 1,000 iterations):

| Test Case | Classes | Rooms | Conflicts | Initial Days | Final Days | Improvement | Time |
|-----------|---------|-------|-----------|--------------|-----------|-------------|------|
| TC_01     | 1000    | 20    | 247,432   | 31           | 31        | 0          | 26.7s |
| TC_02     | 500     | 20    | 12,500    | 19           | 19        | 0          | 9.1s  |
| TC_03     | 200     | 20    | 2,000     | 30           | 30        | 0          | 30.3s |
| TC_04     | 20      | 20    | 0         | 2            | 1         | ✅ 1       | 226ms |
| TC_05     | 30      | 20    | 12        | 3            | 3         | 0          | 591ms |

**Summary:**
- ✅ **TC_04**: Successfully improved from 2 → 1 days (50% reduction)
- **TC_01, TC_02, TC_03, TC_05**: Already near-optimal solutions from greedy initialization

**Convergence Plots:** Generated for all test cases in `log/TC_*.png`

## Hyperparameter Tuning

The SA solver has several tunable hyperparameters. Use the automated tuning system to find optimal values for your test cases.

### Baseline Configuration

Default hyperparameters (in `tools/tune_hyperparams.py`):
- **initialTemp**: 100.0 (starting temperature)
- **minTemp**: 0.01 (stopping temperature)
- **coolingRate**: 0.9999 (temperature decay per iteration)
- **maxIterations**: 50000 (iteration limit)
- **horizonExtension**: 4 (search window size in slots)
- **swapProbability**: 0.3 (probability of SWAP move vs RELOCATE)
- **highSlotBias**: 0.5 (bias toward classes in higher time slots)

### Running Tuning

Run the tuning script to sweep hyperparameters:

```powershell
# Sweep coolingRate with 5 runs per value (default)
python tools/tune_hyperparams.py --testcase test_cases/Input/TC_02 --sweep coolingRate

# Sweep multiple hyperparameters
python tools/tune_hyperparams.py --testcase test_cases/Input/TC_02 --sweep coolingRate initialTemp

# Sweep all hyperparameters with 3 runs each (faster testing)
python tools/tune_hyperparams.py --testcase test_cases/Input/TC_02 --tests 3

# Custom executable and output path
python tools/tune_hyperparams.py --testcase test_cases/Input/TC_02 --exe build/exam_scheduler.exe --output log/tuning.csv
```

**Options:**
- `--testcase <path>`: Input test case file (required)
- `--sweep <params>`: Which hyperparameters to sweep (default: all)
  - Available: `coolingRate`, `initialTemp`, `swapProbability`, `highSlotBias`, `horizonExtension`
- `--tests <count>`: Number of runs per hyperparameter (default: 5)
- `--exe <path>`: Path to exam_scheduler executable (default: `build/exam_scheduler.exe`)
- `--output <path>`: Output CSV file (default: `log/tuning_results.csv`)

### Plotting Tuning Results

After tuning completes, generate plots showing hyperparameter sensitivity:

```powershell
python tools/plot_tuning.py log/tuning_results.csv
```

This creates `log/tuning_results.png` with subplots for each hyperparameter, showing:
- X-axis: Hyperparameter value
- Y-axis: Best days (mean ± standard deviation)
- Sensitivity analysis across parameter ranges

### Using Optimized Hyperparameters

Once you identify good hyperparameters, pass them via CLI:

```powershell
Get-Content test_cases/Input/TC_02 | .\build\exam_scheduler.exe sa --coolingRate 0.9995 --initialTemp 50 --swapProbability 0.5
```

**Example CLI Arguments:**
```powershell
.\build\exam_scheduler.exe sa `
  --initialTemp 100 `
  --minTemp 0.01 `
  --coolingRate 0.9999 `
  --maxIterations 50000 `
  --horizonExtension 4 `
  --swapProbability 0.3 `
  --highSlotBias 0.5
```

