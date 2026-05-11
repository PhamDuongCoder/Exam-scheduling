#ifndef GREEDY_SOLVER_H
#define GREEDY_SOLVER_H

#include "solver.h"

/// Greedy solver that constructs a solution by assigning classes to slots/rooms
/// Classes are processed in order of decreasing conflict count
class GreedySolver : public Solver {
public:
    int solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) override;
    
    std::string getName() const override {
        return "Greedy";
    }
};

#endif // GREEDY_SOLVER_H
