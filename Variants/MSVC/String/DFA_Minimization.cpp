// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: String/DFA_Minimization.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dfa_minimization.md
//
// API一覧
// minimize_dfa(alphabet, transitions, accepting, start=0): 到達可能な部分を同じ言語の最小DFAに変換。
// 元の状態数N>=1、文字種数A>=0。遷移は transitions[state*A+symbol]、受理フラグは0以外で受理。
// 戻り値の初期状態は0。state_map[v]は元の状態vの変換先、到達不能はkUnreachable。
// DfaMinimization::size()/alphabet()/transition(v,c)/accepting(v)はO(1)、state_map()はconst span。
// O(N*(1+A)*(1+log N))時間、O(N*(1+A))領域。遷移は全状態・全文字に対して定義する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DFA_MINIMIZATION_DEFINED
#define SIRKIT_DFA_MINIMIZATION_DEFINED

class DfaMinimization
{
    friend DfaMinimization minimize_dfa(std::size_t, std::span<const std::size_t>,
                                        std::span<const std::uint8_t>, std::size_t);
    std::size_t alphabet_ = 0;
    std::vector<std::size_t> transitions_, state_map_;
    std::vector<std::uint8_t> accepting_;

public:
    static constexpr std::size_t kUnreachable = std::numeric_limits<std::size_t>::max();

    [[nodiscard]] std::size_t size() const noexcept
    {
        return accepting_.size();
    }

    [[nodiscard]] std::size_t alphabet() const noexcept
    {
        return alphabet_;
    }

    [[nodiscard]] std::span<const std::size_t> state_map() const noexcept
    {
        return state_map_;
    }

    [[nodiscard]] std::size_t transition(std::size_t state, std::size_t symbol) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (state >= size() || symbol >= alphabet_)
        {
            throw std::out_of_range("DfaMinimization::transition: state or symbol");
        }
#endif
        return transitions_[state * alphabet_ + symbol];
    }

    [[nodiscard]] bool accepting(std::size_t state) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (state >= size())
        {
            throw std::out_of_range("DfaMinimization::accepting: state");
        }
#endif
        return accepting_[state] != 0;
    }
};

namespace detail
{
struct DfaClassRange
{
    std::size_t first, last;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return last - first;
    }
};

struct DfaReverseColumn
{
    std::vector<std::size_t> offsets, vertices;
};

inline std::vector<DfaReverseColumn> ReverseReachableDfa(
    std::size_t alphabet, std::span<const std::size_t> transitions,
    std::span<const std::size_t> reachable, std::span<const std::size_t> index)
{
    const auto n = reachable.size();
    std::vector<DfaReverseColumn> reverse(alphabet);
    for (std::size_t c = 0; c < alphabet; ++c)
    {
        auto &[offsets, vertices] = reverse[c];
        offsets.resize(n + 1);
        vertices.resize(n);
        for (const auto v : reachable)
        {
            ++offsets[index[transitions[v * alphabet + c]] + 1];
        }
        std::partial_sum(offsets.begin(), offsets.end(), offsets.begin());
        std::vector<std::size_t> cursor(offsets);
        for (std::size_t i = 0; i < n; ++i)
        {
            const auto next = index[transitions[reachable[i] * alphabet + c]];
            vertices[cursor[next]++] = i;
        }
    }
    return reverse;
}

