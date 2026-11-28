// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: DataStructure/Static_Range_Minimum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_range_minimum.md
//
// API一覧
// StaticRangeMinimum<T,Compare>(values,compare={}): 更新のない配列を前処理。O(n+1)時間・領域。
// Compare(a,b)がtrueならaを選ぶ。既定はstd::less<>。比較は狭義弱順序で、比較演算をO(1)とする。
// size(): 配列長。argmin(l,r): 半開区間[l,r)の最小値の最初の添字。min(l,r): 最小値への参照（T=boolは値）。
// 0<=l<r<=size()。size、argmin、minはO(1)。コピーは前処理結果を共有する。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RANGE_MINIMUM_DEFINED
#define SIRKIT_STATIC_RANGE_MINIMUM_DEFINED

template<class T, class Compare = std::less<>>
class StaticRangeMinimum
{
    struct Data
    {
        std::vector<T> values;
        [[no_unique_address]] Compare compare;
        std::size_t block_width;
        std::vector<std::size_t> masks;
        std::vector<std::vector<std::size_t>> block_table;

        Data(std::vector<T> input, Compare order)
            : values(std::move(input)),
              compare(std::move(order)),
              block_width(std::max(std::size_t{1}, static_cast<std::size_t>(std::bit_width(values.size())))),
              masks(values.size())
        {
            std::vector<std::size_t> blocks;
            for (std::size_t first = 0; first < values.size();)
            {
                const auto end = first + std::min(block_width, values.size() - first);
                std::size_t stack = 0;
                for (std::size_t i = first; i < end; ++i)
                {
                    // 単調スタックに残った位置をビットで保存する。同値の位置は残す。
                    while (stack != 0)
                    {
                        const auto top = std::bit_width(stack) - 1;
                        if (!compare(values[i], values[first + top]))
                        {
                            break;
                        }
                        stack ^= std::size_t{1} << top;
                    }
                    stack |= std::size_t{1} << (i - first);
                    masks[i] = stack;
                }
                blocks.push_back(first + std::countr_zero(stack));
                first = end;
            }

            const auto count = blocks.size();
            block_table.push_back(std::move(blocks));
            for (std::size_t width = 2; width <= count; width *= 2)
            {
                const auto &previous = block_table.back();
                std::vector<std::size_t> row(count - width + 1);
                for (std::size_t i = 0; i < row.size(); ++i)
                {
                    row[i] = earlier_min(previous[i], previous[i + width / 2]);
                }
                block_table.push_back(std::move(row));
            }
        }

        std::size_t earlier_min(std::size_t left, std::size_t right) const
        {
            return compare(values[right], values[left]) ? right : left;
        }

        std::size_t within_block(std::size_t first, std::size_t left, std::size_t right) const
        {
            const auto masked = masks[right - 1] & (~std::size_t{0} << (left - first));
            return first + std::countr_zero(masked);
        }

        std::size_t argmin(std::size_t left, std::size_t right) const
        {
            const auto begin_block = left / block_width;
            const auto end_block = (right - 1) / block_width;
            const auto begin_pos = begin_block * block_width;
            if (begin_block == end_block)
            {
                return within_block(begin_pos, left, right);
            }
            const auto end_pos = end_block * block_width;
            auto answer = within_block(begin_pos, left, begin_pos + block_width);
            if (begin_block + 1 < end_block)
            {
                const auto count = end_block - (begin_block + 1);
                const auto level = std::bit_width(count) - 1;
                const auto &row = block_table[level];
                const auto middle = earlier_min(row[begin_block + 1], row[end_block - (std::size_t{1} << level)]);
                answer = earlier_min(answer, middle);
            }
            return earlier_min(answer, within_block(end_pos, end_pos, right));
        }
    };

    std::shared_ptr<const Data> data_;

public:
    using const_reference = typename std::vector<T>::const_reference;

    explicit StaticRangeMinimum(std::vector<T> values, Compare compare = Compare{})
        : data_(std::make_shared<const Data>(std::move(values), std::move(compare)))
    {
    }

    StaticRangeMinimum(const StaticRangeMinimum &) noexcept = default;
    StaticRangeMinimum &operator=(const StaticRangeMinimum &) noexcept = default;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return data_->values.size();
    }

    [[nodiscard]] std::size_t argmin(std::size_t left, std::size_t right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left >= right || right > size())
        {
            throw std::out_of_range("StaticRangeMinimum: invalid interval");
        }
#endif
        return data_->argmin(left, right);
    }

    [[nodiscard]] const_reference min(std::size_t left, std::size_t right) const
    {
        return data_->values[argmin(left, right)];
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
