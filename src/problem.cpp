#include "problem.h"
#include <algorithm>

void Problem::readInput() {
    std::cin >> N >> M;

    // Input classes' info
    d = std::vector<int>(N + 1);
    for (int i = 1; i <= N; i++) {
        std::cin >> d[i];
    }

    // Input rooms' info
    c = std::vector<int>(M + 1);
    for (int j = 1; j <= M; j++) {
        std::cin >> c[j];
    }

    // Input conflicts
    conflict = std::vector<std::set<int>>(N + 1);
    std::cin >> K;
    int x, y;
    for (int k = 0; k < K; k++) {
        std::cin >> x >> y;
        conflict[x].insert(y);
        conflict[y].insert(x);
    }
}

void Problem::writeSolution(const std::vector<int>& s, const std::vector<int>& r) const {
    for (int i = 1; i <= N; i++) {
        std::cout << i << " " << s[i] << " " << r[i] << "\n";
    }
}
