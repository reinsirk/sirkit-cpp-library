// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Static_Range_Mode.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_range_mode.md
//
// API一覧
// StaticRangeMode<T>(values): 長さnの静的な列を前計算する。O(1+n√n)時間・O(n+1)領域。
// query(l,r): 半開区間[l,r)の最頻値1つと出現回数のpairを返す。空区間はnullopt。O(√n+1)。
// size(): 要素数を返す。O(1)。添字は0始まり、n<=INT_MAX。Tは全順序付きでコピー可能。
// 前計算後は、照会の一覧を用意せず1件ずつ答えられる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RANGE_MODE_DEFINED
#define SIRKIT_STATIC_RANGE_MODE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckStaticRangeModeSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("StaticRangeMode: too many values");
    }
}
}

template <std::totally_ordered T = std::int64_t>
    requires std::copyable<T>
class StaticRangeMode
{
public:
    explicit StaticRangeMode(std::span<const T> input = {})
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckStaticRangeModeSize(input.size());
        if (input.empty())
        {
            return;
        }

        alphabet_.assign(input.begin(), input.end());
        std::sort(alphabet_.begin(), alphabet_.end());
        alphabet_.erase(std::unique(alphabet_.begin(), alphabet_.end()), alphabet_.end());

        ids_.resize(input.size());
        offsets_.resize(alphabet_.size() + 1);
        for (std::size_t i = 0; i < input.size(); ++i)
        {
            const auto id = std::lower_bound(alphabet_.begin(), alphabet_.end(), input[i]) - alphabet_.begin();
            ids_[i] = static_cast<int>(id);
            ++offsets_[id + 1];
        }
        std::partial_sum(offsets_.begin(), offsets_.end(), offsets_.begin());
        positions_.resize(input.size());
        ranks_.resize(input.size());
        {
            auto next = offsets_;
            for (int i = 0; i < size(); ++i)
            {
                const auto rank = next[ids_[i]]++;
                ranks_[i] = rank;
                positions_[rank] = i;
            }
        }
        BuildBlockModes();
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(ids_.size());
    }

    [[nodiscard]] std::optional<std::pair<T, int>> query(int left, int right) const
    {
        if (left == right)
        {
            return std::nullopt;
        }

        const int first_block = left / block_size_ + (left % block_size_ != 0);
        const int last_block = right / block_size_;
        Candidate best{};
        int left_end = right;
        int right_begin = right;
        if (first_block < last_block)
        {
            best = block_modes_[TableIndex(first_block, last_block - 1)];
            left_end = first_block * block_size_;
            right_begin = last_block * block_size_;
        }

        // 左端の候補は右へ、右端の候補は左へ数える。
        // bestより1回多く出るかだけを調べ、見つかった分だけしきい値を上げる。
        for (int i = left; i < left_end; ++i)
        {
            const auto id = ids_[i];
            const auto rank = ranks_[i];
            const auto remaining = offsets_[id + 1] - rank;
            while (best.count < remaining && positions_[rank + best.count] < right)
            {
                best = {id, best.count + 1};
            }
        }
        for (int i = right; i > right_begin;)
        {
            --i;
            const auto id = ids_[i];
            const auto rank = ranks_[i];
            const auto preceding = rank - offsets_[id];
            while (best.count <= preceding && positions_[rank - best.count] >= left)
            {
                best = {id, best.count + 1};
            }
        }
        return std::pair<T, int>{alphabet_[best.id], best.count};
    }

private:
    struct Candidate
    {
        int id = 0, count = 0;
    };

    std::vector<T> alphabet_;
    std::vector<int> ids_, offsets_, positions_, ranks_;
    std::vector<Candidate> block_modes_;
    int block_size_ = 1, blocks_ = 0;

    [[nodiscard]] std::size_t TableIndex(int first, int last) const noexcept
    {
        return static_cast<std::size_t>(first) * blocks_ + last;
    }

    void BuildBlockModes()
    {
        block_size_ = static_cast<int>(std::sqrt(size())) + 1;
        blocks_ = size() / block_size_;
        block_modes_.resize(static_cast<std::size_t>(blocks_) * blocks_);
        std::vector<int> counts(alphabet_.size());
        for (int first = 0; first < blocks_; ++first)
        {
            std::fill(counts.begin(), counts.end(), 0);
            Candidate best{};
            int i = first * block_size_;
            for (int last = first; last < blocks_; ++last)
            {
                const int end = (last + 1) * block_size_;
                for (; i < end; ++i)
                {
                    const auto id = ids_[i];
                    const auto count = ++counts[id];
                    if (count > best.count)
                    {
                        best = {id, count};
                    }
                }
                block_modes_[TableIndex(first, last)] = best;
            }
        }
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
