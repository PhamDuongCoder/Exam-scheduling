#include "sa_solver.h"
#include "greedy_solver.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <vector>

namespace {
    // Global random number generator
    std::mt19937 rng(std::random_device{}());

    struct Move {
        int classId;
        int newSlot;
        int newRoom;
        Move(int i, int ts, int rm) : classId(i), newSlot(ts), newRoom(rm) {}
    };

    /// Generate a random valid move from the current solution
    Move generateNeighbour(int N, int M, const Problem& problem,
                           const std::vector<int>& s, const std::vector<int>& r,
                           int maxSlot, int horizonExtension) {
        std::uniform_int_distribution<int> classDist(1, N);
        int classId = classDist(rng);

        // Create candidates: valid (slot, room) pairs for this class
        std::vector<std::pair<int, int>> candidates;

        // Limit search horizon to current max slot + extension
        int horizon = std::min(maxSlot + horizonExtension, N);
        
        // Mark slots with conflicts
        std::vector<bool> conflictedSpots(horizon + 1, false);
        for (int j : problem.conflict[classId]) {
            if (s[j] > 0) {
                conflictedSpots[s[j]] = true;
            }
        }

        // Track occupied slots
        std::vector<std::vector<bool>> occupied(horizon + 1, std::vector<bool>(M + 1, false));
        for (int i = 1; i <= N; i++) {
            if (i != classId && s[i] > 0 && s[i] <= horizon) {
                occupied[s[i]][r[i]] = true;
            }
        }

        // Try all valid (slot, room) combinations
        for (int ts = 1; ts <= horizon; ts++) {
            if (conflictedSpots[ts]) continue;

            // Prefer current room if available
            if (!occupied[ts][r[classId]] && problem.c[r[classId]] >= problem.d[classId]) {
                candidates.push_back({ts, r[classId]});
            }
            
            // Try other rooms
            for (int rm = 1; rm <= M; rm++) {
                if (rm == r[classId]) continue;
                if (!occupied[ts][rm] && problem.c[rm] >= problem.d[classId]) {
                    candidates.push_back({ts, rm});
                }
            }
        }

        // If no candidates, stay in place
        if (candidates.empty()) {
            return Move(classId, s[classId], r[classId]);
        }

        // Return a random candidate
        std::uniform_int_distribution<int> candidateDist(0, (int)candidates.size() - 1);
        auto [newTs, newRm] = candidates[candidateDist(rng)];
        return Move(classId, newTs, newRm);
    }

    /// Update occupied slots after a move
    void updateOccupied(std::vector<std::vector<bool>>& occupied, int N, int M,
                        const std::vector<int>& s, const std::vector<int>& r) {
        for (int ts = 0; ts <= N; ts++) {
            for (int rm = 0; rm <= M; rm++) {
                occupied[ts][rm] = false;
            }
        }
        for (int i = 1; i <= N; i++) {
            if (s[i] > 0) {
                occupied[s[i]][r[i]] = true;
            }
        }
    }
}

int SASolver::solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) {
    int N = problem.N;

    // Initialize with greedy solver
    GreedySolver greedy;
    greedy.solve(problem, s, r);

    // Track best solution found
    std::vector<int> bestS = s;
    std::vector<int> bestR = r;
    int bestMaxSlot = *std::max_element(s.begin() + 1, s.end());
    int bestDays = Problem::calculateDays(bestMaxSlot);

    // Track current solution
    int currentMaxSlot = bestMaxSlot;
    int currentDays = bestDays;

    std::vector<std::vector<bool>> occupied(N + 1, std::vector<bool>(problem.M + 1, false));
    updateOccupied(occupied, N, problem.M, s, r);

    double T = params_.initialTemp;
    std::uniform_real_distribution<double> probDist(0.0, 1.0);

    // Setup convergence logging if requested
    std::ofstream logFile;
    if (logPath_ && params_.logInterval > 0) {
        // Create log directory if it doesn't exist
        std::filesystem::path logDir("log");
        std::filesystem::create_directories(logDir);
        
        // Open log file
        logFile.open(logPath_.value());
        if (logFile.is_open()) {
            // Write header
            logFile << "iteration,bestDays\n";
            logFile << "0," << bestDays << "\n";
            logFile.flush();
        }
    }

    for (int iter = 0; iter < params_.maxIterations && T > params_.minTemp; iter++) {
        Move mv = generateNeighbour(N, problem.M, problem, s, r, currentMaxSlot, params_.horizonExtension);

        // Apply move temporarily
        int oldSlot = s[mv.classId];
        int oldRoom = r[mv.classId];

        s[mv.classId] = mv.newSlot;
        r[mv.classId] = mv.newRoom;

        int candidateMaxSlot = currentMaxSlot;
        if (mv.newSlot > currentMaxSlot) {
            candidateMaxSlot = mv.newSlot;
        } else if (oldSlot == currentMaxSlot) {
            // Rescan if we freed the maximum slot
            candidateMaxSlot = *std::max_element(s.begin() + 1, s.end());
        }

        int delta = candidateMaxSlot - currentMaxSlot;

        // Accept move if it improves or with probability exp(-delta/T)
        if (delta < 0 || probDist(rng) < std::exp(-delta / T)) {
            // Accept
            currentMaxSlot = candidateMaxSlot;
            currentDays = Problem::calculateDays(currentMaxSlot);
            updateOccupied(occupied, N, problem.M, s, r);

            // Update best if improved
            if (currentDays < bestDays) {
                bestDays = currentDays;
                bestMaxSlot = currentMaxSlot;
                bestS = s;
                bestR = r;
            }
        } else {
            // Reject
            s[mv.classId] = oldSlot;
            r[mv.classId] = oldRoom;
        }

        // Log convergence data at specified intervals
        if (logFile.is_open() && params_.logInterval > 0 && (iter + 1) % params_.logInterval == 0) {
            logFile << (iter + 1) << "," << bestDays << "\n";
            logFile.flush();
        }

        T *= params_.coolingRate;
    }

    if (logFile.is_open()) {
        logFile.close();
    }

    // Return best solution found
    s = bestS;
    r = bestR;
    return bestDays;
}
