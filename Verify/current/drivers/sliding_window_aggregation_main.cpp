// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
struct Affine {
    std::uint64_t a, b;
};
struct Compose {
    static constexpr std::uint64_t mod = 998244353;
    Affine operator()(const Affine& first, const Affine& second) const {
        return {first.a * second.a % mod, (first.b * second.a + second.b) % mod};
    }
};
int main() {
    std::size_t q{}; verify_in >> q;
    SlidingWindowAggregation<Affine, Compose> queue(Affine{1, 0});
    while (q--) {
        unsigned type{}; verify_in >> type;
        if (type == 0) {
            Affine value{}; verify_in >> value.a >> value.b; queue.push_back(value);
        } else if (type == 1) queue.pop_front();
        else {
            std::uint64_t x{}; verify_in >> x;
            const auto value = queue.all_prod();
            verify_out << (value.a * x + value.b) % Compose::mod << '\n';
        }
    }
}
