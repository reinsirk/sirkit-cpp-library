// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Eytzinger_Search.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/eytzinger_search.md
// 参考（幅優先順の配列配置）: https://arxiv.org/abs/1509.05053
//
// API一覧
// EytzingerSearch<T,Compare=less<>>(values[,compare]): 値を整列し、固定列の境界探索を準備する。O(n log(n+1))時間・O(n)領域。
// EytzingerSearch<T,Compare>::from_sorted(values[,compare]): 整列済みの列から準備する。O(n)時間・領域。
// size(): 要素数。O(1)。境界の戻り値は整列後の0始まり添字で、該当しなければsize()。
// lower_bound(key)/upper_bound(key): 順序で最初のkey以上/keyより後の位置を返す。O(log(n+1))。
// equal_range(key): 互いに同順位の要素を含む半開区間を返す。O(log(n+1))。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EYTZINGER_SEARCH_DEFINED
#define SIRKIT_EYTZINGER_SEARCH_DEFINED

template <class T, class Compare = std::less<>>
class EytzingerSearch
{
public:
    explicit EytzingerSearch(std::vector<T> values, Compare compare = {})
        : compare_(std::move(compare))
    {
        std::sort(values.begin(), values.end(), std::ref(compare_));
        build(values);
    }

    [[nodiscard]] static EytzingerSearch from_sorted(std::vector<T> values, Compare compare = {})
    {
        EytzingerSearch result(std::move(compare));
        result.build(values);
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    template <class Key>
    [[nodiscard]] std::size_t lower_bound(const Key &key) const
    {
        return boundary<false>(key);
    }

    template <class Key>
    [[nodiscard]] std::size_t upper_bound(const Key &key) const
    {
        return boundary<true>(key);
    }

    template <class Key>
    [[nodiscard]] std::pair<std::size_t, std::size_t> equal_range(const Key &key) const
    {
        return {lower_bound(key), upper_bound(key)};
    }

private:
    explicit EytzingerSearch(Compare compare) : compare_(std::move(compare))
    {
    }

    void position_ranks(std::size_t node, std::size_t &next)
    {
        if (node > ranks_.size())
        {
            return;
        }
        position_ranks(2 * node, next);
        ranks_[node - 1] = next++;
        position_ranks(2 * node + 1, next);
    }

    void build(std::vector<T> &sorted)
    {
        ranks_.resize(sorted.size());
        values_.reserve(sorted.size());
        std::size_t next = 0;
        position_ranks(1, next);
        for (const auto rank : ranks_)
        {
            values_.push_back(std::move(sorted[rank]));
        }
    }

    // 探索後に末尾の右分岐を捨てると、最後に左へ進んだ節点が残る。
    template <bool Upper, class Key>
    [[nodiscard]] std::size_t boundary(const Key &key) const
    {
        std::size_t node = 1;
        while (node <= size())
        {
            if constexpr (Upper)
            {
                node = 2 * node + !std::invoke(compare_, key, values_[node - 1]);
            }
            else
            {
                node = 2 * node + std::invoke(compare_, values_[node - 1], key);
            }
        }
        node = (node >> std::countr_one(node)) >> 1;
        return node == 0 ? size() : ranks_[node - 1];
    }

    Compare compare_;
    std::vector<std::size_t> ranks_;
    std::vector<T> values_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
