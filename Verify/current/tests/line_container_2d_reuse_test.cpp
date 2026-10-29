// SPDX-License-Identifier: CC0-1.0
#ifndef HEADER
#define HEADER "../../../DataStructure/line-container-2d.cpp"
#endif
#include HEADER
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using Point = std::pair<std::int64_t, std::int64_t>;
using Points = std::vector<Point>;
using ld = LineContainer2D::ld;
std::uint64_t checks = 0;

// 元の全点を走査する整数内積を独立参照にする。
void verify(const LineContainer2D& tree, const Points& points, std::int64_t a, std::int64_t b) {
    assert(tree.size() == points.size());
    assert(tree.empty() == points.empty());
    if (points.empty()) {
        assert(!tree.max_optional(ld(a), ld(b)));
        assert(!tree.min_optional(ld(a), ld(b)));
        assert(!tree.argmax_optional(ld(a), ld(b)));
        assert(!tree.argmin_optional(ld(a), ld(b)));
        assert(std::isinf(tree.Xmax) && tree.Xmax < 0);
        assert(std::isinf(tree.Xmin) && tree.Xmin > 0);
        checks += 8;
        return;
    }
    auto low = std::numeric_limits<std::int64_t>::max();
    auto high = std::numeric_limits<std::int64_t>::min();
    std::int64_t xmin = points.front().first, xmax = xmin;
    std::int64_t ymin = points.front().second, ymax = ymin;
    for (auto [x, y] : points) {
        const auto value = a * x + b * y;
        low = std::min(low, value); high = std::max(high, value);
        xmin = std::min(xmin, x); xmax = std::max(xmax, x);
        ymin = std::min(ymin, y); ymax = std::max(ymax, y);
    }
    auto valid = [&](ld x, ld y, ld target) {
        bool present = false;
        for (auto point : points) present |= x == ld(point.first) && y == ld(point.second);
        assert(present);
        assert(ld(a) * x + ld(b) * y == target);
        checks += 2;
    };
    assert(tree.Xmin == ld(xmin) && tree.Xmax == ld(xmax));
    assert(tree.Ymin == ld(ymin) && tree.Ymax == ld(ymax));
    assert(tree.max_ld(a, b) == ld(high));
    assert(tree.min_ld(a, b) == ld(low));
    assert(tree.max_ld(ld(a), ld(b)) == ld(high));
    assert(tree.min_ld(ld(a), ld(b)) == ld(low));
    assert(tree.max_ll(a, b) == high && tree.min_ll(a, b) == low);
    assert(tree.max_rounded_ll(a, b) == high && tree.min_rounded_ll(a, b) == low);
    assert(tree.max_optional(ld(a), ld(b)) == ld(high));
    assert(tree.min_optional(ld(a), ld(b)) == ld(low));
    auto maximum = tree.max_with_point(ld(a), ld(b));
    auto minimum = tree.min_with_point(ld(a), ld(b));
    assert(maximum.value == ld(high) && minimum.value == ld(low));
    valid(maximum.x, maximum.y, high); valid(minimum.x, minimum.y, low);
    auto p = tree.argmax(ld(a), ld(b)); valid(p.first, p.second, high);
    p = tree.argmin(ld(a), ld(b)); valid(p.first, p.second, low);
    p = *tree.argmax_optional(ld(a), ld(b)); valid(p.first, p.second, high);
    p = *tree.argmin_optional(ld(a), ld(b)); valid(p.first, p.second, low);
    checks += 13;
}

