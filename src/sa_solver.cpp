#include "sa_solver.h"
#include "greedy_solver.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <vector>

namespace {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> probDist(0.0, 1.0);

    // -------------------------------------------------------------------------
    // Pick a class to move, biased toward classes at high slots.
    // With probability highSlotBias, pick from classes in the last "day"
    // (slots >= maxSlot - 3). Otherwise pick uniformly at random.
    // -------------------------------------------------------------------------
    int pickClass(int N, const std::vector<int>& s, int maxSlot, double highSlotBias) {
        if (probDist(rng) < highSlotBias) {
            std::vector<int> highSlot;
            for (int i = 1; i <= N; i++)
                if (s[i] >= maxSlot - 3) highSlot.push_back(i);
            if (!highSlot.empty()) {
                std::uniform_int_distribution<int> d(0, (int)highSlot.size() - 1);
                return highSlot[d(rng)];
            }
        }
        std::uniform_int_distribution<int> d(1, N);
        return d(rng);
    }

    // -------------------------------------------------------------------------
    // RELOCATE: move classId to a new (slot, room).
    // Uses the shared occupied matrix directly (O(1) lookup).
    // Temporarily frees classId's current position during candidate search.
    // Returns {-1, -1} if no valid candidate found (no-op).
    // -------------------------------------------------------------------------
    struct RelocateResult { int newSlot, newRoom; };

    RelocateResult generateRelocate(int classId, int N, int M,
                                    const Problem& problem,
                                    const std::vector<int>& s,
                                    const std::vector<int>& r,
                                    std::vector<std::vector<bool>>& occupied,
                                    int maxSlot, int horizonExtension) {
        int horizon = std::min(maxSlot + horizonExtension, N);

        // Mark conflicted slots
        std::vector<bool> conflicted(horizon + 1, false);
        for (int j : problem.conflict[classId])
            if (s[j] > 0 && s[j] <= horizon) conflicted[s[j]] = true;

        // Temporarily free classId's current position so it can be re-used
        occupied[s[classId]][r[classId]] = false;

        std::vector<std::pair<int,int>> candidates;
        for (int ts = 1; ts <= horizon; ts++) {
            if (conflicted[ts]) continue;
            // Prefer current room (cheaper, avoids unnecessary room changes)
            if (!occupied[ts][r[classId]] && problem.c[r[classId]] >= problem.d[classId])
                candidates.push_back({ts, r[classId]});
            for (int rm = 1; rm <= M; rm++) {
                if (rm == r[classId]) continue;
                if (!occupied[ts][rm] && problem.c[rm] >= problem.d[classId])
                    candidates.push_back({ts, rm});
            }
        }

        // Restore
        occupied[s[classId]][r[classId]] = true;

        if (candidates.empty()) return {-1, -1};

        std::uniform_int_distribution<int> d(0, (int)candidates.size() - 1);
        auto [newTs, newRm] = candidates[d(rng)];
        return {newTs, newRm};
    }

    // -------------------------------------------------------------------------
    // SWAP: exchange the time slots of classA and classB (rooms stay fixed).
    // After swap: classA goes to slotB with roomA, classB goes to slotA with roomB.
    // Validity requires:
    //   - roomA is free at slotB (excluding classB's own room)
    //   - roomB is free at slotA (excluding classA's own room)
    //   - no new conflicts for either class at their new slot
    // Returns classB = -1 if no valid swap found after several attempts.
    // -------------------------------------------------------------------------
    struct SwapResult { int classB; };

    SwapResult generateSwap(int classA, int N, const Problem& problem,
                            const std::vector<int>& s, const std::vector<int>& r,
                            std::vector<std::vector<bool>>& occupied) {
        std::uniform_int_distribution<int> d(1, N);

        for (int attempt = 0; attempt < 15; attempt++) {
            int classB = d(rng);
            if (classB == classA || s[classB] == s[classA]) continue;

            int slotA = s[classA], slotB = s[classB];
            int roomA = r[classA], roomB = r[classB];

            // Temporarily free both, then check cross-availability
            occupied[slotA][roomA] = false;
            occupied[slotB][roomB] = false;
            bool roomOk = !occupied[slotB][roomA] && !occupied[slotA][roomB];
            occupied[slotA][roomA] = true;
            occupied[slotB][roomB] = true;

            if (!roomOk) continue;

            // Conflict check: classA at slotB (classB is vacating slotB)
            bool ok = true;
            for (int j : problem.conflict[classA]) {
                if (j != classB && s[j] == slotB) { ok = false; break; }
            }
            if (!ok) continue;

            // Conflict check: classB at slotA (classA is vacating slotA)
            for (int j : problem.conflict[classB]) {
                if (j != classA && s[j] == slotA) { ok = false; break; }
            }
            if (!ok) continue;

            return {classB};
        }
        return {-1};
    }
}

