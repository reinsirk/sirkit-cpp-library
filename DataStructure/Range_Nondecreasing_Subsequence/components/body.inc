#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_RANGE_NONDECREASING_SUBSEQUENCE_DEFINED
#define SIRKIT_RANGE_NONDECREASING_SUBSEQUENCE_DEFINED

namespace reinsirk::detail
{
template <std::size_t Alphabet>
struct MonotoneSubsequenceSummary
{
    std::array<int, Alphabet * (Alphabet + 1) / 2> lengths{};

    static constexpr std::size_t index(std::size_t low, std::size_t high) noexcept
    {
        return high * (high + 1) / 2 + low;
    }

    [[nodiscard]] constexpr int get(std::size_t low, std::size_t high) const noexcept
    {
        return lengths[index(low, high)];
    }
};

template <std::size_t Alphabet>
constexpr MonotoneSubsequenceSummary<Alphabet> EmptyMonotoneSubsequence() noexcept
{
    return {};
}

template <std::size_t Alphabet>
constexpr MonotoneSubsequenceSummary<Alphabet> SingleMonotoneSubsequence(std::size_t symbol) noexcept
{
    MonotoneSubsequenceSummary<Alphabet> result{};
    for (std::size_t high = symbol; high < Alphabet; ++high)
    {
        for (std::size_t low = 0; low <= symbol; ++low)
        {
            result.lengths[result.index(low, high)] = 1;
        }
    }
    return result;
}

template <std::size_t Alphabet>
constexpr MonotoneSubsequenceSummary<Alphabet> JoinMonotoneSubsequences(
    const MonotoneSubsequenceSummary<Alphabet> &left, const MonotoneSubsequenceSummary<Alphabet> &right) noexcept
{
    MonotoneSubsequenceSummary<Alphabet> result{};
    for (std::size_t high = 0; high < Alphabet; ++high)
    {
        for (std::size_t low = 0; low <= high; ++low)
        {
            // 境界値cutを共有し、前半はcut以下、後半はcut以上の値を選ぶ。
            auto best = left.get(low, low) + right.get(low, high);
            for (std::size_t cut = low + 1; cut <= high; ++cut)
            {
                best = std::max(best, left.get(low, cut) + right.get(cut, high));
            }
            result.lengths[result.index(low, high)] = best;
        }
    }
    return result;
}
} // namespace reinsirk::detail

template <std::size_t Alphabet>
    requires (1 <= Alphabet && Alphabet <= 256)
class RangeNondecreasingSubsequence
{
    using Summary = reinsirk::detail::MonotoneSubsequenceSummary<Alphabet>;
    using Tree = segtree<Summary, reinsirk::detail::JoinMonotoneSubsequences<Alphabet>, reinsirk::detail::EmptyMonotoneSubsequence<Alphabet>>;

    Tree tree_;

    static void check_symbol([[maybe_unused]] int symbol)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (symbol < 0 || symbol >= static_cast<int>(Alphabet))
        {
            throw std::out_of_range("RangeNondecreasingSubsequence: symbol");
        }
#endif
    }

    static Summary leaf(int symbol)
    {
        check_symbol(symbol);
        return ::reinsirk::detail::SingleMonotoneSubsequence<Alphabet>(static_cast<std::size_t>(symbol));
    }

    static std::vector<Summary> build(std::span<const int> symbols)
    {
        static_cast<void>(::reinsirk::detail::CheckedSegmentArrayCount(symbols.size(), "RangeNondecreasingSubsequence: size"));
        std::vector<Summary> nodes;
        nodes.reserve(symbols.size());
        for (const auto symbol : symbols)
        {
            nodes.push_back(leaf(symbol));
        }
        return nodes;
    }

public:
    RangeNondecreasingSubsequence() = default;

    explicit RangeNondecreasingSubsequence(std::span<const int> symbols) : tree_(build(symbols))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return tree_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return tree_.empty();
    }

    void set(int position, int symbol)
    {
        tree_.set(position, leaf(symbol));
    }

    [[nodiscard]] int longest(int left, int right) const
    {
        return tree_.prod(left, right).get(0, Alphabet - 1);
    }

    [[nodiscard]] int longest(int left, int right, int low, int high) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check_symbol(low);
        check_symbol(high);
        if (low > high)
        {
            throw std::invalid_argument("RangeNondecreasingSubsequence: symbol interval");
        }
#endif
        return tree_.prod(left, right).get(static_cast<std::size_t>(low), static_cast<std::size_t>(high));
    }

    [[nodiscard]] int replacement_count(int left, int right) const
    {
        const auto keep = longest(left, right);
        return right - left - keep;
    }

    [[nodiscard]] int all() const
    {
        return tree_.all_prod().get(0, Alphabet - 1);
    }
};

#endif
