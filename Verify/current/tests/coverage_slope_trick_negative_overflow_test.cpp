// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Slope_Trick.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {
template<class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const std::overflow_error&) {
        return;
    }
    assert(false);
}
} // namespace

int main() {
    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    sirkit::SlopeTrick constant;
    constant.add_constant(low);
    rejects([&] { constant.add_constant(-1); });
    assert(constant.empty() && constant.eval(-3) == low && constant.min_value() == low);

    sirkit::SlopeTrick f;
    f.add_left_hinge(0);
    f.shift(1);
    rejects([&] { f.add_left_hinge(low); });
    assert(!f.empty() && f.size() == 1);
    assert(f.argmin().lower == 1 && !f.argmin().upper);

    // 自己代入の後も片側の関数と最小区間を保持する。
    const auto& for_copy = f;
    f = for_copy;
    auto& for_move = f;
    f = std::move(for_move);
    for (std::int64_t x = -4; x <= 4; ++x) assert(f.eval(x) == std::max<std::int64_t>(1 - x, 0));
    f.clear_left();
    f.add_right_hinge(2);
    assert(!f.empty() && f.size() == 1 && !f.argmin().lower && f.argmin().upper == 2);
    for (std::int64_t x = -4; x <= 4; ++x) assert(f.eval(x) == std::max<std::int64_t>(x - 2, 0));
}