inline void RefineDfaPartition(std::span<const DfaReverseColumn> reverse,
                               std::vector<std::size_t> &order, std::vector<std::size_t> &owner,
                               std::vector<DfaClassRange> &groups)
{
    const auto n = order.size();
    std::vector<std::size_t> position(n), marked(n), from, affected;
    for (std::size_t i = 0; i < n; ++i)
    {
        position[order[i]] = i;
    }
    std::vector<std::pair<std::size_t, std::size_t>> pending;
    const auto schedule = [&](std::size_t group)
    {
        for (std::size_t c = 0; c < reverse.size(); ++c)
        {
            pending.emplace_back(group, c);
        }
    };
    schedule(groups[0].size() <= groups[1].size() ? 0 : 1);
    while (!pending.empty())
    {
        const auto [splitter, symbol] = pending.back();
        pending.pop_back();
        const auto &[offsets, vertices] = reverse[symbol];
        const auto [begin, end] = groups[splitter];

        // 区間を並べ替える前に、分割に使う前駆集合を確定する。
        from.clear();
        for (std::size_t i = begin; i < end; ++i)
        {
            const auto v = order[i];
            from.insert(from.end(), vertices.begin() + offsets[v], vertices.begin() + offsets[v + 1]);
        }

        affected.clear();
        for (const auto v : from)
        {
            const auto id = owner[v];
            if (marked[id] == 0)
            {
                affected.push_back(id);
            }
            const auto destination = groups[id].first + marked[id]++;
            const auto other = order[destination];
            order[position[v]] = other;
            position[other] = position[v];
            order[destination] = v;
            position[v] = destination;
        }

        for (const auto id : affected)
        {
            const auto count = std::exchange(marked[id], 0);
            const auto old = groups[id];
            if (count == old.size())
            {
                continue;
            }
            const auto cut = old.first + count;
            DfaClassRange smaller{old.first, cut}, larger{cut, old.last};
            if (smaller.size() > larger.size())
            {
                std::swap(smaller, larger);
            }
            const auto added = groups.size();
            groups[id] = larger;
            groups.push_back(smaller);
            for (std::size_t i = smaller.first; i < smaller.last; ++i)
            {
                owner[order[i]] = added;
            }
            // 既存の予定は大きい方を引き継ぎ、小さい方だけ新しく予定に加える。
            schedule(added);
        }
    }
}
}

[[nodiscard]] inline DfaMinimization minimize_dfa(std::size_t alphabet, std::span<const std::size_t> transitions,
                                                  std::span<const std::uint8_t> accepting, std::size_t start = 0)
{
    const auto states = accepting.size();
#ifndef SKIP_RUNTIME_CHECKS
    if (states == 0 || start >= states || transitions.size() / states != alphabet || transitions.size() % states != 0)
    {
        throw std::invalid_argument("minimize_dfa: state count, start, or transition table dimensions");
    }
    if (std::any_of(transitions.begin(), transitions.end(), [states](auto target)
                    {
                        return target >= states;
                    }))
    {
        throw std::out_of_range("minimize_dfa: transition target");
    }
#endif

    constexpr auto missing = DfaMinimization::kUnreachable;
    std::vector<std::size_t> index(states, missing), reachable{start};
    index[start] = 0;
    for (std::size_t i = 0; i < reachable.size(); ++i)
    {
        const auto row = reachable[i] * alphabet;
        for (std::size_t c = 0; c < alphabet; ++c)
        {
            const auto to = transitions[row + c];
            if (index[to] == missing)
            {
                index[to] = reachable.size();
                reachable.push_back(to);
            }
        }
    }

    const auto n = reachable.size();
    std::vector<std::size_t> order(n), owner(n);
    std::iota(order.begin(), order.end(), std::size_t{0});
    const auto middle = std::partition(order.begin(), order.end(),
                                       [&](auto i)
                                       {
                                           return accepting[reachable[i]] != 0;
                                       });
    const auto cut = static_cast<std::size_t>(middle - order.begin());
    std::vector<detail::DfaClassRange> groups;
    if (cut == 0 || cut == n)
    {
        groups.push_back({0, n});
    }
    else
    {
        groups = {{0, cut}, {cut, n}};
        for (auto it = middle; it != order.end(); ++it)
        {
            owner[*it] = 1;
        }
        const auto reverse = detail::ReverseReachableDfa(alphabet, transitions, reachable, index);
        detail::RefineDfaPartition(reverse, order, owner, groups);
    }

    std::vector<std::size_t> renumber(groups.size());
    const auto start_group = owner[0];
    std::iota(renumber.begin(), renumber.end(), std::size_t{0});
    std::swap(renumber[0], renumber[start_group]);
    DfaMinimization result;
    result.alphabet_ = alphabet;
    result.state_map_.assign(states, missing);
    result.accepting_.resize(groups.size());
    result.transitions_.resize(groups.size() * alphabet);
    for (std::size_t i = 0; i < n; ++i)
    {
        result.state_map_[reachable[i]] = renumber[owner[i]];
    }
    for (std::size_t id = 0; id < groups.size(); ++id)
    {
        const auto v = reachable[order[groups[id].first]];
        result.accepting_[renumber[id]] = accepting[v] != 0;
        for (std::size_t c = 0; c < alphabet; ++c)
        {
            const auto to = transitions[v * alphabet + c];
            result.transitions_[renumber[id] * alphabet + c] = result.state_map_[to];
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
