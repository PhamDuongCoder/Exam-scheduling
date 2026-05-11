#include <iostream>
#include <memory>
#include "problem.h"
#include "solver.h"
#include "greedy_solver.h"
#include "sa_solver.h"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [solver] [--log <logfile>]\n";
    std::cerr << "  solver: 'greedy' or 'sa' (default: 'sa')\n";
    std::cerr << "  --log <logfile>: Write convergence log to CSV file (SA only)\n";
}

int main(int argc, char* argv[]) {
    // Parse arguments
    std::string solverChoice = "sa";
    std::string logPath;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--log" && i + 1 < argc) {
            logPath = argv[++i];
        } else if (arg == "greedy" || arg == "sa") {
            solverChoice = arg;
        } else if (arg[0] != '-') {
            solverChoice = arg;
        }
    }

    if (solverChoice != "greedy" && solverChoice != "sa") {
        printUsage(argv[0]);
        return 1;
    }

    // Read problem
    Problem problem;
    problem.readInput();

    // Create solver
    std::unique_ptr<Solver> solver;
    if (solverChoice == "greedy") {
        solver = std::make_unique<GreedySolver>();
    } else {
        auto saSolver = std::make_unique<SASolver>();
        if (!logPath.empty()) {
            saSolver->setLogPath(logPath);
        }
        solver = std::move(saSolver);
    }

    // Solve
    std::vector<int> s, r;
    solver->solve(problem, s, r);

    // Output solution
    problem.writeSolution(s, r);

    return 0;
}
