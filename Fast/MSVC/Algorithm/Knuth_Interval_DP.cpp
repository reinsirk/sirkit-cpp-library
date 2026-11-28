// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Knuth_Interval_DP.cpp
// 参考（円環へ適用する方法）: https://atcoder.jp/contests/awc0100/editorial/21322
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/knuth_interval_dp.md
//
// API一覧
// KnuthIntervalDP(n,cost)/KnuthIntervalDP(n,max_length,cost): 長さ2以上の区間DPをKnuth最適化で構築。
// D(l,r)=cost(l,r)+min_{l<k<r}(D(l,k)+D(k,r))。長さ0,1は0。costの単調性・四辺形不等式は文書参照。
// 添字は0始まり半開、0<=l<=r<=nかつr-l<=max_length。省略時の最大区間長はn。
// size()/max_length(): 列長/最大区間長、O(1)。get(l,r): 最小費用。split(l,r): 最適な分割の最大k、長さ0,1はnullopt。各O(1)。
// 構築O(n*max_length+1)時間・領域（costがO(1)の場合）。費用と途中の和はint64_tに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_KNUTH_INTERVAL_DP_DEFINED
#define SIRKIT_KNUTH_INTERVAL_DP_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::int64_t KnuthIntervalCostSum(std::int64_t left, std::int64_t right)
{
    return static_cast<std::int64_t>(static_cast<std::uint64_t>(left) + static_cast<std::uint64_t>(right));
}
} // namespace reinsirk::detail

class KnuthIntervalDP
{
    using Value = std::int64_t;
    struct State
    {
        Value value;
        int split;
    };

    std::vector<State> states_;
    int size_ = 0;
    int limit_ = 0;

    static std::uint64_t total_states(int size, int limit) noexcept
    {
        if (limit < 2)
        {
            return 0;
        }
        // 長さ2からlimitまでの区間数は等差数列。長さ0,1は保存しない。
        const auto n = static_cast<std::uint64_t>(size);
        const auto m = static_cast<std::uint64_t>(limit);
        return (m - 1) * (2 * n - m) / 2;
    }

    static std::size_t storage_size(int size, int limit)
    {
        const auto states = total_states(size, limit);
        if (states > std::vector<State>().max_size())
        {
            throw std::length_error("KnuthIntervalDP: too many states");
        }
        return static_cast<std::size_t>(states);
    }

    [[nodiscard]] std::size_t index(int left, int right) const noexcept
    {
        return static_cast<std::size_t>(total_states(size_, right - left - 1)) + static_cast<std::size_t>(left);
    }

    [[nodiscard]] Value value_unchecked(int left, int right) const noexcept
    {
        return right - left < 2 ? 0 : states_[index(left, right)].value;
    }

    void check_range([[maybe_unused]] int left, [[maybe_unused]] int right) const
    {
    }

public:
    template <typename Cost>
    KnuthIntervalDP(int size, Cost cost)
        : KnuthIntervalDP(size, size, std::move(cost))
    {
    }

    template <typename Cost>
    KnuthIntervalDP(int size, int max_length, Cost cost)
        : states_(storage_size(size, max_length)), size_(size), limit_(max_length)
    {
        for (std::int64_t length = 2; length <= limit_; ++length)
        {
            const int span = static_cast<int>(length);
            for (int left = 0; left <= size_ - span; ++left)
            {
                const int right = left + span;
                int first = left + 1;
                int last = right - 1;
                // 同点では最大の分割点を使うと、前層の隣り合う最適点で探索を挟める。
                if (span > 2)
                {
                    first = states_[index(left, right - 1)].split;
                    last = states_[index(left + 1, right)].split;
                }
                Value best = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(value_unchecked(left, first), value_unchecked(first, right));
                int split = first;
                for (int middle = first + 1; middle <= last; ++middle)
                {
                    const Value candidate = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(value_unchecked(left, middle),
                                                                                   value_unchecked(middle, right));
                    if (candidate <= best)
                    {
                        split = middle;
                        best = candidate;
                    }
                }
                const Value interval_cost = std::invoke(cost, left, right);
                states_[index(left, right)] = {/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::KnuthIntervalCostSum(best, interval_cost), split};
            }
        }
    }

    KnuthIntervalDP(const KnuthIntervalDP &) = default;
    KnuthIntervalDP &operator=(const KnuthIntervalDP &) = default;

    KnuthIntervalDP(KnuthIntervalDP &&other) noexcept
        : states_(std::move(other.states_)), size_(std::exchange(other.size_, 0)), limit_(std::exchange(other.limit_, 0))
    {
    }

    KnuthIntervalDP &operator=(KnuthIntervalDP &&other) noexcept
    {
        if (this != &other)
        {
            states_ = std::move(other.states_);
            size_ = std::exchange(other.size_, 0);
            limit_ = std::exchange(other.limit_, 0);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] int max_length() const noexcept
    {
        return limit_;
    }

    [[nodiscard]] Value get(int left, int right) const
    {
        check_range(left, right);
        return value_unchecked(left, right);
    }

    [[nodiscard]] std::optional<int> split(int left, int right) const
    {
        check_range(left, right);
        if (right - left < 2)
        {
            return std::nullopt;
        }
        return states_[index(left, right)].split;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
