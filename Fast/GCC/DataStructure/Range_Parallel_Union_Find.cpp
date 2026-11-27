// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Range_Parallel_Union_Find.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_parallel_union_find.md
//
// API一覧
// RangeParallelUnionFind(n=0): n頂点を独立成分として構築。O(n+1)時間・領域、n<=INT_MAX。
// merge_range(a,b,len[,on_merge]): 各i in [0,len)で頂点a+iとb+iを併合し、新規結合数を返す。
// on_merge(keep, absorbed)は実際の成分結合の直後に1回呼ぶ。再入更新は不可。例外時は完了済みの結合を保つ。
// merge(a,b)/unite(a,b): 1組だけ結合し、新規結合かを返す／戻り値なし。
// root(i)/leader(i)、same(a,b)/issame(a,b)、size(i)は通常のUnionFindと同じ。償却O(α(n+1))。
// size()/empty()/component_count(): 全頂点数、空判定、成分数。各O(1)。
// 最大処理区間長K、操作数QでO((n log(K+2)+Q)α(n+1))時間、O(n log(K+2))領域。
// レベルhは2^h長の有効開始位置n-2^h+1個を確保する。空区間・重なった区間も可。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RANGE_PARALLEL_UNION_FIND_DEFINED
#define SIRKIT_RANGE_PARALLEL_UNION_FIND_DEFINED

class RangeParallelUnionFind
{
public:
    explicit RangeParallelUnionFind(std::size_t n = 0)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("RangeParallelUnionFind: too many vertices");
        }
        levels_.emplace_back(n, -1);
        components_ = static_cast<int>(n);
    }

    RangeParallelUnionFind(const RangeParallelUnionFind &) = default;
    RangeParallelUnionFind &operator=(const RangeParallelUnionFind &) = default;

    RangeParallelUnionFind(RangeParallelUnionFind &&other) noexcept
        : levels_(std::move(other.levels_)), components_(std::exchange(other.components_, 0))
    {
        other.levels_.clear();
    }

    RangeParallelUnionFind &operator=(RangeParallelUnionFind &&other) noexcept
    {
        if (this != &other)
        {
            levels_ = std::move(other.levels_);
            components_ = std::exchange(other.components_, 0);
            other.levels_.clear();
        }
        return *this;
    }

    template <class OnMerge>
    int merge_range(int a, int b, int length, OnMerge &&on_merge)
    {
        CheckRanges(a, b, length);
        if (length == 0 || a == b || components_ == 1)
        {
            return 0;
        }

        const auto level = static_cast<unsigned>(std::bit_width(static_cast<unsigned>(length)) - 1);
        const int n = size();
        while (levels_.size() <= level)
        {
            const auto next_length = std::size_t{1} << levels_.size();
            levels_.emplace_back(static_cast<std::size_t>(n) - next_length + 1, -1);
        }

        // 先頭と末尾のブロックで区間全体を覆う。重なる部分の再結合は起きない。
        int result = MergeBlock(level, a, b, on_merge);
        const int tail = length - (1 << level);
        if (tail != 0)
        {
            result += MergeBlock(level, a + tail, b + tail, on_merge);
        }
        return result;
    }

    int merge_range(int a, int b, int length)
    {
        return merge_range(a, b, length, [](int, int)
                           {
                           });
    }

    bool merge(int a, int b)
    {
        return merge_range(a, b, 1) != 0;
    }

    void unite(int a, int b)
    {
        (void)merge(a, b);
    }

    [[nodiscard]] int root(int i)
    {
        CheckIndex(i);
        return Find(levels_.front(), i);
    }

    [[nodiscard]] int leader(int i)
    {
        return root(i);
    }

    [[nodiscard]] bool same(int a, int b)
    {
        CheckIndex(a);
        CheckIndex(b);
        return Find(levels_.front(), a) == Find(levels_.front(), b);
    }

    [[nodiscard]] bool issame(int a, int b)
    {
        return same(a, b);
    }

    [[nodiscard]] int size(int i)
    {
        const auto representative = root(i);
        return -levels_.front()[representative];
    }

    [[nodiscard]] int size() const noexcept
    {
        return levels_.empty() ? 0 : static_cast<int>(levels_.front().size());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return components_;
    }

private:
    std::vector<std::vector<int>> levels_;
    int components_ = 0;

    void CheckIndex(int i) const
    {
        (void)i;
    }

    void CheckRanges(int a, int b, int length) const
    {
        (void)a;
        (void)b;
        (void)length;
    }

    static int Find(std::vector<int> &parents, int i) noexcept
    {
        while (parents[i] >= 0)
        {
            const int next = parents[i];
            if (parents[next] < 0)
            {
                return next;
            }
            i = parents[i] = parents[next];
        }
        return i;
    }

    template <class OnMerge>
    int MergeBlock(unsigned level, int a, int b, OnMerge &on_merge)
    {
        auto &parents = levels_[level];
        auto keep = Find(parents, a);
        auto absorbed = Find(parents, b);
        if (keep == absorbed)
        {
            return 0;
        }
        if (parents[keep] > parents[absorbed])
        {
            std::swap(keep, absorbed);
        }

        int count = 1;
        if (level != 0)
        {
            // 通知が例外を送出しても、未完了のブロックを結合済みにしない。
            const unsigned child = level - 1;
            const int half = 1 << child;
            count = MergeBlock(child, a, b, on_merge);
            count += MergeBlock(child, a + half, b + half, on_merge);
        }

        parents[keep] += parents[absorbed];
        parents[absorbed] = keep;
        if (level == 0)
        {
            --components_;
            on_merge(keep, absorbed);
        }
        return count;
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
