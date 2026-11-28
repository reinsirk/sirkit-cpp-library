// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Subarray_Aggregate_Runs.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subarray_aggregate_runs.md
//
// API一覧
// for_each_subarray_aggregate_run(input, combine, visit, equal={}):
// 各右端rについて、区間[l,r]の集約値が等しい左端lを、左端順のランにまとめてvisit(r,runs)へ渡す。
// SubarrayAggregateRun<T>のvalueは代表値。first,lastは左端の範囲[first,last)。
// combineは結合的。equalは集約と両立する同値関係（既定は==）。runsはvisitの実行中だけ参照可能。
// 右端rごとのラン数をK_rとすると、O(n+ΣK_r)回の演算・比較、O(max K_r)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <functional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBARRAY_AGGREGATE_RUNS_DEFINED
#define SIRKIT_SUBARRAY_AGGREGATE_RUNS_DEFINED

template <class T>
struct SubarrayAggregateRun
{
    T value;
    std::size_t first;
    std::size_t last;
};

template <std::ranges::input_range Range, class Combine, class Visit, class Equal = std::equal_to<>>
void for_each_subarray_aggregate_run(Range &&input, Combine combine, Visit visit, Equal equal = {})
{
    using T = std::ranges::range_value_t<Range>;
    using Run = SubarrayAggregateRun<T>;
    std::vector<Run> runs;
    std::vector<Run> next;
    auto append = [&](T value, std::size_t first, std::size_t last)
    {
        if (!next.empty() && std::invoke(equal, std::as_const(next.back().value), std::as_const(value)))
        {
            next.back().last = last;
        }
        else
        {
            next.push_back({std::move(value), first, last});
        }
    };
    std::size_t right = 0;
    for (auto &&element : input)
    {
        T value(element);
        next.clear();
        for (const Run &run : runs)
        {
            append(std::invoke(combine, run.value, std::as_const(value)), run.first, run.last);
        }
        append(std::move(value), right, right + 1);
        runs.swap(next);
        std::invoke(visit, right, std::span<const Run>(runs));
        ++right;
    }
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
