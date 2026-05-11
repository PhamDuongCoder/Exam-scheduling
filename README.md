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

## Adding New Solvers

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

### Customizing Logging

Modify `SASolver::Params` in `include/sa_solver.h`:
- `logInterval`: Log every N iterations (default: 1000, set to 0 to disable)
- `maxIterations`: Total iterations (default: 1,000,000)
- `initialTemp`, `coolingRate`, `minTemp`: SA hyperparameters