int main() {
    std::mt19937_64 rng(0x918b4701);
    for (int epoch = 0; epoch < 12; ++epoch) {
        LineContainer2D tree, other;
        Points points, other_points;
        for (int i = 0; i < 600; ++i) {
            const int operation = int(rng() % 12);
            if (operation < 7) {
                Point p{std::int64_t(rng() % 2049) - 1024, std::int64_t(rng() % 2049) - 1024};
                if (i % 4 == 0) p = {i % 2 ? i : -i, 0};
                if (i % 11 == 0 && !points.empty()) p = points.back();
                tree.add(ld(p.first), ld(p.second)); points.push_back(p);
            } else if (operation == 7) {
                LineContainer2D copy(tree); verify(copy, points, 11, -19);
                other = copy; other_points = points;
            } else if (operation == 8) {
                LineContainer2D moved(std::move(tree));
                verify(tree, {}, 1, 2); verify(moved, points, -23, 17);
                tree = std::move(moved); verify(moved, {}, 3, 5);
            } else if (operation == 9) {
                tree.swap(other); points.swap(other_points);
            } else if (operation == 10) {
                tree.clear(); points.clear();
            } else {
                auto* self = &tree;
                tree = *self; tree = std::move(*self);
            }
            verify(tree, points, std::int64_t(rng() % 2049) - 1024, std::int64_t(rng() % 2049) - 1024);
            verify(tree, points, 0, i % 2 ? 1 : -1);
            verify(tree, points, i % 2 ? 1 : -1, 0);
            if (i % 29 == 0) verify(other, other_points, 0, 0);
        }
    }
    // 同傾きで極値を交互に更新し、消したノードの再利用と所有の移動を重ねる。
    for (int repetition = 0; repetition < 20; ++repetition) {
        LineContainer2D tree; Points points;
        for (int i = 0; i < 256; ++i) {
            Point p{std::int64_t(i % 2 ? i : -i), 0};
            tree.add(p.first, p.second); points.push_back(p);
            verify(tree, points, 1, 7); verify(tree, points, -1, -7);
        }
        LineContainer2D copy(tree), moved(std::move(tree));
        verify(tree, {}, 1, 0); copy.clear(); copy.swap(moved);
        verify(copy, points, -1, 1); verify(moved, {}, 0, 1);
        moved.add(9, 7); verify(moved, {{9, 7}}, 3, 5);
    }
    // 半整数の丸め・旧切捨て・飽和を別々に確認する。
    for (ld coordinate : {ld{1.5}, ld{-1.5}}) {
        LineContainer2D tree; tree.add(coordinate, 0);
        const std::int64_t rounded = coordinate > 0 ? 2 : -2;
        const std::int64_t truncated = coordinate > 0 ? 1 : -1;
        assert(tree.max_rounded_ll(1, 0) == rounded && tree.min_rounded_ll(1, 0) == rounded);
        assert(tree.max_ll(1, 0) == truncated && tree.min_ll(1, 0) == truncated);
        checks += 2;
    }
    {
        LineContainer2D tree; tree.add(1e20L, -1e20L);
        assert(tree.max_ll(1, 0) == tree.INTEGER_LIMIT);
        assert(tree.min_ll(0, 1) == -tree.INTEGER_LIMIT);
        checks += 2;
    }
    // 有限積の相殺、及び検査省略時も残る比のoverflow保護。
    {
        LineContainer2D tree;
        const ld large = std::numeric_limits<ld>::max() / 2;
        tree.add(large, large);
        assert(tree.max_ld(ld{4}, ld{-4}) == 0);
        assert(tree.min_ld(ld{4}, ld{-4}) == 0);
        assert(tree.max_with_point(ld{4}, ld{-4}).value == 0);
        checks += 3;
        const ld small = std::numeric_limits<ld>::denorm_min();
        if (small > 0) {
            bool failed = false;
            try { (void)tree.max_ld(small, large); }
            catch (const std::overflow_error&) { failed = true; }
            assert(failed); ++checks;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    {
        LineContainer2D empty;
        bool failed = false;
        try { (void)empty.max_ld(ld{1}, ld{1}); }
        catch (const std::out_of_range&) { failed = true; }
        assert(failed); ++checks;
        failed = false;
        try { (void)empty.max_optional(std::numeric_limits<ld>::quiet_NaN(), 1); }
        catch (const std::invalid_argument&) { failed = true; }
        assert(failed); ++checks;
    }
#endif
    std::cout << checks << " independent line2d value/arg/ownership/reuse checks passed\n";
}
