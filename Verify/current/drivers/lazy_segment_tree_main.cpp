// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
using RangeAffineValue = std::uint32_t;
struct RangeAffineUpdate { std::uint32_t a = 1, b = 0; };
RangeAffineValue range_affine_op(RangeAffineValue a, RangeAffineValue b) {
    const std::uint32_t sum = a + b;
    return sum >= 998244353u ? sum - 998244353u : sum;
}
RangeAffineValue range_affine_identity() { return 0; }
RangeAffineValue range_affine_mapping(RangeAffineUpdate f, RangeAffineValue value, std::size_t length) {
    return static_cast<std::uint32_t>((std::uint64_t{f.a} * value + std::uint64_t{f.b} * length) % 998244353);
}
RangeAffineUpdate range_affine_composition(RangeAffineUpdate newer, RangeAffineUpdate older) {
    return {static_cast<std::uint32_t>(std::uint64_t{newer.a} * older.a % 998244353),
            static_cast<std::uint32_t>((std::uint64_t{newer.a} * older.b + newer.b) % 998244353)};
}
RangeAffineUpdate range_affine_id() { return {}; }
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<RangeAffineValue> a(n);
    for (auto& x : a) verify_in >> x;
    lazy_segtree<RangeAffineValue, range_affine_op, range_affine_identity, RangeAffineUpdate,
                 range_affine_mapping, range_affine_composition, range_affine_id> tree(a);
    while (q--) {
        int type{}, l{}, r{}; verify_in >> type >> l >> r;
        if (type == 0) {
            RangeAffineUpdate f; verify_in >> f.a >> f.b;
            tree.apply(l, r, f);
        } else verify_out << tree.prod(l, r) << '\n';
    }
}
