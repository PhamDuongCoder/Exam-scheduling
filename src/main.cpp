#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include "problem.h"
#include "solver.h"
#include "greedy_solver.h"
#include "sa_solver.h"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [solver] [--log <logfile>] [hyperparams]\n";
    std::cerr << "  solver: 'greedy' or 'sa' (default: 'sa')\n";
    std::cerr << "  --log <logfile>: Write convergence log to CSV file (SA only)\n";
    std::cerr << "  Hyperparameters (SA only):\n";
    std::cerr << "    --initialTemp <value>\n";
    std::cerr << "    --minTemp <value>\n";
    std::cerr << "    --coolingRate <value>\n";
    std::cerr << "    --maxIterations <value>\n";
    std::cerr << "    --horizonExtension <value>\n";
    std::cerr << "    --swapProbability <value>\n";
    std::cerr << "    --highSlotBias <value>\n";
}

int main(int argc, char* argv[]) {
    // Parse arguments
    std::string solverChoice = "sa";
    std::string logPath;
    SASolver::Params saParams;  // Use defaults initially
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--log" && i + 1 < argc) {
            logPath = argv[++i];
        } else if (arg == "--initialTemp" && i + 1 < argc) {
            saParams.initialTemp = std::stod(argv[++i]);
        } else if (arg == "--minTemp" && i + 1 < argc) {
            saParams.minTemp = std::stod(argv[++i]);
        } else if (arg == "--coolingRate" && i + 1 < argc) {
            saParams.coolingRate = std::stod(argv[++i]);
        } else if (arg == "--maxIterations" && i + 1 < argc) {
            saParams.maxIterations = std::stoi(argv[++i]);
        } else if (arg == "--horizonExtension" && i + 1 < argc) {
            saParams.horizonExtension = std::stoi(argv[++i]);
        } else if (arg == "--swapProbability" && i + 1 < argc) {
            saParams.swapProbability = std::stod(argv[++i]);
        } else if (arg == "--highSlotBias" && i + 1 < argc) {
            saParams.highSlotBias = std::stod(argv[++i]);
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
        auto saSolver = std::make_unique<SASolver>(saParams);
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
