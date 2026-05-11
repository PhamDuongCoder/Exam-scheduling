#ifndef PROBLEM_H
#define PROBLEM_H

#include <vector>
#include <set>
#include <iostream>

struct Problem {
    int N;  // Number of classes
    int M;  // Number of rooms
    int K;  // Number of conflict pairs
    
    std::vector<int> d;  // d[i] = number of students in class i (1-indexed)
    std::vector<int> c;  // c[j] = capacity of room j (1-indexed)
    std::vector<std::set<int>> conflict;  // conflict[i] = set of classes that conflict with class i
    
    Problem() : N(0), M(0), K(0) {}
    
    /// Read problem input from stdin
    void readInput();
    
    /// Write solution to stdout
    /// @param s slot assignment for each class (1-indexed)
    /// @param r room assignment for each class (1-indexed)
    void writeSolution(const std::vector<int>& s, const std::vector<int>& r) const;
    
    /// Calculate number of days from maximum slot
    static int calculateDays(int maxSlot) {
        return (maxSlot + 3) / 4;
    }
};

#endif // PROBLEM_H
