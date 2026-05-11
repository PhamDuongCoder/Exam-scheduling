#include "greedy_solver.h"
#include <algorithm>
#include <numeric>

int GreedySolver::solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) {
    int N = problem.N;
    int M = problem.M;
    
    // Initialize slot and room assignments
    s = std::vector<int>(N + 1, 0);
    r = std::vector<int>(N + 1, 0);
    
    // Track room-time slot occupation
    std::vector<std::vector<bool>> occupied(N + 1, std::vector<bool>(M + 1, false));
    
    // Sort classes in order of decreasing conflict count
    std::vector<int> order(N);
    std::iota(order.begin(), order.end(), 1);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return problem.conflict[a].size() > problem.conflict[b].size();
    });
    
    // Assign time slot and room for each class
    for (int i : order) {
        bool assigned = false;
        
        // Try time slots in increasing order
        for (int ts = 1; ts <= N && !assigned; ts++) {
            // Check if this time slot conflicts with any conflicting class
            bool tsOk = true;
            for (int j : problem.conflict[i]) {
                if (s[j] == ts) {
                    tsOk = false;
                    break;
                }
            }
            if (!tsOk) continue;
            
            // Found a non-conflicting time slot, try to find a room
            for (int rm = 1; rm <= M; rm++) {
                if (!occupied[ts][rm] && problem.c[rm] >= problem.d[i]) {
                    s[i] = ts;
                    r[i] = rm;
                    occupied[ts][rm] = true;
                    assigned = true;
                    break;
                }
            }
        }
    }
    
    // Calculate number of days
    int maxSlot = *std::max_element(s.begin() + 1, s.end());
    return Problem::calculateDays(maxSlot);
}
