// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Todd_Coxeter.cpp
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/todd_coxeter
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/todd_coxeter.md
//
// API一覧
// MonoidRelation{left,right}: 同値の生成元列。空列は単位元。
// todd_coxeter(k,relations,limit): 関係式で定めた有限モノイドの右Cayleyグラフを列挙。仮頂点がlimitを超えるとnullopt。
// k・limitは0以上INT_MAX以下。生成元番号は0..k-1。P=仮頂点総数、L=全関係式の両辺の総長、R=関係式数。
// O(P*(1+k+L+R)*alpha(P))時間・O(P*(1+k))領域。有限でもPは最終頂点数より大きくなり得る。
// [FiniteMonoidCayleyGraph] size()/generators(): 元の数/生成元数。単位元は0。
// transition(state,g): 元に生成元gを右から掛けた結果。O(1)。
// evaluate(word,state=0): stateにwordを左から順に右掛けする。O(|word|+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TODD_COXETER_DEFINED
#define SIRKIT_TODD_COXETER_DEFINED

struct MonoidRelation
{
    std::vector<int> left, right;
};

namespace detail
{
class MonoidEnumerator;
}

class FiniteMonoidCayleyGraph
{
    friend class detail::MonoidEnumerator;
    std::size_t generators_;
    std::vector<int> transitions_;

    FiniteMonoidCayleyGraph(std::size_t generators, std::vector<int> transitions)
        : generators_(generators), transitions_(std::move(transitions))
    {
    }

    void check_state(int state) const
    {
        static_cast<void>(state);
    }

    void check_generator(int generator) const
    {
        static_cast<void>(generator);
    }

public:
    [[nodiscard]] std::size_t size() const noexcept
    {
        return generators_ == 0 ? 1 : transitions_.size() / generators_;
    }

    [[nodiscard]] std::size_t generators() const noexcept
    {
        return generators_;
    }

    [[nodiscard]] int transition(int state, int generator) const
    {
        check_state(state);
        check_generator(generator);
        return transitions_[static_cast<std::size_t>(state) * generators_ + generator];
    }

    [[nodiscard]] int evaluate(std::span<const int> word, int state = 0) const
    {
        check_state(state);
        for (const int generator : word)
        {
            check_generator(generator);
            state = transitions_[static_cast<std::size_t>(state) * generators_ + generator];
        }
        return state;
    }
};

namespace detail
{
class MonoidEnumerator
{
    const std::size_t generators_, limit_;
    std::vector<int> transitions_, parents_;
    std::vector<unsigned char> completed_;
    std::vector<std::pair<int, int>> pending_;

    int new_vertex()
    {
        const auto id = parents_.size();
        if (id == limit_)
        {
            throw CapacityReached{};
        }
        transitions_.insert(transitions_.end(), generators_, -1);
        parents_.push_back(-1);
        completed_.push_back(0);
        return static_cast<int>(id);
    }

    int root(int vertex)
    {
        int leader = vertex;
        while (parents_[leader] >= 0)
        {
            leader = parents_[leader];
        }
        while (vertex != leader)
        {
            const int next = parents_[vertex];
            parents_[vertex] = leader;
            vertex = next;
        }
        return leader;
    }

    int follow(int vertex, std::size_t generator)
    {
        const auto cell = static_cast<std::size_t>(root(vertex)) * generators_ + generator;
        if (transitions_[cell] == -1)
        {
            const int added = new_vertex();
            transitions_[cell] = added;
        }
        return root(transitions_[cell]);
    }

    int follow(int vertex, std::span<const int> word)
    {
        for (const int generator : word)
        {
            vertex = follow(vertex, static_cast<std::size_t>(generator));
        }
        return vertex;
    }

    void identify(int first, int second)
    {
        pending_.emplace_back(first, second);
        while (!pending_.empty())
        {
            const auto [u, v] = pending_.back();
            pending_.pop_back();
            int a = root(u), b = root(v);
            if (a == b)
            {
                continue;
            }
            if (parents_[a] > parents_[b])
            {
                std::swap(a, b);
            }
            parents_[a] += parents_[b];
            parents_[b] = a;
            completed_[a] |= completed_[b];
            const auto left = static_cast<std::size_t>(a) * generators_;
            const auto right = static_cast<std::size_t>(b) * generators_;
            for (std::size_t g = 0; g < generators_; ++g)
            {
                const int incoming = transitions_[right + g];
                if (incoming == -1)
                {
                    continue;
                }
                int &stored = transitions_[left + g];
                if (stored == -1)
                {
                    stored = incoming;
                }
                else
                {
                    pending_.emplace_back(stored, incoming);
                }
            }
        }
    }

    FiniteMonoidCayleyGraph compact()
    {
        std::vector<int> representatives;
        std::vector<int> remap(parents_.size(), -1);
        for (std::size_t i = 0; i < parents_.size(); ++i)
        {
            const auto leader = root(static_cast<int>(i));
            if (remap[leader] == -1)
            {
                remap[leader] = static_cast<int>(representatives.size());
                representatives.push_back(leader);
            }
        }
        std::vector<int> table;
        table.reserve(representatives.size() * generators_);
        for (const int leader : representatives)
        {
            const auto offset = static_cast<std::size_t>(leader) * generators_;
            for (std::size_t g = 0; g < generators_; ++g)
            {
                table.push_back(remap[root(transitions_[offset + g])]);
            }
        }
        return FiniteMonoidCayleyGraph(generators_, std::move(table));
    }

public:
    struct CapacityReached
    {
    };

    MonoidEnumerator(std::size_t generators, std::size_t limit)
        : generators_(generators), limit_(limit)
    {
    }

    FiniteMonoidCayleyGraph enumerate(std::span<const MonoidRelation> relations)
    {
        new_vertex();
        for (std::size_t i = 0; i < parents_.size(); ++i)
        {
            const int vertex = static_cast<int>(i);
            if (completed_[root(vertex)])
            {
                continue;
            }
            for (const auto &relation : relations)
            {
                const int a = follow(vertex, relation.left);
                const int b = follow(vertex, relation.right);
                identify(a, b);
            }
            for (std::size_t g = 0; g < generators_; ++g)
            {
                follow(vertex, g);
            }
            completed_[root(vertex)] = 1;
        }
        return compact();
    }
};
}

[[nodiscard]] inline std::optional<FiniteMonoidCayleyGraph> todd_coxeter(
    std::size_t generators, std::span<const MonoidRelation> relations, std::size_t limit)
{
    try
    {
        return detail::MonoidEnumerator(generators, limit).enumerate(relations);
    }
    catch (const detail::MonoidEnumerator::CapacityReached &)
    {
        return std::nullopt;
    }
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
