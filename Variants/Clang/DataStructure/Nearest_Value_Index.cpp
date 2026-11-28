// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Nearest_Value_Index.cpp
// 参考（最近順位の用途）: https://atcoder.jp/contests/abc364/tasks/abc364_d
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/nearest_value_index.md
//
// API一覧
// NearestValueIndex(values): int64_tの座標を保持。重複も個別に数える。O(n*log(n+1)+1)時間・O(n+1)領域。
// size()/empty(): 座標数/空か。O(1)。空構築可。
// kth_nearest(center,k): 距離順で1始まりk番目のNearestValue{value,distance}。同距離なら座標の小さい順。
// k=0またはk>nではnullopt。O(log(n+1)+1)時間。distanceはuint64_t。
// count_within(center,radius): 距離がradius以下の座標数。O(log(n+1)+1)時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NEAREST_VALUE_INDEX_DEFINED
#define SIRKIT_NEAREST_VALUE_INDEX_DEFINED

struct NearestValue
{
    std::int64_t value = 0;
    std::uint64_t distance = 0;
};

class NearestValueIndex
{
    static constexpr std::uint64_t kSignBit = std::uint64_t{1} << 63;
    std::vector<std::uint64_t> points_;

    static constexpr std::uint64_t order_key(std::int64_t value) noexcept
    {
        return static_cast<std::uint64_t>(value) ^ kSignBit;
    }

    [[nodiscard]] NearestValue describe(std::size_t index, std::uint64_t center) const noexcept
    {
        const auto value = points_[index];
        return {
            static_cast<std::int64_t>(value ^ kSignBit),
            std::max(center, value) - std::min(center, value),
        };
    }

public:
    NearestValueIndex() = default;

    explicit NearestValueIndex(std::span<const std::int64_t> values)
    {
        points_.reserve(values.size());
        for (const auto value : values)
        {
            points_.push_back(order_key(value));
        }
        std::sort(points_.begin(), points_.end());
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return points_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return points_.empty();
    }

    [[nodiscard]] std::optional<NearestValue> kth_nearest(std::int64_t center, std::size_t rank) const noexcept
    {
        if (rank == 0 || rank > size())
        {
            return std::nullopt;
        }
        const auto key = order_key(center);
        const auto split = static_cast<std::size_t>(std::lower_bound(points_.begin(), points_.end(), key) - points_.begin());
        const auto right_size = size() - split;
        auto low = rank > right_size ? rank - right_size : 0;
        auto high = std::min(rank, split);
        // 左側は手前から逆順、右側は順にたどると、どちらの距離列も昇順になる。
        // 左側の次候補が右側の採用末尾以下なら、同距離のときも左側を優先する。
        while (low < high)
        {
            const auto take_left = std::midpoint(low, high);
            const auto take_right = rank - take_left;
            const auto next_left = key - points_[split - take_left - 1];
            const auto last_right = points_[split + take_right - 1] - key;
            if (next_left <= last_right)
            {
                low = take_left + 1;
            }
            else
            {
                high = take_left;
            }
        }
        if (low == 0)
        {
            return describe(split + rank - 1, key);
        }
        const auto last_left = describe(split - low, key);
        if (low == rank)
        {
            return last_left;
        }
        const auto last_right = describe(split + (rank - low) - 1, key);
        return last_left.distance > last_right.distance ? last_left : last_right;
    }

    [[nodiscard]] std::size_t count_within(std::int64_t center, std::uint64_t radius) const noexcept
    {
        constexpr auto kLastKey = std::numeric_limits<std::uint64_t>::max();
        const auto key = order_key(center);
        const auto low = key - std::min(radius, key);
        const auto high = key + std::min(radius, kLastKey - key);
        const auto first = std::lower_bound(points_.begin(), points_.end(), low);
        return static_cast<std::size_t>(std::upper_bound(first, points_.end(), high) - first);
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
