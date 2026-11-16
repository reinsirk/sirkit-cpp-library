// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <utility>
#include <vector>
inline void clean(Heap& values, Heap& deleted) {
    while (!values.empty() && !deleted.empty() && values.top() == deleted.top()) {
        values.pop(); deleted.pop();
    }
}
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<int> values(n), negatives(n);
    for (int i = 0; i < n; ++i) { verify_in >> values[i]; negatives[i] = -values[i]; }
    Heap low(std::move(values)), high(std::move(negatives)), low_deleted, high_deleted;
    while (q--) {
        int type{}; verify_in >> type;
        if (type == 0) { int x{}; verify_in >> x; low.push(x); high.push(-x); }
        else if (type == 1) {
            clean(low, low_deleted);
            const int x = low.pop_value(); if (high.top()==-x) high.pop(); else high_deleted.push(-x); verify_out << x << '\n';
        } else {
            clean(high, high_deleted);
            const int x = -high.pop_value(); if (low.top()==x) low.pop(); else low_deleted.push(x); verify_out << x << '\n';
        }
    }
}
