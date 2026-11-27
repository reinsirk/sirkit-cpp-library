// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Noncommutative_Weighted_Union_Find.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/noncommutative_weighted_union_find.md
//
// API一覧
// NoncommutativeWeightedUnionFind<S,op,e,inverse>(n=0): n頂点を独立に構築。O(n+1)時間・領域。
// unite_checked(from,to,w): A_to=A_from*wを追加。Merged/AlreadyConsistent/Contradictionを返す。
// constrain(from,to,w): 矛盾しない場合true。merge: 新規結合ならtrue。unite: 戻り値なし。
// diff(from,to): A_from^{-1}*A_to。try_diffは別成分ならnullopt、consistentは制約を追加せず判定。
// root(i)/leader(i), same(a,b)/issame(a,b), size(i): 代表、連結判定、成分サイズ。
// weight(i): 代表からiへの差。root_with_weight(i): {代表, weight(i)}。
// size()/empty()/component_count(): 全頂点数、空判定、成分数。O(1)。他は償却O(α(n+1))。
// op/e/inverseは群の積/単位元/逆元。Sはコピー可能でムーブ代入が非送出、等値比較可能。
// 頂点は0..n-1、n<=INT_MAX。演算・コピーの計算時間をO(1)としたときの計算量。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NONCOMMUTATIVE_WEIGHTED_UNION_FIND_DEFINED
#define SIRKIT_NONCOMMUTATIVE_WEIGHTED_UNION_FIND_DEFINED

template <std::equality_comparable S, auto op, auto e, auto inverse>
class NoncommutativeWeightedUnionFind
{
    static_assert(std::is_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_invocable_r_v<S, decltype(e)>);
    static_assert(std::is_invocable_r_v<S, decltype(inverse), const S &>);
    static_assert(std::is_copy_constructible_v<S> && std::is_move_constructible_v<S> &&
                  std::is_nothrow_move_assignable_v<S>);

public:
    enum class MergeResult
    {
        Merged,
        AlreadyConsistent,
        Contradiction,
    };

    explicit NoncommutativeWeightedUnionFind(std::size_t n = 0)
        : parent_(CheckedSize(n), -1), potential_(n, std::invoke(e)), components_(static_cast<int>(n))
    {
    }

    NoncommutativeWeightedUnionFind(const NoncommutativeWeightedUnionFind &) = default;

    NoncommutativeWeightedUnionFind &operator=(const NoncommutativeWeightedUnionFind &other)
    {
        if (this != &other)
        {
            NoncommutativeWeightedUnionFind replacement(other);
            Swap(replacement);
        }
        return *this;
    }

    NoncommutativeWeightedUnionFind(NoncommutativeWeightedUnionFind &&other) noexcept
        : parent_(std::move(other.parent_)), potential_(std::move(other.potential_)),
          components_(std::exchange(other.components_, 0))
    {
        other.parent_.clear();
        other.potential_.clear();
    }

    NoncommutativeWeightedUnionFind &operator=(NoncommutativeWeightedUnionFind &&other) noexcept
    {
        if (this != &other)
        {
            NoncommutativeWeightedUnionFind replacement(std::move(other));
            Swap(replacement);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(parent_.size());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return parent_.empty();
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return components_;
    }

    [[nodiscard]] int root(int vertex)
    {
        CheckVertex(vertex);
        return Compress(vertex);
    }

    [[nodiscard]] int leader(int vertex)
    {
        return root(vertex);
    }

    [[nodiscard]] bool same(int a, int b)
    {
        CheckVertex(a);
        CheckVertex(b);
        const auto representative = Compress(a);
        return representative == Compress(b);
    }

    [[nodiscard]] bool issame(int a, int b)
    {
        return same(a, b);
    }

    [[nodiscard]] int size(int vertex)
    {
        return -parent_[root(vertex)];
    }

    [[nodiscard]] S weight(int vertex)
    {
        CheckVertex(vertex);
        Compress(vertex);
        return potential_[vertex];
    }

    [[nodiscard]] std::pair<int, S> root_with_weight(int vertex)
    {
        const auto representative = root(vertex);
        return {representative, potential_[vertex]};
    }

    [[nodiscard]] std::optional<S> try_diff(int from, int to)
    {
        if (!same(from, to))
        {
            return std::nullopt;
        }
        return Product(Inverse(potential_[from]), potential_[to]);
    }

    [[nodiscard]] S diff(int from, int to)
    {
        auto result = try_diff(from, to);
#ifndef SKIP_RUNTIME_CHECKS
        if (!result)
        {
            throw std::invalid_argument("NoncommutativeWeightedUnionFind: disjoint vertices");
        }
#endif
        return std::move(*result);
    }

    [[nodiscard]] bool consistent(int from, int to, const S &relation)
    {
        return !same(from, to) || Product(potential_[from], relation) == potential_[to];
    }

    MergeResult unite_checked(int from, int to, const S &relation)
    {
        CheckVertex(from);
        CheckVertex(to);
        auto keep = Compress(from);
        auto absorbed = Compress(to);
        const auto proposed = Product(potential_[from], relation);
        if (keep == absorbed)
        {
            return proposed == potential_[to] ? MergeResult::AlreadyConsistent : MergeResult::Contradiction;
        }

        // A_to=A_from*relation。根保持側から吸収側への差はg_from*relation*inv(g_to)。
        auto bridge = Product(proposed, Inverse(potential_[to]));
        if (parent_[keep] > parent_[absorbed])
        {
            bridge = Inverse(bridge);
            std::swap(keep, absorbed);
        }
        potential_[absorbed] = std::move(bridge);
        parent_[keep] += parent_[absorbed];
        parent_[absorbed] = keep;
        --components_;
        return MergeResult::Merged;
    }

    bool constrain(int from, int to, const S &relation)
    {
        return unite_checked(from, to, relation) != MergeResult::Contradiction;
    }

    bool merge(int from, int to, const S &relation)
    {
        return unite_checked(from, to, relation) == MergeResult::Merged;
    }

    void unite(int from, int to, const S &relation)
    {
        (void)unite_checked(from, to, relation);
    }

private:
    std::vector<int> parent_;
    // potential_[v] は親からvへの差。根の値は単位元。
    std::vector<S> potential_;
    int components_;

    static std::size_t CheckedSize(std::size_t n)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("NoncommutativeWeightedUnionFind: too many vertices");
        }
        return n;
    }

    void Swap(NoncommutativeWeightedUnionFind &other) noexcept
    {
        parent_.swap(other.parent_);
        potential_.swap(other.potential_);
        std::swap(components_, other.components_);
    }

    void CheckVertex(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(vertex) >= parent_.size())
        {
            throw std::out_of_range("NoncommutativeWeightedUnionFind: invalid vertex");
        }
#else
        (void)vertex;
#endif
    }

    static S Product(const S &a, const S &b)
    {
        return std::invoke(op, a, b);
    }

    static S Inverse(const S &value)
    {
        return std::invoke(inverse, value);
    }

    int Compress(int vertex)
    {
        const int parent = parent_[vertex];
        if (parent < 0)
        {
            return vertex;
        }
        if (parent_[parent] < 0)
        {
            return parent;
        }

        const int representative = Compress(parent);
        auto combined = Product(potential_[parent], potential_[vertex]);
        potential_[vertex] = std::move(combined);
        parent_[vertex] = representative;
        return representative;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
