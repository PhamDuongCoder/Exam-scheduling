#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include "problem.h"

/// Abstract base class for exam scheduling solvers
class Solver {
public:
    virtual ~Solver() = default;
    
    /// Solve the problem
    /// @param problem The problem instance
    /// @param s Output: slot assignment for each class (1-indexed)
    /// @param r Output: room assignment for each class (1-indexed)
    /// @return Number of days used in the solution
    virtual int solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) = 0;
    
    /// Get solver name for display
    virtual std::string getName() const = 0;
};

#endif // SOLVER_H
