// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Subset_Xor_Square_Range.cpp
// 参考（Verify問題）: https://yukicoder.me/problems/no/3792
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subset_xor_square_range.md
//
// API一覧
// SubsetXorSquareRange<Bits,Mod>(values): 非負32ビット整数列の部分集合XORを集計。構築O(Bits*n)。
//   Bitsは1以上32以下（既定30）、すべての値は2^Bits未満。Modは正のuint32_t（既定998244353）。
// set(index,value): 要素を置き換える。O(Bits log(n+1))。
// sum_squares(left,right): 半開区間内の添字を選ぶ全ての部分集合についてXORの2乗和をmod Modで返す。
//   同じ値でも添字を区別する。空区間の答えは0。O(Bits log(n+1))。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSET_XOR_SQUARE_RANGE
#define SIRKIT_SUBSET_XOR_SQUARE_RANGE

template <int Bits = 30, std::uint32_t Mod = 998244353>
class SubsetXorSquareRange
{
    static_assert(1 <= Bits && Bits <= 32);
    static_assert(Mod != 0);
    static constexpr auto kAll = std::numeric_limits<std::uint32_t>::max() >> (32 - Bits);
    struct Columns
    {
        std::array<std::uint32_t, Bits> equivalent;
        std::uint32_t active;
    };

    // 異なる2本の非零列はF2で一次独立。同じ列のビットは、常に同時に立つ。
    // 等値関係の共通部分と非零列の和集合だけで、結合後の2次モーメントが定まる。
    [[nodiscard]] static Columns join(const Columns &left, const Columns &right)
    {
        Columns result;
        for (int j = 0; j < Bits; ++j)
        {
            result.equivalent[j] = left.equivalent[j] & right.equivalent[j];
        }
        result.active = left.active | right.active;
        return result;
    }

    [[nodiscard]] static Columns singleton(std::uint32_t value)
    {
        if ((value & ~kAll) != 0)
        {
            throw std::invalid_argument("SubsetXorSquareRange: value outside configured bit width");
        }
        Columns result;
        for (int j = 0; j < Bits; ++j)
        {
            result.equivalent[j] = ((value >> j) & 1U) != 0 ? value : kAll ^ value;
        }
        result.active = value;
        return result;
    }

    [[nodiscard]] static constexpr std::uint32_t square(std::uint32_t value)
    {
        return static_cast<std::uint32_t>(std::uint64_t{value} * value % Mod);
    }

    std::size_t size_;
    std::vector<Columns> tree_;
    std::vector<std::uint32_t> two_powers_;

public:
    explicit SubsetXorSquareRange(std::span<const std::uint32_t> values)
        : size_(values.size()), tree_(2 * size_), two_powers_(size_ + 1)
    {
        two_powers_[0] = 1 % Mod;
        for (std::size_t index = 0; index < size_; ++index)
        {
            tree_[size_ + index] = singleton(values[index]);
            two_powers_[index + 1] = static_cast<std::uint32_t>(std::uint64_t{two_powers_[index]} * 2 % Mod);
        }
        for (std::size_t node = size_; node > 1;)
        {
            --node;
            tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
        }
    }

    void set(std::size_t index, std::uint32_t value)
    {
        if (index >= size_)
        {
            throw std::out_of_range("SubsetXorSquareRange::set");
        }
        auto node = index + size_;
        tree_[node] = singleton(value);
        while (node > 1)
        {
            node /= 2;
            tree_[node] = join(tree_[2 * node], tree_[2 * node + 1]);
        }
    }

    [[nodiscard]] std::uint32_t sum_squares(std::size_t left, std::size_t right) const
    {
        if (left > right || right > size_)
        {
            throw std::out_of_range("SubsetXorSquareRange::sum_squares");
        }
        const auto length = right - left;
        if (length == 0)
        {
            return 0;
        }
        if (length == 1)
        {
            return square(tree_[size_ + left].active);
        }
        Columns columns;
        columns.equivalent.fill(kAll);
        columns.active = 0;
        for (left += size_, right += size_; left < right; left /= 2, right /= 2)
        {
            if ((left & 1U) != 0)
            {
                columns = join(columns, tree_[left++]);
            }
            if ((right & 1U) != 0)
            {
                columns = join(columns, tree_[--right]);
            }
        }
        std::uint64_t sum = square(columns.active);
        for (auto remaining = columns.active; remaining != 0;)
        {
            const auto group = columns.equivalent[std::countr_zero(remaining)];
            sum += square(group);
            remaining ^= group;
        }
        return static_cast<std::uint32_t>((sum % Mod) * two_powers_[length - 2] % Mod);
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
