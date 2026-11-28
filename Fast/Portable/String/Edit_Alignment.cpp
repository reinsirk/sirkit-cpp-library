// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Edit_Alignment.cpp
// 参考（線形領域での分割復元）: https://doi.org/10.1145/360825.360861
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/edit_alignment.md
//
// API一覧
// edit_alignment(source,target,equal={}): spanまたはstring_viewを受け取り、最小編集回数と操作列を返す。
// spanではequal(a,b)で元の値aと先の値bの一致を判定する。既定比較は==。
// EditAlignment{distance,steps}: distanceは挿入・削除・置換の合計、stepsは先頭から処理する操作列。
// EditAction::kMatch/kReplaceは両列、kEraseは元の列、kInsertは先の列を1要素進める。
// n,mを長さとしてO(n*m+n+m+1)時間、結果を除きO(1+min(n,m)+log(max(n,m)+1))領域。
// 空の列にも対応。同じコストの操作列が複数あればいずれかを返す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EDIT_ALIGNMENT_DEFINED
#define SIRKIT_EDIT_ALIGNMENT_DEFINED

enum class EditAction : unsigned char
{
    kMatch,
    kReplace,
    kErase,
    kInsert,
};

struct EditAlignment
{
    std::size_t distance = 0;
    std::vector<EditAction> steps;
};

namespace detail
{
inline std::size_t checked_edit_alignment_size(std::size_t source, std::size_t target, std::size_t capacity)
{
    if (source > capacity || target > capacity - source)
    {
        throw std::length_error("edit_alignment: operation capacity exceeded");
    }
    return source + target;
}

template <bool Reverse, class A, class B, class Equal>
std::vector<std::size_t> edit_alignment_cost_row(std::span<const A> source, std::span<const B> target, Equal &equal)
{
    std::vector<std::size_t> length(target.size() + 1);
    std::iota(length.begin(), length.end(), std::size_t{0});
    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const auto &a = source[Reverse ? source.size() - 1 - i : i];
        auto diagonal = length[0]++;
        for (std::size_t j = 0; j < target.size(); ++j)
        {
            const auto &b = target[Reverse ? target.size() - 1 - j : j];
            const auto previous = length[j + 1];
            length[j + 1] = std::min({length[j] + 1, previous + 1, diagonal + !std::invoke(equal, a, b)});
            diagonal = previous;
        }
    }
    return length;
}

template <class A, class B, class Equal>
std::size_t edit_alignment_split(std::span<const A> left, std::span<const A> right, std::span<const B> target,
                                 Equal &equal)
{
    const auto start = edit_alignment_cost_row<false>(left, target, equal);
    const auto end = edit_alignment_cost_row<true>(right, target, equal);
    std::size_t split = 0;
    auto best = start[0] + end[target.size()];
    for (std::size_t j = 1; j <= target.size(); ++j)
    {
        const auto cost = start[j] + end[target.size() - j];
        if (cost < best)
        {
            best = cost;
            split = j;
        }
    }
    return split;
}

template <class A, class B, class Equal>
void append_edit_alignment(std::span<const A> a, std::span<const B> b, Equal &equal, EditAlignment &result,
                           EditAction erase, EditAction insert)
{
    const auto append = [&](EditAction kind, std::size_t count)
    {
        result.steps.insert(result.steps.end(), count, kind);
        if (kind != EditAction::kMatch)
        {
            result.distance += count;
        }
    };
    if (b.empty())
    {
        append(erase, a.size());
        return;
    }
    if (a.size() == 1)
    {
        const auto found = std::ranges::find_if(b, [&](const B &value)
                                                {
                                                    return std::invoke(equal, a.front(), value);
                                                });
        if (found == b.end())
        {
            append(EditAction::kReplace, 1);
            append(insert, b.size() - 1);
        }
        else
        {
            const auto position = static_cast<std::size_t>(found - b.begin());
            append(insert, position);
            append(EditAction::kMatch, 1);
            append(insert, b.size() - 1 - position);
        }
        return;
    }
    const auto middle = a.size() / 2;
    const auto left = a.first(middle), right = a.subspan(middle);
    const auto split = edit_alignment_split(left, right, b, equal);
    append_edit_alignment(left, b.first(split), equal, result, erase, insert);
    append_edit_alignment(right, b.subspan(split), equal, result, erase, insert);
}
}

template <class A, std::size_t NX, class B, std::size_t NY, class Equal = std::equal_to<>>
[[nodiscard]] EditAlignment edit_alignment(std::span<A, NX> source, std::span<B, NY> target, Equal equal = {})
{
    const std::span<const std::remove_const_t<A>> a = source;
    const std::span<const std::remove_const_t<B>> b = target;
    EditAlignment result;
    result.steps.reserve(detail::checked_edit_alignment_size(a.size(), b.size(), result.steps.max_size()));
    if (a.size() >= b.size())
    {
        detail::append_edit_alignment(a, b, equal, result, EditAction::kErase, EditAction::kInsert);
    }
    else
    {
        const auto reverse = [&](const B &right, const A &left)
        {
            return std::invoke(equal, left, right);
        };
        detail::append_edit_alignment(b, a, reverse, result, EditAction::kInsert, EditAction::kErase);
    }
    return result;
}

[[nodiscard]] inline EditAlignment edit_alignment(std::string_view source, std::string_view target)
{
    return edit_alignment(std::span<const char>(source.data(), source.size()),
                          std::span<const char>(target.data(), target.size()));
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
