#ifndef SA_SOLVER_H
#define SA_SOLVER_H

#include "solver.h"
#include <string>
#include <optional>

/// Simulated Annealing solver for exam scheduling
/// Starts with a greedy solution and iteratively improves it using SA metaheuristic
class SASolver : public Solver {
public:
    /// Hyperparameters
    struct Params {
        double initialTemp = 100.0;
        double minTemp = 0.01;
        double coolingRate = 0.9999;
        int maxIterations = 50000;
        int horizonExtension = 4;  // Extend search horizon by this many slots
        int logInterval = 1000;    // Log convergence every N iterations (0 = no logging)
        double highSlotBias = 0.7;  // Probability to pick from high-slot classes
        double swapProbability = 0.5; // Probability to do SWAP vs RELOCATE
    };
    
    explicit SASolver(const Params& params) : params_(params) {}
    
    SASolver() : params_(Params{}) {}
    
    /// Set the path for convergence log (CSV file)
    /// If set, convergence data will be logged during solve()
    void setLogPath(const std::string& path) {
        logPath_ = path;
    }
    
    int solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) override;
    
    std::string getName() const override {
        return "Simulated Annealing";
    }

private:
    Params params_;
    std::optional<std::string> logPath_;
};

#endif // SA_SOLVER_H
