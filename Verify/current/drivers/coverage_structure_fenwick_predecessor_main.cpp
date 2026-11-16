// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <string>
#include <vector>
int main() {
    int n{}, q{}; std::string initial; verify_in >> n >> q >> initial;
    auto tree = [&] {
        if (std::count(initial.begin(), initial.end(), '1') <= n / 64) {
            fenwick_tree<int> result(n);
            for (int i = 0; i < n; ++i) if (initial[i] == '1') result.set(i, 1);
            return result;
        }
        std::vector<int> values(n);
        for (int i = 0; i < n; ++i) values[i] = initial[i] - '0';
        return fenwick_tree<int>(values);
    }();
    while (q--) {
        int type{}, k{}; verify_in >> type >> k;
        if (type < 2) {
            const char value = type == 0 ? '1' : '0';
            if (initial[k] != value) { tree.set(k, value - '0'); initial[k] = value; }
        }
        else if (type == 2) verify_out << tree.get(k) << '\n';
        else if (type == 3) {
            const int result = tree.upper_bound(tree.sum(0, k));
            verify_out << (result == n ? -1 : result) << '\n';
        } else {
            const int count = tree.sum(0, k + 1);
            verify_out << (count ? tree.lower_bound(count) : -1) << '\n';
        }
    }
}