// =============================================================================
// SASolver::solve
// =============================================================================
int SASolver::solve(const Problem& problem, std::vector<int>& s, std::vector<int>& r) {
    int N = problem.N, M = problem.M;

    // --- Greedy initialization ---
    GreedySolver greedy;
    greedy.solve(problem, s, r);

    // --- Occupied matrix (maintained incrementally, O(1) per move) ---
    std::vector<std::vector<bool>> occupied(N + 1, std::vector<bool>(M + 1, false));
    for (int i = 1; i <= N; i++) occupied[s[i]][r[i]] = true;

    int currentMaxSlot = *std::max_element(s.begin() + 1, s.end());
    int currentDays    = Problem::calculateDays(currentMaxSlot);

    std::vector<int> bestS = s, bestR = r;
    int bestDays = currentDays;

    double T = params_.initialTemp;

    // --- Convergence log buffer (written to file in one shot at the end) ---
    std::vector<std::pair<int,int>> logBuffer;
    bool logging = logPath_.has_value() && params_.logInterval > 0;
    if (logging) logBuffer.push_back({0, bestDays});

    // =========================================================================
    // Main SA loop
    // =========================================================================
    for (int iter = 0; iter < params_.maxIterations && T > params_.minTemp; iter++) {

        int classA = pickClass(N, s, currentMaxSlot, params_.highSlotBias);
        bool accepted = false;

        if (probDist(rng) < params_.swapProbability) {
            // --- SWAP move ---
            auto [classB] = generateSwap(classA, N, problem, s, r, occupied);
            if (classB != -1) {
                int slotA = s[classA], slotB = s[classB];
                int roomA = r[classA], roomB = r[classB];

                // Apply swap (incremental occupied update)
                occupied[slotA][roomA] = false;
                occupied[slotB][roomB] = false;
                s[classA] = slotB; r[classA] = roomA;
                s[classB] = slotA; r[classB] = roomB;
                occupied[slotB][roomA] = true;
                occupied[slotA][roomB] = true;

                // Swap exchanges slot values between two classes -> maxSlot unchanged
                // delta = 0, always accept
                accepted = true;
            }
        } else {
            // --- RELOCATE move ---
            auto [newSlot, newRoom] = generateRelocate(
                classA, N, M, problem, s, r, occupied,
                currentMaxSlot, params_.horizonExtension);

            if (newSlot != -1) {
                int oldSlot = s[classA], oldRoom = r[classA];

                // Compute candidate max slot without modifying s yet
                int candidateMaxSlot = currentMaxSlot;
                if (newSlot > currentMaxSlot) {
                    candidateMaxSlot = newSlot;
                } else if (oldSlot == currentMaxSlot) {
                    // Temporarily apply to rescan
                    s[classA] = newSlot;
                    candidateMaxSlot = *std::max_element(s.begin() + 1, s.end());
                    s[classA] = oldSlot;
                }

                int delta = candidateMaxSlot - currentMaxSlot;
                if (delta < 0 || probDist(rng) < std::exp(-delta / T)) {
                    // Accept: apply move (incremental occupied update)
                    occupied[oldSlot][oldRoom] = false;
                    s[classA] = newSlot;
                    r[classA] = newRoom;
                    occupied[newSlot][newRoom] = true;

                    currentMaxSlot = candidateMaxSlot;
                    currentDays    = Problem::calculateDays(currentMaxSlot);
                    accepted = true;
                }
            }
        }

        // Update best solution
        if (accepted && currentDays < bestDays) {
            bestDays = currentDays;
            bestS = s;
            bestR = r;
        }

        // Buffer convergence log point
        if (logging && (iter + 1) % params_.logInterval == 0)
            logBuffer.push_back({iter + 1, bestDays});

        T *= params_.coolingRate;
    }

    // --- Write log file in one shot ---
    if (logging && !logBuffer.empty()) {
        std::filesystem::create_directories(
            std::filesystem::path(logPath_.value()).parent_path());
        std::ofstream logFile(logPath_.value());
        if (logFile.is_open()) {
            logFile << "iteration,bestDays\n";
            for (auto [it, days] : logBuffer)
                logFile << it << ',' << days << '\n';
        }
    }

    s = bestS;
    r = bestR;
    return bestDays;
}