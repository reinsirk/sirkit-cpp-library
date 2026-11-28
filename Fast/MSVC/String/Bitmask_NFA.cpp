// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Bitmask_NFA.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bitmask_nfa.md
//
// API一覧
// [BitmaskNfa<Alphabet>] 状態数・文字種数が各64以下の非決定性有限オートマトン。
// BitmaskNfa(states, initial=0): 状態数と初期状態を指定。O(states*Alphabet)。
// add_transition(from,symbol,to)/add_epsilon(from,to): 文字付き遷移/空文字遷移を追加。O(1)。
// set_accepting(state,accept=true): 受理状態を設定・解除。O(1)。
// determinize(max_states): 到達可能な状態だけを持つBitmaskDfaを生成。上限超過でlength_error。
// N=NFAの状態数、A=文字種数、D=到達可能なDFA状態数。平均O(N*A*(N+D))時間、O(A*(N+D))領域。
// [BitmaskDfa<Alphabet>] 初期状態は0。size()は状態数。accepts(word)はO(|word|)で受理判定。
// transition(state,symbol)/accepting(state): 遷移先/受理状態かどうか。O(1)。
// count_matches<T>(choices): 各位置の文字候補のビット集合から受理語の数を数える。O(D*(1+|choices|*A))時間、O(D)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_BITMASK_NFA_DEFINED
#define SIRKIT_BITMASK_NFA_DEFINED

template <std::size_t Alphabet>
class BitmaskNfa;

template <std::size_t Alphabet>
class BitmaskDfa
{
    static_assert(1 <= Alphabet && Alphabet <= 64);
    friend class BitmaskNfa<Alphabet>;

    struct Row
    {
        std::array<std::size_t, Alphabet> next;
        bool accepting;
    };

    explicit BitmaskDfa(std::vector<Row> rows)
        : rows_(std::move(rows))
    {
    }

    std::vector<Row> rows_;

public:
    using Mask = std::uint64_t;
    static constexpr Mask kAllSymbols = std::numeric_limits<Mask>::max() >> (64 - Alphabet);

    [[nodiscard]] std::size_t size() const noexcept
    {
        return rows_.size();
    }

    [[nodiscard]] std::size_t transition(std::size_t state, std::size_t symbol) const
    {
        return rows_[state].next[symbol];
    }

    [[nodiscard]] bool accepting(std::size_t state) const
    {
        return rows_[state].accepting;
    }

    [[nodiscard]] bool accepts(std::span<const std::size_t> word) const
    {
        if (rows_.empty())
        {
            return false;
        }
        std::size_t state = 0;
        for (const auto symbol : word)
        {
            state = rows_[state].next[symbol];
        }
        return rows_[state].accepting;
    }

    template <class Count>
    [[nodiscard]] Count count_matches(std::span<const Mask> choices) const
    {
        if (rows_.empty())
        {
            return Count{};
        }
        std::vector<Count> counts(size()), next(size());
        counts.front() = Count{1};
        for (const auto allowed : choices)
        {
            std::fill(next.begin(), next.end(), Count{});
            for (std::size_t state = 0; state < size(); ++state)
            {
                for (Mask remaining = allowed; remaining != 0; remaining &= remaining - 1)
                {
                    next[rows_[state].next[std::countr_zero(remaining)]] += counts[state];
                }
            }
            counts.swap(next);
        }
        Count total{};
        for (std::size_t state = 0; state < size(); ++state)
        {
            if (rows_[state].accepting)
            {
                total += counts[state];
            }
        }
        return total;
    }
};

template <std::size_t Alphabet>
class BitmaskNfa
{
    static_assert(1 <= Alphabet && Alphabet <= 64);
    using Mask = std::uint64_t;
    using SymbolTargets = std::array<Mask, Alphabet>;

    struct Row
    {
        SymbolTargets next{};
        Mask epsilon = 0;
    };

    std::vector<Row> rows_;
    std::size_t initial_ = 0;
    Mask accepting_ = 0;

    [[nodiscard]] static constexpr Mask bit(std::size_t index) noexcept
    {
        return Mask{1} << index;
    }

    static std::size_t checked_size(std::size_t states, std::size_t initial)
    {
        static_cast<void>(initial);
        return states;
    }

    void check_state(std::size_t state) const
    {
        static_cast<void>(state);
    }

    [[nodiscard]] std::vector<Mask> epsilon_closures() const
    {
        std::vector<Mask> closures(rows_.size());
        for (std::size_t state = 0; state < rows_.size(); ++state)
        {
            Mask reached = bit(state), frontier = reached;
            while (frontier != 0)
            {
                const auto source = std::countr_zero(frontier);
                frontier &= frontier - 1;
                const auto unseen = rows_[source].epsilon & ~reached;
                frontier |= unseen;
                reached |= unseen;
            }
            closures[state] = reached;
        }
        return closures;
    }

public:
    explicit BitmaskNfa(std::size_t states, std::size_t initial = 0)
        : rows_(checked_size(states, initial)), initial_(initial)
    {
    }

    void add_transition(std::size_t from, std::size_t symbol, std::size_t to)
    {
        check_state(from);
        check_state(to);
        rows_[from].next[symbol] |= bit(to);
    }

    void add_epsilon(std::size_t from, std::size_t to)
    {
        check_state(from);
        check_state(to);
        rows_[from].epsilon |= bit(to);
    }

    void set_accepting(std::size_t state, bool accept = true)
    {
        check_state(state);
        if (accept)
        {
            accepting_ |= bit(state);
        }
        else
        {
            accepting_ &= ~bit(state);
        }
    }

    [[nodiscard]] BitmaskDfa<Alphabet> determinize(
        std::size_t max_states = std::numeric_limits<std::size_t>::max()) const
    {
        using Dfa = BitmaskDfa<Alphabet>;
        if (rows_.empty())
        {
            return Dfa({});
        }
        const auto closures = epsilon_closures();
        std::vector<SymbolTargets> targets(rows_.size());
        for (std::size_t state = 0; state < rows_.size(); ++state)
        {
            for (std::size_t symbol = 0; symbol < Alphabet; ++symbol)
            {
                for (Mask reached = rows_[state].next[symbol]; reached != 0; reached &= reached - 1)
                {
                    targets[state][symbol] |= closures[std::countr_zero(reached)];
                }
            }
        }

        std::unordered_map<Mask, std::size_t> id;
        std::vector<Mask> subsets;
        std::vector<typename Dfa::Row> table;
        const auto intern = [&](Mask subset)
        {
            auto [where, fresh] = id.try_emplace(subset, subsets.size());
            if (fresh)
            {
                if (subsets.size() == max_states)
                {
                    throw std::length_error("BitmaskNfa::determinize: state limit");
                }
                subsets.push_back(subset);
            }
            return where->second;
        };
        intern(closures[initial_]);
        for (std::size_t row = 0; row < subsets.size(); ++row)
        {
            const auto subset = subsets[row];
            SymbolTargets moves{};
            for (Mask states = subset; states != 0; states &= states - 1)
            {
                const auto &next = targets[std::countr_zero(states)];
                for (std::size_t symbol = 0; symbol < Alphabet; ++symbol)
                {
                    moves[symbol] |= next[symbol];
                }
            }
            typename Dfa::Row result{{}, (subset & accepting_) != 0};
            for (std::size_t symbol = 0; symbol < Alphabet; ++symbol)
            {
                result.next[symbol] = intern(moves[symbol]);
            }
            table.push_back(result);
        }
        return Dfa(std::move(table));
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
