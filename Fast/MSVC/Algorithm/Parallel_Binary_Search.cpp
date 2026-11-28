// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Parallel_Binary_Search.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/parallel_binary_search.md
//
// API一覧
// parallel_binary_search(event_count,query_count,reset,apply,predicate): 各照会が初めてtrueになる処理済みイベント数を返す。
// 同じ照会はイベント数に対しfalseからtrueに一度だけ切り替わる。初期状態でtrueなら0、最後までfalseならevent_count+1。
// reset()は状態の初期化、apply(i)は0始まりのi番目のイベント、predicate(q)は照会qの条件判定。
// O((M+Q+1)log(M+2))時間・O(M+Q+1)領域。Mはイベント数、Qは照会数、コールバック時間は別。
// reset()はO(log(M+2))回。applyとpredicateは各O(M log(M+2))、O(Q log(M+2))回。照会順は保証しない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PARALLEL_BINARY_SEARCH_DEFINED
#define SIRKIT_PARALLEL_BINARY_SEARCH_DEFINED

template<class Reset, class Apply, class Predicate>
std::vector<std::size_t> parallel_binary_search(std::size_t event_count, std::size_t query_count,
                                                Reset &&reset, Apply &&apply, Predicate &&predicate)
{
    if (query_count == 0)
    {
        return {};
    }
    if (event_count == std::numeric_limits<std::size_t>::max())
    {
        throw std::length_error("parallel_binary_search: too many events");
    }
    const auto never = event_count + 1;
    std::vector<std::size_t> lower(query_count);
    std::vector<std::size_t> upper(query_count, never);
    // 各時点の照会を、可変長配列の追加確保なしで列挙する。
    std::vector<std::size_t> head(never);
    std::vector<std::size_t> next(query_count);
    for (;;)
    {
        std::fill(head.begin(), head.end(), query_count);
        bool pending = false;
        std::size_t last = 0;
        for (std::size_t query = 0; query < query_count; ++query)
        {
            if (lower[query] == upper[query])
            {
                continue;
            }
            pending = true;
            const auto middle = std::midpoint(lower[query], upper[query]);
            next[query] = head[middle];
            head[middle] = query;
            last = std::max(last, middle);
        }
        if (!pending)
        {
            return upper;
        }

        reset();
        for (std::size_t done = 0; done <= last; ++done)
        {
            if (done != 0)
            {
                apply(done - 1);
            }
            for (auto query = head[done]; query != query_count; query = next[query])
            {
                if (predicate(query))
                {
                    upper[query] = done;
                }
                else
                {
                    lower[query] = done + 1;
                }
            }
        }
    }
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
