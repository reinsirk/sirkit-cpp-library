// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
struct Affine { std::uint32_t a = 1, b = 0; };
struct Compose {
    Affine operator()(Affine first, Affine second) const {
        return {static_cast<std::uint32_t>(std::uint64_t{first.a} * second.a % 998244353),
                static_cast<std::uint32_t>((std::uint64_t{first.b} * second.a + second.b) % 998244353)};
    }
};
int main() {
    int q{}; verify_in >> q;
    std::vector<std::uint32_t> queries; queries.reserve(q);
    std::vector<Affine> values; values.reserve(q);
    for (int i = 0; i < q; ++i) {
        int type{}; verify_in >> type;
        std::uint32_t x{};
        if (type == 0) { Affine f; verify_in >> f.a >> f.b; values.push_back(f); }
        else if (type == 2) verify_in >> x;
        queries.push_back((static_cast<std::uint32_t>(type) << 30) | x);
    }
    DisjointSparseTable<Affine, Compose> table(values, Affine{});
    std::size_t left = 0, right = 0;
    for (const auto query : queries) {
        if ((query >> 30) == 0) ++right;
        else if ((query >> 30) == 1) ++left;
        else { const auto f = table.prod(left, right); verify_out << (std::uint64_t{f.a} * (query & ((std::uint32_t{1} << 30) - 1)) + f.b) % 998244353 << '\n'; }
    }
}
