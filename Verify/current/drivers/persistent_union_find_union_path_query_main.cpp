// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

int main() {
    constexpr std::int64_t mod = 998244353;
    int n{}, x{}, q{};
    verify_in >> n >> x >> q;
    PersistentUnionFind uf(n);
    std::vector<PersistentUnionFind::Version> versions{uf.version()};
    std::vector<std::int64_t> weights{0}, component_sum(n);
    versions.reserve(n);
    weights.reserve(n);
    while (q--) {
        int type{};
        verify_in >> type;
        if (type == 1) {
            int v{};
            std::int64_t weight{};
            verify_in >> v >> weight;
            const int left = uf.root(x), right = uf.root(v);
            if (left == right) continue;
            const std::int64_t left_size = uf.size(left), right_size = uf.size(right);
            const auto cross_sum = (weight % mod) * left_size % mod * right_size % mod;
            const auto merged_sum = (component_sum[left] + component_sum[right] + cross_sum) % mod;
            if (uf.unite(x, v)) {
                component_sum[uf.root(x)] = merged_sum;
                versions.push_back(uf.version());
                weights.push_back(weight);
            }
        } else if (type == 2) {
            int u{}, v{};
            verify_in >> u >> v;
            if (!uf.issame(u, v)) {
                verify_out << -1 << '\n';
                continue;
            }
            std::int64_t answer = 0;
            if (u != v) {
                std::size_t low = 0, high = versions.size() - 1;
                while (high - low > 1) {
                    const auto middle = low + (high - low) / 2;
                    if (uf.issame(u, v, versions[middle])) high = middle;
                    else low = middle;
                }
                answer = weights[high];
            }
            verify_out << answer << '\n';
            x = static_cast<int>((static_cast<std::int64_t>(x) + answer) % n);
        } else if (type == 3) {
            int v{};
            verify_in >> v;
            verify_out << component_sum[uf.root(v)] << '\n';
        } else {
            int value{};
            verify_in >> value;
            x = (x + value) % n;
        }
    }
}
