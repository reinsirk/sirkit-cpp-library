// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Linear_LCA.cpp
// 参考（Euler Tourと±1 RMQ）: https://doi.org/10.1007/10719839_9
// 参考（AtCoder Algorithm Lectures）: https://info.atcoder.jp/entry/algorithm_lectures/linear_time_lca
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/linear_lca.md
//
// API一覧
// LinearLCA(parents): 親配列から森を構築。根の親は-1。頂点は0始まり、親と子の番号順は任意。
//   Euler Tourの深さの差が±1になることを利用し、O(n+1) 時間・領域で前処理。再帰を使わない。
// get(u,v)/operator()(u,v): 最小共通祖先。別成分なら-1。
// depth_of(v): 成分根からの辺数。get_distance(u,v): 辺数の距離。別成分なら-1。
// size(): 頂点数。構築以外はすべて O(1)。n<=(INT_MAX-1)/2。空の森可。親番号の範囲外と閉路は例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LINEAR_LCA_DEFINED
#define SIRKIT_LINEAR_LCA_DEFINED

namespace detail
{
inline int checked_linear_lca_count(std::size_t count)
{
    constexpr auto kMaximum = static_cast<std::size_t>((std::numeric_limits<int>::max() - 1) / 2);
    if (count > kMaximum)
    {
        throw std::length_error("LinearLCA: too many vertices");
    }
    return static_cast<int>(count);
}

class LinearLcaRmq
{
public:
    void build(std::vector<int> depths)
    {
        depths_ = std::move(depths);
        const auto log = std::bit_width(static_cast<unsigned int>(depths_.size())) - 1U;
        block_size_ = std::max(1U, log / 2);
        const auto block_count = (depths_.size() - 1) / block_size_ + 1;
        signatures_.assign(block_count, 0);
        BuildMicroTable();

        std::vector<int> minima(block_count);
        for (std::size_t block = 0; block < block_count; ++block)
        {
            const auto start = block * block_size_;
            const auto limit = std::min(start + block_size_, depths_.size());
            unsigned int shape = 0;
            for (std::size_t i = start + 1; i < limit; ++i)
            {
                if (depths_[i - 1] < depths_[i])
                {
                    shape |= 1U << (i - start - 1);
                }
            }
            signatures_[block] = shape;
            minima[block] = WithinBlock(start, limit - 1);
        }
        sparse_.clear();
        sparse_.push_back(std::move(minima));
        for (std::size_t width = 2; width <= block_count; width *= 2)
        {
            const auto &previous = sparse_.back();
            const auto half = width / 2;
            std::vector<int> row(block_count - width + 1);
            for (std::size_t i = 0; i < row.size(); ++i)
            {
                row[i] = EarlierMinimum(previous[i], previous[i + half]);
            }
            sparse_.push_back(std::move(row));
        }
    }

    [[nodiscard]] int depth(std::size_t index) const
    {
        return depths_[index];
    }

    [[nodiscard]] int index(std::size_t left, std::size_t right) const
    {
        const auto first = left / block_size_, last = right / block_size_;
        if (first == last)
        {
            return WithinBlock(left, right);
        }
        const int tail = WithinBlock(left, (first + 1) * block_size_ - 1);
        const int head = WithinBlock(last * block_size_, right);
        const int border = EarlierMinimum(tail, head);
        if (last - first == 1)
        {
            return border;
        }
        const auto level = std::bit_width(last - first - 1) - 1U;
        const auto span = std::size_t{1} << level;
        const int inner = EarlierMinimum(sparse_[level][first + 1], sparse_[level][last - span]);
        return EarlierMinimum(border, inner);
    }

private:
    unsigned int block_size_ = 1;
    std::vector<int> depths_;
    std::vector<unsigned int> signatures_;
    std::vector<std::uint8_t> micro_;
    std::vector<std::vector<int>> sparse_;

    [[nodiscard]] std::size_t Offset(unsigned int signature, std::size_t left, std::size_t right) const
    {
        return (static_cast<std::size_t>(signature) * block_size_ + left) * block_size_ + right;
    }

