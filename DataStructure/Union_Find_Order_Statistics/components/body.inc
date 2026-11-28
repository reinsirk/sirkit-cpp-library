#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#ifndef SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED
#define SIRKIT_UNION_FIND_ORDER_STATISTICS_DEFINED

template <std::totally_ordered T>
    requires std::copyable<T>
class UnionFindOrderStatistics
{
    using Index = std::size_t;

public:
    explicit UnionFindOrderStatistics(const std::vector<T> &labels)
        : groups_(labels.size()), values_(labels), roots_(labels.size())
    {
        std::sort(values_.begin(), values_.end());
        values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
        if (!labels.empty())
        {
            const auto path_nodes = std::bit_width(values_.size() - 1) + 1;
            const auto estimated = static_cast<std::uint64_t>(labels.size()) * path_nodes;
            nodes_.reserve(static_cast<Index>(std::min<std::uint64_t>(estimated, nodes_.max_size())));
        }
        for (Index v = 0; v < labels.size(); ++v)
        {
            roots_[v] = BuildSingleton(0, values_.size(), LowerBound(labels[v]));
        }
    }

    UnionFindOrderStatistics(const UnionFindOrderStatistics &) = default;
    UnionFindOrderStatistics(UnionFindOrderStatistics &&) noexcept = default;

    UnionFindOrderStatistics &operator=(UnionFindOrderStatistics other) noexcept
    {
        groups_.swap(other.groups_);
        values_.swap(other.values_);
        roots_.swap(other.roots_);
        nodes_.swap(other.nodes_);
        return *this;
    }

    bool merge(int a, int b)
    {
        a = groups_.root(a);
        b = groups_.root(b);
        if (!groups_.merge(a, b))
        {
            return false;
        }
        const auto joined = MergeTrees(roots_[a], roots_[b]);
        roots_[a] = joined;
        roots_[b] = joined;
        return true;
    }

    int root(int vertex)
    {
        return groups_.root(vertex);
    }

    bool issame(int a, int b)
    {
        return groups_.issame(a, b);
    }

    int size(int vertex)
    {
        return groups_.size(vertex);
    }

    int size() const noexcept
    {
        return groups_.size();
    }

    int component_count() const noexcept
    {
        return groups_.component_count();
    }

    std::optional<T> kth(int vertex, int k)
    {
        auto id = roots_[groups_.root(vertex)];
        if (k < 0 || k >= SizeOf(id))
        {
            return std::nullopt;
        }
        Index from = 0;
        Index to = values_.size();
        while (to - from > 1)
        {
            const auto &node = nodes_[id - 1];
            const auto mid = from + (to - from) / 2;
            const int left_count = SizeOf(node.left);
            if (k < left_count)
            {
                id = node.left;
                to = mid;
            }
            else
            {
                k -= left_count;
                id = node.right;
                from = mid;
            }
        }
        return values_[from];
    }

    int count(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        const auto end = LowerBound(label);
        if (end == values_.size() || values_[end] != label)
        {
            return 0;
        }
        return Prefix(id, 0, values_.size(), end + 1) - Prefix(id, 0, values_.size(), end);
    }

    int count_less(int vertex, const T &label)
    {
        const auto id = roots_[groups_.root(vertex)];
        return Prefix(id, 0, values_.size(), LowerBound(label));
    }

private:
    struct Node
    {
        Index left = 0;
        Index right = 0;
        int count = 1;
    };

    UnionFind groups_;
    std::vector<T> values_;
    std::vector<Index> roots_;
    std::vector<Node> nodes_;

    Index LowerBound(const T &label) const
    {
        return static_cast<Index>(std::lower_bound(values_.begin(), values_.end(), label) - values_.begin());
    }

    int SizeOf(Index id) const
    {
        return id == 0 ? 0 : nodes_[id - 1].count;
    }

    Index BuildSingleton(Index from, Index to, Index position)
    {
        Node node;
        if (to - from > 1)
        {
            const auto mid = from + (to - from) / 2;
            if (position < mid)
            {
                node.left = BuildSingleton(from, mid, position);
            }
            else
            {
                node.right = BuildSingleton(mid, to, position);
            }
        }
        nodes_.push_back(node);
        return nodes_.size();
    }

    Index MergeTrees(Index first, Index second)
    {
        if (first == 0)
        {
            return second;
        }
        if (second == 0)
        {
            return first;
        }
        auto &left = nodes_[first - 1];
        const auto &right = nodes_[second - 1];
        left.left = MergeTrees(left.left, right.left);
        left.right = MergeTrees(left.right, right.right);
        left.count += right.count;
        return first;
    }

    int Prefix(Index id, Index from, Index to, Index end) const
    {
        if (id == 0 || end <= from)
        {
            return 0;
        }
        const auto &node = nodes_[id - 1];
        if (to <= end)
        {
            return node.count;
        }
        const auto mid = from + (to - from) / 2;
        return Prefix(node.left, from, mid, end) + Prefix(node.right, mid, to, end);
    }
};

#endif
