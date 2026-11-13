// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
struct PointAffineFunction { std::uint32_t a = 1, b = 0; };
struct PointAffineComposition {
    PointAffineFunction operator()(const PointAffineFunction& left, const PointAffineFunction& right) const {
        return {static_cast<std::uint32_t>(std::uint64_t{left.a} * right.a % 998244353),
                static_cast<std::uint32_t>((std::uint64_t{left.b} * right.a + right.b) % 998244353)};
    }
};
PointAffineFunction point_affine_identity() { return {}; }
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<PointAffineFunction> a(n);
    for (auto& f : a) verify_in >> f.a >> f.b;
    segtree<PointAffineFunction, PointAffineComposition{}, point_affine_identity> tree(a);
    while (q--) {
        int type{}; verify_in >> type;
        if (type == 0) {
            int p{}; PointAffineFunction f; verify_in >> p >> f.a >> f.b;
            tree.set(p, f);
        } else {
            int l{}, r{}; std::uint64_t x{}; verify_in >> l >> r >> x;
            const auto f = tree.prod(l, r);
            verify_out << (std::uint64_t{f.a} * x + f.b) % 998244353 << '\n';
        }
    }
}
