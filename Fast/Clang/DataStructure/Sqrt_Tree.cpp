// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Sqrt_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sqrt_tree.md
// 参考（平方分割と階層選択）: https://info.atcoder.jp/entry/algorithm_lectures/sqrt_tree
//
// API一覧
// SqrtTree<T,Op>(values,identity,op): 静的列を構築。Opはconst呼出可能な結合的演算、identityは単位元。
// n要素の構築はO(n log log(n+2)+1)時間・領域。演算・コピーをO(1)とする。
// size()/empty(): 元の要素数/空か。get(i): 元の要素。いずれもO(1)。添字は0始まり。
// prod(l,r)/all_prod(): 半開区間[l,r)/全列の積。空ならidentity。非可換でも可。O(1)で演算は高々2回。
// 添字・区間の範囲外はout_of_range。SKIP_RUNTIME_CHECKSで検査を除く。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SQRT_TREE_DEFINED
#define SIRKIT_SQRT_TREE_DEFINED

template <class T, class Op>
class SqrtTree
{
public:
    using const_reference = typename std::vector<T>::const_reference;

    explicit SqrtTree(std::vector<T> values = {}, T identity = T{}, Op op = Op{})
        : op_(std::move(op)), identity_(std::move(identity)), values_(std::move(values))
    {
        if (size() <= 2)
        {
            return;
        }
        for (auto bits = static_cast<std::size_t>(std::bit_width(size() - 1)); bits > 1;)
        {
            const auto block_bits = (bits + 1) / 2;
            for (auto bit = block_bits; bit < bits; ++bit)
            {
                layer_by_bit_[bit] = layers_.size();
            }
            layers_.emplace_back(values_, block_bits, bits - block_bits, op_);
            bits = block_bits;
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return values_.empty();
    }

    [[nodiscard]] const_reference get(std::size_t index) const
    {
        return values_[index];
    }

    [[nodiscard]] T prod(std::size_t left, std::size_t right) const
    {
        if (left == right)
        {
            return identity_;
        }
        const auto last = right - 1;
        if (left == last)
        {
            return values_[left];
        }

        const auto bit = static_cast<std::size_t>(std::bit_width(left ^ last) - 1);
        if (bit == 0)
        {
            return op_(values_[left], values_[last]);
        }
        const auto &layer = layers_[layer_by_bit_[bit]];
        const auto first_block = left >> layer.block_bits;
        const auto last_block = last >> layer.block_bits;
        if (first_block + 1 == last_block)
        {
            return op_(layer.boundary[layer.suffix_offset + left], layer.boundary[last]);
        }
        return op_(op_(layer.boundary[layer.suffix_offset + left], layer.block_product(first_block + 1, last_block - 1)), layer.boundary[last]);
    }

    [[nodiscard]] T all_prod() const
    {
        return prod(0, size());
    }

private:
    struct Layer
    {
        std::size_t block_bits;
        std::size_t group_bits;
        std::size_t blocks;
        std::size_t suffix_offset;
        std::vector<T> boundary;
        std::vector<T> between;

        Layer(const std::vector<T> &values, std::size_t block_shift, std::size_t group_shift, const Op &op)
            : block_bits(block_shift), group_bits(group_shift), blocks(((values.size() - 1) >> block_bits) + 1),
              suffix_offset(group_bits == 1 ? 0 : values.size())
        {
            boundary.reserve(values.size() + suffix_offset);
            boundary.insert(boundary.end(), values.begin(), values.end());
            if (suffix_offset != 0)
            {
                boundary.insert(boundary.end(), values.begin(), values.end());
            }
            const auto block_size = std::size_t{1} << block_bits;
            for (std::size_t first = 0; first < values.size();)
            {
                const auto end = first + std::min(block_size, values.size() - first);
                const bool right_child = ((first >> block_bits) & 1u) != 0;
                // 子が2個なら左の接尾辞と右の接頭辞だけで十分。
                if (suffix_offset != 0 || right_child)
                {
                    for (auto i = first + 1; i < end; ++i)
                    {
                        boundary[i] = op(boundary[i - 1], values[i]);
                    }
                }
                if (suffix_offset != 0 || !right_child)
                {
                    for (auto i = end - 1; i > first; --i)
                    {
                        boundary[suffix_offset + i - 1] = op(values[i - 1], boundary[suffix_offset + i]);
                    }
                }
                first = end;
            }

            const auto group_size = std::size_t{1} << group_bits;
            if (group_size <= 2)
            {
                return;
            }
            between.reserve((blocks >> group_bits) * triangle(group_size) + triangle(blocks & (group_size - 1)));
            for (std::size_t first = 0; first < blocks; first += group_size)
            {
                const auto width = std::min(group_size, blocks - first);
                for (std::size_t i = 0; i < width; ++i)
                {
                    T product = boundary[suffix_offset + ((first + i) << block_bits)];
                    between.push_back(product);
                    for (auto j = i + 1; j < width; ++j)
                    {
                        product = op(product, boundary[suffix_offset + ((first + j) << block_bits)]);
                        between.push_back(product);
                    }
                }
            }
        }

        [[nodiscard]] const_reference block_product(std::size_t first, std::size_t last) const
        {
            const auto group_size = std::size_t{1} << group_bits;
            const auto group = first >> group_bits;
            const auto width = std::min(group_size, blocks - (group << group_bits));
            const auto local = first & (group_size - 1);
            // 最終グループだけは子ブロック数が少ない。各グループの上三角だけを格納する。
            const auto offset = group * triangle(group_size) + triangle(width) - triangle(width - local);
            return between[offset + last - first];
        }

        [[nodiscard]] static constexpr std::size_t triangle(std::size_t count) noexcept
        {
            return count * (count + 1) / 2;
        }
    };

    [[no_unique_address]] Op op_;
    T identity_;
    std::vector<T> values_;
    std::vector<Layer> layers_;
    std::array<std::size_t, std::numeric_limits<std::size_t>::digits> layer_by_bit_{};
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
