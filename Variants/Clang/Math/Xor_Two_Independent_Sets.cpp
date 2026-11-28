// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Xor_Two_Independent_Sets.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_two_independent_sets.md
// 参考（線形マトロイド和）: https://atcoder.jp/contests/game-24/editorial/25827
//
// API一覧
// XorTwoIndependentSets(): 空にする。uint64_tをGF(2)のベクトルとして扱う。
// insert(value): ベクトルを追加する。最大個数が増えたときtrue。各入力の番号は0始まり。
// rank(): 別々に線形独立な二組へ分けられる入力の最大個数。O(1)。
// groups(): 最大個数を実現する二組の入力番号を、それぞれ昇順で返す。O(W)時間・領域。
// W=64。n回のinsertは合計O(nW+W^3)時間・O(W)領域。乱数は使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_XOR_TWO_INDEPENDENT_SETS_DEFINED
#define SIRKIT_XOR_TWO_INDEPENDENT_SETS_DEFINED

namespace detail
{
class XorUnionBasis
{
    static constexpr auto kBits = std::numeric_limits<std::uint64_t>::digits;

public:
    [[nodiscard]] std::pair<std::uint64_t, std::uint64_t> Reduce(std::uint64_t value) const noexcept
    {
        std::uint64_t mask = 0;
        while (value != 0)
        {
            const auto bit = std::bit_width(value) - 1;
            const auto &[pivot, trace] = rows_[bit];
            if (pivot == 0)
            {
                break;
            }
            value ^= pivot;
            mask ^= trace;
        }
        return {value, mask};
    }

    void Add(std::uint64_t value, std::uint64_t identity = 0) noexcept
    {
        const auto [remainder, trace] = Reduce(value);
        if (remainder != 0)
        {
            rows_[std::bit_width(remainder) - 1] = {remainder, identity ^ trace};
        }
    }

private:
    std::array<std::pair<std::uint64_t, std::uint64_t>, kBits> rows_{};
};
} // namespace detail

class XorTwoIndependentSets
{
    static constexpr auto kBits = std::numeric_limits<std::uint64_t>::digits;
    static constexpr std::size_t kGroups = 2;
    static constexpr std::size_t kCapacity = kGroups * kBits;
    static constexpr std::size_t kUnseen = kCapacity + 1;

    struct Selected
    {
        std::uint64_t value = 0;
        std::size_t original = 0;
        std::size_t group = kGroups;
    };

    struct GroupBasis
    {
        detail::XorUnionBasis basis;
        std::array<std::size_t, kBits> members{};
        std::size_t size = 0;
    };

public:
    bool insert(std::uint64_t value) noexcept
    {
        const auto index = input_count_++;
        if (used_ == kCapacity || blocked_.Reduce(value).first == 0)
        {
            return false;
        }

        const auto start = used_;
        selected_[start] = {value, index, kGroups};
        std::array<std::size_t, kCapacity + 1> parent{};
        parent.fill(kUnseen);
        parent[start] = start;
        std::array<std::size_t, kCapacity + 1> queue{};
        queue[0] = start;
        std::size_t back = 1;
        for (std::size_t front = 0; front < back; ++front)
        {
            const auto id = queue[front];
            for (std::size_t group = 0; group < kGroups; ++group)
            {
                if (selected_[id].group == group)
                {
                    continue;
                }
                const auto &destination = groups_[group];
                const auto [remainder, trace] = destination.basis.Reduce(selected_[id].value);
                if (remainder != 0)
                {
                    Assign(id, group, parent);
                    return true;
                }
                for (auto support = trace; support != 0; support &= support - 1)
                {
                    const auto replace = destination.members[std::countr_zero(support)];
                    if (parent[replace] == kUnseen)
                    {
                        parent[replace] = id;
                        queue[back++] = replace;
                    }
                }
            }
        }
        // 到達したベクトルが張る空間を両組で飽和している。今後の入力もこの空間内なら追加できない。
        for (std::size_t i = 0; i < back; ++i)
        {
            blocked_.Add(selected_[queue[i]].value);
        }
        return false;
    }

    [[nodiscard]] std::size_t rank() const noexcept
    {
        return used_;
    }

    [[nodiscard]] std::array<std::vector<std::size_t>, kGroups> groups() const
    {
        std::array<std::vector<std::size_t>, kGroups> answer;
        for (std::size_t i = 0; i < used_; ++i)
        {
            answer[selected_[i].group].push_back(selected_[i].original);
        }
        return answer;
    }

private:
    void Assign(std::size_t id, std::size_t group, const std::array<std::size_t, kCapacity + 1> &parent) noexcept
    {
        while (id != used_)
        {
            const auto previous_group = selected_[id].group;
            selected_[id].group = group;
            group = previous_group;
            id = parent[id];
        }
        selected_[used_++].group = group;
        groups_ = {};
        for (std::size_t i = 0; i < used_; ++i)
        {
            auto &destination = groups_[selected_[i].group];
            destination.members[destination.size] = i;
            destination.basis.Add(selected_[i].value, std::uint64_t{1} << destination.size);
            ++destination.size;
        }
    }

    std::size_t input_count_ = 0;
    std::size_t used_ = 0;
    std::array<Selected, kCapacity + 1> selected_{};
    std::array<GroupBasis, kGroups> groups_{};
    detail::XorUnionBasis blocked_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