    void BuildMicroTable()
    {
        const auto shapes = 1U << (block_size_ - 1);
        micro_.resize(static_cast<std::size_t>(shapes) * block_size_ * block_size_);
        for (unsigned int shape = 0; shape < shapes; ++shape)
        {
            for (unsigned int left = 0; left < block_size_; ++left)
            {
                int current_depth = 0, minimum_depth = 0;
                auto best = left;
                micro_[Offset(shape, left, left)] = static_cast<std::uint8_t>(best);
                for (unsigned int right = left + 1; right < block_size_; ++right)
                {
                    current_depth += ((shape >> (right - 1)) & 1U) != 0 ? 1 : -1;
                    if (current_depth < minimum_depth)
                    {
                        minimum_depth = current_depth;
                        best = right;
                    }
                    micro_[Offset(shape, left, right)] = static_cast<std::uint8_t>(best);
                }
            }
        }
    }

    [[nodiscard]] int WithinBlock(std::size_t left, std::size_t right) const
    {
        const auto first = left / block_size_;
        const auto offset = Offset(signatures_[first], left % block_size_, right % block_size_);
        return static_cast<int>(first * block_size_ + micro_[offset]);
    }

    [[nodiscard]] int EarlierMinimum(int first, int second) const
    {
        return std::pair{depths_[first], first} <= std::pair{depths_[second], second} ? first : second;
    }
};
} // namespace detail

class LinearLCA
{
public:
    explicit LinearLCA(std::span<const int> parents)
    {
        const int n = detail::checked_linear_lca_count(parents.size());
        std::vector<int> child(static_cast<std::size_t>(n) + 1, -1), sibling(n, -1);
        for (int vertex = n; vertex > 0;)
        {
            --vertex;
            const int parent = parents[vertex];
            if (parent < -1 || parent >= n)
            {
                throw std::out_of_range("LinearLCA: invalid parent");
            }
            const int owner = parent == -1 ? n : parent;
            sibling[vertex] = child[owner];
            child[owner] = vertex;
        }
        const auto expected = static_cast<std::size_t>(2 * n + 1);
        std::vector<int> pending{n}, depths{0};
        pending.reserve(parents.size() + 1);
        depths.reserve(expected);
        tour_.reserve(expected);
        tour_.push_back(-1);
        first_.resize(n);

        while (!pending.empty())
        {
            const int vertex = pending.back();
            const int next = child[vertex];
            if (next != -1)
            {
                child[vertex] = sibling[next];
                first_[next] = static_cast<int>(tour_.size());
                tour_.push_back(next);
                depths.push_back(static_cast<int>(pending.size()));
                pending.push_back(next);
            }
            else
            {
                pending.pop_back();
                if (!pending.empty())
                {
                    const int previous = pending.back();
                    tour_.push_back(previous == n ? -1 : previous);
                    depths.push_back(static_cast<int>(pending.size() - 1));
                }
            }
        }
        if (tour_.size() != expected)
        {
            throw std::invalid_argument("LinearLCA: cycle in parent input");
        }
        rmq_.build(std::move(depths));
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(first_.size());
    }

    [[nodiscard]] int get(int u, int v) const
    {
        CheckVertex(u);
        CheckVertex(v);
        const auto [left, right] = std::minmax(first_[u], first_[v]);
        return tour_[rmq_.index(left, right)];
    }

    [[nodiscard]] int operator()(int u, int v) const
    {
        return get(u, v);
    }

    [[nodiscard]] int depth_of(int vertex) const
    {
        CheckVertex(vertex);
        return rmq_.depth(first_[vertex]) - 1;
    }

    [[nodiscard]] int get_distance(int u, int v) const
    {
        const int ancestor = get(u, v);
        if (ancestor == -1)
        {
            return -1;
        }
        const int depth = depth_of(ancestor);
        return (depth_of(u) - depth) + (depth_of(v) - depth);
    }

private:
    detail::LinearLcaRmq rmq_;
    std::vector<int> tour_;
    std::vector<int> first_;

    void CheckVertex(int vertex) const
    {
        if (static_cast<std::size_t>(vertex) >= first_.size())
        {
            throw std::out_of_range("LinearLCA: vertex");
        }
    }
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
