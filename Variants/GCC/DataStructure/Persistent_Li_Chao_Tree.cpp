// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Persistent_Li_Chao_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_li_chao_tree.md
//
// API一覧
// PersistentLiChaoTree<T>(coordinates): 照会するx座標を登録。重複はまとめる。Tは符号付き整数。
// Version{}: 直線を含まない初期版。各版は作成した木で使う。
// add_line(version,line): 直線を追加した版を作る。元の版も使える。償却O(log(n+1))時間・追加領域。
// add_segment(version,l,r,line): l<=x<rに直線を追加した版。l<=r。償却O(log²(n+1))時間・追加領域。
// get(version,x): xでの最小値をO(log(n+1))で返す。登録外のx、または有効な直線がない場合はnullopt。
// Line{slope,intercept}: y=slope*x+intercept。計算途中もTに収まること。nは登録した異なる座標数。
// 構築は座標の入力数をmとしてO(m log(m+1))時間、O(m)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_LI_CHAO_TREE_DEFINED
#define SIRKIT_PERSISTENT_LI_CHAO_TREE_DEFINED

template <std::signed_integral T = std::int64_t>
class PersistentLiChaoTree
{
    using Index = std::size_t;

public:
    struct Line
    {
        T slope;
        T intercept;
    };

    class Version
    {
        friend class PersistentLiChaoTree;

    public:
        Version() = default;

    private:
        explicit Version(Index root) : root_(root)
        {
        }

        Index root_ = 0;
    };

    explicit PersistentLiChaoTree(std::vector<T> coordinates) : xs_(std::move(coordinates))
    {
        std::sort(xs_.begin(), xs_.end());
        xs_.erase(std::unique(xs_.begin(), xs_.end()), xs_.end());
    }

    PersistentLiChaoTree(const PersistentLiChaoTree &) = default;
    PersistentLiChaoTree(PersistentLiChaoTree &&) noexcept = default;

    PersistentLiChaoTree &operator=(PersistentLiChaoTree other) noexcept
    {
        xs_.swap(other.xs_);
        nodes_.swap(other.nodes_);
        lines_.swap(other.lines_);
        return *this;
    }

    Version add_line(Version version, Line line)
    {
        CheckVersion(version);
        return Update(version, 0, xs_.size(), line);
    }

    Version add_segment(Version version, T left, T right, Line line)
    {
        CheckVersion(version);
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right)
        {
            throw std::invalid_argument("PersistentLiChaoTree: reversed segment");
        }
#endif
        return Update(version, LowerBound(left), LowerBound(right), line);
    }

    std::optional<T> get(Version version, T x) const
    {
        CheckVersion(version);
        const Index position = LowerBound(x);
        if (position == xs_.size() || xs_[position] != x)
        {
            return std::nullopt;
        }
        Index from = 0;
        Index to = xs_.size();
        Index node = version.root_;
        std::optional<T> best;
        while (node)
        {
            const auto &current = Get(node);
            if (current.line)
            {
                const T y = Evaluate(current.line, x);
                if (!best || y < *best)
                {
                    best = y;
                }
            }
            const Index mid = from + (to - from) / 2;
            if (position < mid)
            {
                node = current.left;
                to = mid;
            }
            else
            {
                node = current.right;
                from = mid;
            }
        }
        return best;
    }

private:
    struct Node
    {
        Index line = 0;
        Index left = 0;
        Index right = 0;
    };

    static constexpr Node kEmpty{};
    std::vector<T> xs_;
    std::vector<Node> nodes_;
    std::vector<Line> lines_;

    const Node &Get(Index id) const
    {
        return id == 0 ? kEmpty : nodes_[id - 1];
    }

    T Evaluate(Index line, T x) const
    {
        const auto &[slope, intercept] = lines_[line - 1];
        return static_cast<T>(slope * x + intercept);
    }

    Index LowerBound(T x) const
    {
        return static_cast<Index>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
    }

    Index Store(Node node)
    {
        nodes_.push_back(node);
        return nodes_.size();
    }

    Index InsertLine(Index id, Index candidate, Index from, Index to)
    {
        Node next = Get(id);
        if (next.line == 0)
        {
            next.line = candidate;
            return Store(next);
        }
        const auto strictly_better = [&](Index at)
        {
            return Evaluate(candidate, xs_[at]) < Evaluate(next.line, xs_[at]);
        };
        const bool wins_left = strictly_better(from);
        const bool wins_right = strictly_better(to - 1);
        if (!wins_left && !wins_right)
        {
            return id;
        }
        if (wins_left && wins_right)
        {
            next.line = candidate;
            return Store(next);
        }
        const Index mid = from + (to - from) / 2;
        const bool wins_mid = strictly_better(mid);
        if (wins_mid)
        {
            std::swap(next.line, candidate);
        }
        if (wins_left != wins_mid)
        {
            next.left = InsertLine(next.left, candidate, from, mid);
        }
        else
        {
            next.right = InsertLine(next.right, candidate, mid, to);
        }
        return Store(next);
    }

    Index InsertSegment(Index id, Index line, Index from, Index to, Index first, Index last)
    {
        if (first <= from && to <= last)
        {
            return InsertLine(id, line, from, to);
        }
        const Node old = Get(id);
        Node next = old;
        const Index mid = from + (to - from) / 2;
        if (first < mid)
        {
            next.left = InsertSegment(old.left, line, from, mid, first, last);
        }
        if (last > mid)
        {
            next.right = InsertSegment(old.right, line, mid, to, first, last);
        }
        if (next.left == old.left && next.right == old.right)
        {
            return id;
        }
        return Store(next);
    }

    Version Update(Version version, Index first, Index last, Line line)
    {
        if (first == last)
        {
            return version;
        }
        const Index depth = std::bit_width(xs_.size() - 1) + 1;
        const Index maximum_new_nodes = first == 0 && last == xs_.size() ? depth : 2 * depth * (depth + 1);
        const Index required = nodes_.size() + maximum_new_nodes;
        const Index capacity = nodes_.capacity();
        if (required > capacity)
        {
            nodes_.reserve(std::max(required, std::min(nodes_.max_size(), capacity + capacity / 2)));
        }
        lines_.push_back(line);
        const Index root = InsertSegment(version.root_, lines_.size(), 0, xs_.size(), first, last);
        if (root == version.root_)
        {
            lines_.pop_back();
        }
        return Version(root);
    }

    void CheckVersion([[maybe_unused]] Version version) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (version.root_ > nodes_.size())
        {
            throw std::out_of_range("PersistentLiChaoTree: invalid version");
        }
#endif
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
