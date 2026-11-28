// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Add_State_Ids.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_add_state_ids.md
// 参考（Verify問題）: https://yukicoder.me/problems/no/469
//
// API一覧
// RangeAddStateIds(n): n要素の0で開始。更新で現れる差分木の節点を必要な分だけ持つ。構築O(1)。
// size(): 要素数。id(): 現在の数列のID。初期列のIDは0。同じオブジェクトでIDが等しいことと数列の一致は同値。O(1)。
// add(l,r,delta): 半開区間[l,r)に加算し、新しいIDを返す。各値は法2^64の整数。期待償却O(log(n+1))時間・追加O(log(n+1))領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_ADD_STATE_IDS_DEFINED
#define SIRKIT_RANGE_ADD_STATE_IDS_DEFINED

namespace detail
{
inline std::uint32_t checked_range_add_node_id(std::size_t count)
{
    if (count > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::length_error("RangeAddStateIds: node capacity");
    }
    return static_cast<std::uint32_t>(count);
}
}

class RangeAddStateIds
{
public:
    using state_type = std::uint32_t;

    explicit RangeAddStateIds(std::uint64_t n = 0) : size_(n)
    {
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] state_type id() const noexcept
    {
        return current_;
    }

    state_type add(std::uint64_t first, std::uint64_t last, std::int64_t delta)
    {
        if (first == last || delta == 0)
        {
            return current_;
        }
        const auto change = static_cast<std::uint64_t>(delta);
        auto result = add_difference(current_, 0, size_, first, change);
        if (last < size_)
        {
            result = add_difference(result, 0, size_, last, std::uint64_t{0} - change);
        }
        current_ = result;
        return current_;
    }

private:
    using NodeIds = std::unordered_map<std::uint64_t, state_type>;

    state_type intern(NodeIds &ids, std::uint64_t payload)
    {
        if (payload == 0)
        {
            return 0;
        }
        if (const auto found = ids.find(payload); found != ids.end())
        {
            return found->second;
        }
        const auto result = detail::checked_range_add_node_id(nodes_.size());
        if (nodes_.size() == nodes_.capacity())
        {
            const auto capacity = std::min(nodes_.max_size(), 2 * nodes_.size());
            nodes_.reserve(std::max(nodes_.size() + 1, capacity));
        }
        // 表の追加が成功した後は再確保なしで整数1個を格納できる。
        ids.emplace(payload, result);
        nodes_.push_back(payload);
        return result;
    }

    state_type add_difference(state_type node, std::uint64_t first, std::uint64_t last,
                              std::uint64_t index, std::uint64_t delta)
    {
        const auto payload = nodes_[node];
        if (last - first == 1)
        {
            return intern(leaves_, payload + delta);
        }
        const auto middle = first + (last - first) / 2;
        auto left = static_cast<state_type>(payload >> 32);
        auto right = static_cast<state_type>(payload);
        if (index < middle)
        {
            left = add_difference(left, first, middle, index, delta);
        }
        else
        {
            right = add_difference(right, middle, last, index, delta);
        }
        return intern(branches_, (std::uint64_t{left} << 32) | right);
    }

    std::uint64_t size_ = 0;
    state_type current_ = 0;
    std::vector<std::uint64_t> nodes_{0};
    NodeIds leaves_;
    NodeIds branches_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
