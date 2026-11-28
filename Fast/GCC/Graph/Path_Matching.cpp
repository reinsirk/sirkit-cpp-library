// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Path_Matching.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/path_matching.md
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/matching_on_path
//
// API一覧
// PathMatching<T>(weights): パスの辺 (i,i+1) の重み weights[i] を順に入力。
//   ちょうど k 本の互いに接しない辺を選ぶ最大重みを、すべての k について前処理。
//   重みは負・ゼロも可。T は 0・加減算・順序を備え、途中の交代和や解の総和を表現できること。
//   辺数 m に対して O((m+1) log(m+2)) 時間、O(m+1) 領域。
// max_cardinality()/best(k)/values(): 最大本数/ちょうど k 本の最大重み/本数順の const span。O(1)。
//   values()[0] は 0。空の辺列にも対応。不正な k は例外。
// edge_indices(k): ちょうど k 本で best(k) を達成する 0 始まりの辺番号を昇順に返す。O(m+1) 時間・領域。
// maximum_path_matching_weight<T>(weights,k): 1 つの k での最大重み。平均 O(m+1) 時間、O(m+1) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PATH_MATCHING_DEFINED
#define SIRKIT_PATH_MATCHING_DEFINED

namespace detail
{
template <class T>
struct PathMatchingChange
{
    T gain;
    std::size_t first, end, serial;
};

template <class T>
bool path_gain_descending(const PathMatchingChange<T> &a, const PathMatchingChange<T> &b)
{
    if (b.gain < a.gain)
    {
        return true;
    }
    return !(a.gain < b.gain) && a.serial < b.serial;
}

template <class T>
std::vector<PathMatchingChange<T>> path_matching_changes(std::span<const T> weights)
{
    const auto n = weights.size() + 1;
    std::vector<T> potential(n, T{0});
    for (std::size_t i = 0; i < weights.size(); ++i)
    {
        potential[i + 1] = weights[i] - potential[i];
    }

    std::vector<std::size_t> stack;
    stack.reserve(n);
    std::vector<PathMatchingChange<T>> changes;
    changes.reserve(n / 2);
    // スタック内の候補値は左から狭義単調増加。右の新候補より小さくなるまで末尾の局所最大を取り出す。
    for (std::size_t next = 0; next < n; ++next)
    {
        while (stack.size() >= 2)
        {
            const auto size = stack.size();
            const auto first = stack[size - 2];
            const auto second = stack[size - 1];
            if (potential[first] < potential[next])
            {
                break;
            }
            changes.push_back({potential[first] + potential[second], first, second, changes.size()});
            stack.resize(size - 2);
        }
        stack.push_back(next);
    }
    for (auto size = stack.size(); size >= 2; size -= 2)
    {
        const auto first = stack[size - 2];
        const auto second = stack[size - 1];
        changes.push_back({potential[first] + potential[second], first, second, changes.size()});
    }
    return changes;
}
} // namespace detail

template <class T>
class PathMatching
{
public:
    explicit PathMatching(std::span<const T> weights)
        : edge_count_(weights.size()), changes_(detail::path_matching_changes(weights)), values_(1, T{0})
    {
        std::sort(changes_.begin(), changes_.end(), detail::path_gain_descending<T>);
        values_.reserve(changes_.size() + 1);
        for (const auto &change : changes_)
        {
            values_.push_back(values_.back() + change.gain);
        }
    }

    [[nodiscard]] std::size_t max_cardinality() const
    {
        return changes_.size();
    }

    [[nodiscard]] std::span<const T> values() const
    {
        return values_;
    }

    [[nodiscard]] T best(std::size_t k) const
    {
        return values_.at(k);
    }

    [[nodiscard]] std::vector<std::size_t> edge_indices(std::size_t k) const
    {
        if (k > max_cardinality())
        {
            throw std::out_of_range("PathMatching: cardinality");
        }
        if (k == 0)
        {
            return {};
        }

        std::vector<unsigned char> boundaries(edge_count_ + 1);
        for (std::size_t i = 0; i < k; ++i)
        {
            boundaries[changes_[i].first] ^= 1U;
            boundaries[changes_[i].end] ^= 1U;
        }
        std::vector<std::size_t> selected;
        selected.reserve(k);
        unsigned char flipped = 0;
        for (std::size_t edge = 0; edge < edge_count_; ++edge)
        {
            flipped ^= boundaries[edge];
            if (flipped != 0)
            {
                selected.push_back(edge);
            }
        }
        return selected;
    }

private:
    std::size_t edge_count_;
    std::vector<detail::PathMatchingChange<T>> changes_;
    std::vector<T> values_;
};

template <class T>
[[nodiscard]] T maximum_path_matching_weight(std::span<const T> weights, std::size_t k)
{
    const auto max_cardinality = (weights.size() + 1) / 2;
    if (k > max_cardinality)
    {
        throw std::out_of_range("maximum_path_matching_weight: cardinality");
    }
    if (k == 0)
    {
        return T{0};
    }
    auto changes = detail::path_matching_changes(weights);
    if (k < max_cardinality)
    {
        std::nth_element(changes.begin(), changes.begin() + k, changes.end(), detail::path_gain_descending<T>);
    }
    T total{0};
    for (std::size_t i = 0; i < k; ++i)
    {
        total = total + changes[i].gain;
    }
    return total;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
