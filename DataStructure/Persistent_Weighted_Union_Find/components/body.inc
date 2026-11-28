#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#ifndef SIRKIT_PERSISTENT_WEIGHTED_UNION_FIND_DEFINED
#define SIRKIT_PERSISTENT_WEIGHTED_UNION_FIND_DEFINED

namespace detail
{
template <class T>
struct PersistentWeightAdd
{
    T operator()(const T &a, const T &b) const
    {
        if constexpr (std::signed_integral<T>)
        {
            if ((b > 0 && a > std::numeric_limits<T>::max() - b) ||
                (b < 0 && a < std::numeric_limits<T>::min() - b))
            {
                throw std::overflow_error("PersistentWeightedUnionFind: potential sum");
            }
        }
        return static_cast<T>(a + b);
    }
};

template <class T>
struct PersistentWeightSubtract
{
    T operator()(const T &a, const T &b) const
    {
        if constexpr (std::signed_integral<T>)
        {
            if ((b > 0 && a < std::numeric_limits<T>::min() + b) ||
                (b < 0 && a > std::numeric_limits<T>::max() + b))
            {
                throw std::overflow_error("PersistentWeightedUnionFind: potential difference");
            }
        }
        return static_cast<T>(a - b);
    }
};
}

template <std::equality_comparable T, class Add = detail::PersistentWeightAdd<T>,
          class Subtract = detail::PersistentWeightSubtract<T>, int Shift = 4>
class PersistentWeightedUnionFind
{
    struct Entry
    {
        int parent;
        T delta;
    };
    struct Group
    {
        T zero;
        [[no_unique_address]] Add add;
        [[no_unique_address]] Subtract subtract;
    };

    using Array = PersistentArray<Entry, Shift>;

public:
    using Version = typename Array::Node *;

    enum class MergeResult
    {
        Merged,
        AlreadyConsistent,
        Contradiction,
    };

    explicit PersistentWeightedUnionFind(std::size_t n = 0, T zero = T{}, Add add = {}, Subtract subtract = {})
        : entries_(CheckedSize(n), Entry{-1, zero}),
          group_(std::make_shared<const Group>(Group{std::move(zero), std::move(add), std::move(subtract)}))
    {
        entries_.restore(nullptr);
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(entries_.size());
    }

    [[nodiscard]] Version version() const noexcept
    {
        return entries_.root;
    }

    void restore(Version base)
    {
        entries_.restore(base);
    }

    [[nodiscard]] int root(int vertex, Version base) const
    {
        return FindRoot(vertex, base).first;
    }

    [[nodiscard]] int root(int vertex) const
    {
        return root(vertex, version());
    }

    [[nodiscard]] int size(int vertex, Version base) const
    {
        return -FindRoot(vertex, base).second;
    }

    [[nodiscard]] int size(int vertex) const
    {
        return size(vertex, version());
    }

    [[nodiscard]] bool same(int from, int to, Version base) const
    {
        return root(from, base) == root(to, base);
    }

    [[nodiscard]] bool same(int from, int to) const
    {
        return same(from, to, version());
    }

    [[nodiscard]] bool issame(int from, int to, Version base) const
    {
        return same(from, to, base);
    }

    [[nodiscard]] bool issame(int from, int to) const
    {
        return same(from, to);
    }

    [[nodiscard]] std::pair<int, T> root_with_weight(int vertex, Version base) const
    {
        auto result = FindWeightedRoot(vertex, base);
        return {result.root, std::move(result.delta)};
    }

    [[nodiscard]] std::pair<int, T> root_with_weight(int vertex) const
    {
        return root_with_weight(vertex, version());
    }

    [[nodiscard]] std::optional<T> try_diff(int from, int to, Version base) const
    {
        const auto a = FindWeightedRoot(from, base);
        const auto b = FindWeightedRoot(to, base);
        if (a.root != b.root)
        {
            return std::nullopt;
        }
        return std::invoke(group_->subtract, b.delta, a.delta);
    }

    [[nodiscard]] std::optional<T> try_diff(int from, int to) const
    {
        return try_diff(from, to, version());
    }

    MergeResult unite_checked(int from, int to, const T &delta, Version base)
    {
        const auto a = FindWeightedRoot(from, base);
        const auto b = FindWeightedRoot(to, base);
        if (a.root == b.root)
        {
            const bool consistent = std::invoke(group_->subtract, b.delta, a.delta) == delta;
            restore(base);
            return consistent ? MergeResult::AlreadyConsistent : MergeResult::Contradiction;
        }
        const T root_delta = std::invoke(group_->subtract, std::invoke(group_->add, a.delta, delta), b.delta);
        const Entry joined{a.parent + b.parent, group_->zero};
        if (a.parent > b.parent)
        {
            const T reversed = std::invoke(group_->subtract, group_->zero, root_delta);
            entries_.update_two(base, b.root, joined, a.root, Entry{b.root, reversed});
        }
        else
        {
            entries_.update_two(base, a.root, joined, b.root, Entry{a.root, root_delta});
        }
        return MergeResult::Merged;
    }

    MergeResult unite_checked(int from, int to, const T &delta)
    {
        return unite_checked(from, to, delta, version());
    }

private:
    struct WeightedRoot
    {
        int root;
        int parent;
        T delta;
    };

    Array entries_;
    std::shared_ptr<const Group> group_;

    static int CheckedSize(std::size_t n)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("PersistentWeightedUnionFind: too many vertices");
        }
        return static_cast<int>(n);
    }

    [[nodiscard]] std::pair<int, int> FindRoot(int vertex, Version base) const
    {
        int parent = entries_.get(base, vertex).parent;
        while (parent >= 0)
        {
            vertex = parent;
            parent = entries_.get(base, vertex).parent;
        }
        return {vertex, parent};
    }

    [[nodiscard]] WeightedRoot FindWeightedRoot(int vertex, Version base) const
    {
        Entry entry = entries_.get(base, vertex);
        T delta = group_->zero;
        while (entry.parent >= 0)
        {
            delta = std::invoke(group_->add, delta, entry.delta);
            vertex = entry.parent;
            entry = entries_.get(base, vertex);
        }
        return {vertex, entry.parent, std::move(delta)};
    }
};
#endif
