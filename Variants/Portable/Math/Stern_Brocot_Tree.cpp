// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Math/Stern_Brocot_Tree.cpp
// 参考: https://nyaannyaan.github.io/library/math/stern-brocot-tree-binary-search.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/stern_brocot_tree.md
//
// API一覧
// [SternBrocotTree]
// Fraction: uint64_tの{分子,分母}。正の分数を受け取り、既約分数を返す。
// Run: {方向,回数}。方向は'L'か'R'、回数は正。長さ0のパスは根1/1。
// encode(f): 連長圧縮した根からfへのパス。decode(path): パスから分数。
// ancestor(f,depth): 深さdepthの祖先。根は深さ0。深すぎる場合はnullopt。
// lca(a,b): a,bの最小共通祖先。range(f): f以下の部分木が表す開区間。
// approximate(f,max_numerator,max_denominator): 分子・分母が上限以下の最近接の下側・上側の分数。
// 範囲や近似に有限な端点がなければ0/1または1/0。
// 時間は扱うラン数Rに対してO(R+1)。分数からのラン数はO(log(max(分子,分母)+1))。
// encodeの出力以外の追加領域はO(1)。
// decodeは結果がuint64_tに収まらない場合にstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STERN_BROCOT_TREE_DEFINED
#define SIRKIT_STERN_BROCOT_TREE_DEFINED

class SternBrocotTree
{
public:
    using Fraction = std::pair<std::uint64_t, std::uint64_t>;
    using Run = std::pair<char, std::uint64_t>;

private:
    struct Interval
    {
        Fraction lower{0, 1};
        Fraction upper{1, 0};
    };

    static std::uint64_t AddScaled(std::uint64_t value, std::uint64_t other, std::uint64_t count)
    {
        if (other != 0 && count > (std::numeric_limits<std::uint64_t>::max() - value) / other)
        {
            throw std::overflow_error("SternBrocotTree: fraction");
        }
        return value + other * count;
    }

    static void Move(Interval &interval, char direction, std::uint64_t count)
    {
        auto &[a, b] = direction == 'L' ? interval.upper : interval.lower;
        const auto [c, d] = direction == 'L' ? interval.lower : interval.upper;
        a = AddScaled(a, c, count);
        b = AddScaled(b, d, count);
    }

    static Fraction Mediant(const Interval &interval)
    {
        const auto [a, b] = interval.lower;
        const auto [c, d] = interval.upper;
        return {AddScaled(a, c, 1), AddScaled(b, d, 1)};
    }

    static void CheckFraction([[maybe_unused]] Fraction fraction)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (fraction.first == 0 || fraction.second == 0)
        {
            throw std::invalid_argument("SternBrocotTree: positive fraction required");
        }
#endif
    }

    static Run TakeRun(Fraction &fraction)
    {
        auto &[a, b] = fraction;
        if (a < b)
        {
            const auto count = (b - 1) / a;
            b -= count * a;
            return {'L', count};
        }
        const auto count = (a - 1) / b;
        a -= count * b;
        return {'R', count};
    }

public:
    static std::vector<Run> encode(Fraction fraction)
    {
        CheckFraction(fraction);
        std::vector<Run> path;
        while (fraction.first != fraction.second)
        {
            path.push_back(TakeRun(fraction));
        }
        return path;
    }

    static Fraction decode(std::span<const Run> path)
    {
        Interval interval;
        for (const auto &[direction, count] : path)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if ((direction != 'L' && direction != 'R') || count == 0)
            {
                throw std::invalid_argument("SternBrocotTree: invalid path");
            }
#endif
            Move(interval, direction, count);
        }
        return Mediant(interval);
    }

    static std::optional<Fraction> ancestor(Fraction fraction, std::uint64_t depth)
    {
        CheckFraction(fraction);
        Interval interval;
        while (depth != 0 && fraction.first != fraction.second)
        {
            const auto [direction, count] = TakeRun(fraction);
            const auto steps = std::min(depth, count);
            Move(interval, direction, steps);
            depth -= steps;
        }
        if (depth != 0)
        {
            return std::nullopt;
        }
        return Mediant(interval);
    }

    static Fraction lca(Fraction a, Fraction b)
    {
        CheckFraction(a);
        CheckFraction(b);
        Interval interval;
        while (a.first != a.second && b.first != b.second)
        {
            const auto [left_direction, left_count] = TakeRun(a);
            const auto [right_direction, right_count] = TakeRun(b);
            if (left_direction != right_direction)
            {
                break;
            }
            Move(interval, left_direction, std::min(left_count, right_count));
            if (left_count != right_count)
            {
                break;
            }
        }
        return Mediant(interval);
    }

    static std::pair<Fraction, Fraction> approximate(Fraction fraction, std::uint64_t max_numerator, std::uint64_t max_denominator)
    {
        CheckFraction(fraction);
        Interval interval;
        if (max_numerator == 0 || max_denominator == 0)
        {
            return {interval.lower, interval.upper};
        }
        while (interval.lower.first <= max_numerator - interval.upper.first &&
               interval.lower.second <= max_denominator - interval.upper.second)
        {
            if (fraction.first == fraction.second)
            {
                const auto exact = Mediant(interval);
                return {exact, exact};
            }
            const auto [direction, count] = TakeRun(fraction);
            const auto &[a, b] = direction == 'L' ? interval.upper : interval.lower;
            const auto &[c, d] = direction == 'L' ? interval.lower : interval.upper;
            auto steps = count;
            if (c != 0)
            {
                steps = std::min(steps, (max_numerator - a) / c);
            }
            if (d != 0)
            {
                steps = std::min(steps, (max_denominator - b) / d);
            }
            Move(interval, direction, steps);
        }
        return {interval.lower, interval.upper};
    }

    static std::pair<Fraction, Fraction> range(Fraction fraction)
    {
        CheckFraction(fraction);
        Interval interval;
        while (fraction.first != fraction.second)
        {
            const auto [direction, count] = TakeRun(fraction);
            Move(interval, direction, count);
        }
        return {interval.lower, interval.upper};
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
