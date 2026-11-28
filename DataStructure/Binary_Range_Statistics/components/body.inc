#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_BINARY_RANGE_STATISTICS_DEFINED
#define SIRKIT_BINARY_RANGE_STATISTICS_DEFINED

struct BinaryRunStatistics
{
    int length = 0;
    int ones = 0;
    std::array<int, 2> prefix{};
    std::array<int, 2> suffix{};
    std::array<int, 2> longest{};
};

namespace detail
{
constexpr BinaryRunStatistics binary_run_identity()
{
    return {};
}

constexpr BinaryRunStatistics binary_run_uniform(int length, bool bit)
{
    BinaryRunStatistics result;
    result.length = length;
    result.ones = bit ? length : 0;
    result.prefix[bit] = result.suffix[bit] = result.longest[bit] = length;
    return result;
}

constexpr BinaryRunStatistics binary_run_merge(const BinaryRunStatistics &a, const BinaryRunStatistics &b)
{
    BinaryRunStatistics result;
    result.length = a.length + b.length;
    result.ones = a.ones + b.ones;
    for (int bit = 0; bit < 2; ++bit)
    {
        result.prefix[bit] = a.prefix[bit] + (a.prefix[bit] == a.length ? b.prefix[bit] : 0);
        result.suffix[bit] = b.suffix[bit] + (b.suffix[bit] == b.length ? a.suffix[bit] : 0);
        result.longest[bit] = std::max({a.longest[bit], b.longest[bit], a.suffix[bit] + b.prefix[bit]});
    }
    return result;
}

// fのbit0/bit1は入力が0/1のときに返す値。
inline constexpr std::uint8_t kBinaryRunSetZero = 0;
inline constexpr std::uint8_t kBinaryRunFlip = 1;
inline constexpr std::uint8_t kBinaryRunIdentity = 2;
inline constexpr std::uint8_t kBinaryRunSetOne = 3;

constexpr std::uint8_t binary_run_no_change()
{
    return kBinaryRunIdentity;
}

constexpr std::uint8_t binary_run_compose(std::uint8_t newer, std::uint8_t older)
{
    const unsigned zero = (newer >> (older & 1)) & 1;
    const unsigned one = (newer >> (older >> 1)) & 1;
    return static_cast<std::uint8_t>(zero | (one << 1));
}

constexpr BinaryRunStatistics binary_run_map(std::uint8_t action, BinaryRunStatistics value)
{
    if (action == kBinaryRunIdentity)
    {
        return value;
    }
    if (action == kBinaryRunFlip)
    {
        value.ones = value.length - value.ones;
        std::swap(value.prefix[0], value.prefix[1]);
        std::swap(value.suffix[0], value.suffix[1]);
        std::swap(value.longest[0], value.longest[1]);
        return value;
    }
    return binary_run_uniform(value.length, action == kBinaryRunSetOne);
}
}

class BinaryRangeStatistics
{
    using Tree = lazy_segtree<BinaryRunStatistics, detail::binary_run_merge, detail::binary_run_identity,
                              std::uint8_t, detail::binary_run_map, detail::binary_run_compose, detail::binary_run_no_change>;

public:
    explicit BinaryRangeStatistics(std::string_view bits = {}) : tree_(MakeLeaves(bits))
    {
    }

    explicit BinaryRangeStatistics(int n) : tree_(ZeroLeaves(n))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return tree_.size();
    }

    void flip(int left, int right)
    {
        tree_.apply(left, right, detail::kBinaryRunFlip);
    }

    void assign(int left, int right, bool bit)
    {
        tree_.apply(left, right, bit ? detail::kBinaryRunSetOne : detail::kBinaryRunSetZero);
    }

    [[nodiscard]] BinaryRunStatistics stats(int left, int right)
    {
        return tree_.prod(left, right);
    }

    [[nodiscard]] BinaryRunStatistics all_stats()
    {
        return tree_.all_prod();
    }

private:
    Tree tree_;

    static std::vector<BinaryRunStatistics> MakeLeaves(std::string_view bits)
    {
        detail::CheckedSegmentArrayCount(bits.size(), "BinaryRangeStatistics: size");
        std::vector<BinaryRunStatistics> leaves;
        leaves.reserve(bits.size());
        for (const char bit : bits)
        {
            if (bit != '0' && bit != '1')
            {
                throw std::invalid_argument("BinaryRangeStatistics: invalid bit");
            }
            leaves.push_back(detail::binary_run_uniform(1, bit == '1'));
        }
        return leaves;
    }

    static std::vector<BinaryRunStatistics> ZeroLeaves(int size)
    {
        if (size < 0)
        {
            throw std::length_error("BinaryRangeStatistics: negative size");
        }
        return std::vector<BinaryRunStatistics>(static_cast<std::size_t>(size), detail::binary_run_uniform(1, false));
    }
};
#endif
