#include <numeric>

#ifndef SIRKIT_SUFFIX_TREE_DEFINED
#define SIRKIT_SUFFIX_TREE_DEFINED

namespace detail
{
inline void suffix_tree_length_check(std::size_t length)
{
    constexpr auto kLimit = static_cast<std::size_t>((std::numeric_limits<int>::max() - 1) / 2);
    if (length > kLimit)
    {
        throw std::length_error("SuffixTree: text too long");
    }
}
} // namespace detail

class SuffixTree
{
  public:
    struct Node
    {
        int parent;
        int depth;
        int suffix_start;
        int first_suffix;
        int last_suffix;
    };

    struct Repeat
    {
        int first_start = 0;
        int second_start = 0;
        int length = 0;
    };

    explicit SuffixTree(std::string_view text) : data_(std::make_shared<const Data>(text))
    {
    }

    SuffixTree(const SuffixTree &) noexcept = default;
    SuffixTree &operator=(const SuffixTree &) noexcept = default;

    [[nodiscard]] std::span<const Node> nodes() const noexcept
    {
        return data_->nodes;
    }

    [[nodiscard]] std::span<const int> children(int vertex) const
    {
        return data_->children_of(vertex_index(vertex));
    }

    [[nodiscard]] std::span<const int> suffixes(int vertex) const
    {
        const auto &node = data_->nodes[vertex_index(vertex)];
        const auto first = static_cast<std::size_t>(node.first_suffix);
        const auto count = static_cast<std::size_t>(node.last_suffix - node.first_suffix);
        return std::span<const int>(data_->suffix_array_order).subspan(first, count);
    }

    [[nodiscard]] std::string_view edge_label(int vertex) const
    {
        const auto &node = data_->nodes[vertex_index(vertex)];
        if (vertex == 0)
        {
            return {};
        }
        const int parent_depth = data_->nodes[node.parent].depth;
        const int representative = data_->suffix_array_order[node.first_suffix];
        return std::string_view(data_->text).substr(
            static_cast<std::size_t>(representative + parent_depth),
            static_cast<std::size_t>(node.depth - parent_depth));
    }

    [[nodiscard]] Repeat longest_nonoverlapping_repeat() const
    {
        const auto &data = *data_;
        std::vector<std::size_t> order;
        order.reserve(data.nodes.size());
        order.push_back(0);
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            for (const int child : data.children_of(order[i]))
            {
                order.push_back(static_cast<std::size_t>(child));
            }
        }

        std::vector<std::pair<int, int>> span(data.nodes.size());
        Repeat best;
        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const auto vertex = *it;
            const auto &node = data.nodes[vertex];
            int first = static_cast<int>(data.text.size());
            int last = 0;
            if (node.suffix_start >= 0)
            {
                first = last = node.suffix_start;
            }
            for (const int child : data.children_of(vertex))
            {
                first = std::min(first, span[child].first);
                last = std::max(last, span[child].second);
            }
            span[vertex] = {first, last};
            // 同じ接頭辞を持つうち、距離が最大の二つの出現を使う。
            const int length = std::min(node.depth, last - first);
            if (length > best.length)
            {
                best = {first, last, length};
            }
        }
        return best;
    }

  private:
    struct Data
    {
        explicit Data(std::string_view source)
        {
            detail::suffix_tree_length_check(source.size());
            text.assign(source);
            suffix_array_order = suffix_array(text);
            const auto lcp = lcp_array(text, suffix_array_order);
            const int length = static_cast<int>(text.size());
            nodes.reserve(text.size() + 1);
            nodes.push_back({-1, 0, -1, 0, length});
            std::vector<int> open{0};
            for (int rank = 0; rank < length; ++rank)
            {
                const int common = rank == 0 ? 0 : lcp[rank - 1];
                int child = 0;
                while (nodes[open.back()].depth > common)
                {
                    child = open.back();
                    nodes[child].last_suffix = rank;
                    open.pop_back();
                }
                int parent = open.back();
                if (nodes[parent].depth < common)
                {
                    // 前の枝の途中で新しい接尾辞と分かれる。その位置に頂点を挿入する。
                    const int split = static_cast<int>(nodes.size());
                    nodes.push_back({parent, common, -1, nodes[child].first_suffix, length});
                    nodes[child].parent = split;
                    open.push_back(split);
                    parent = split;
                }
                const int endpoint = static_cast<int>(nodes.size());
                const int start = suffix_array_order[rank];
                nodes.push_back({parent, length - start, start, rank, length});
                open.push_back(endpoint);
            }

            child_offsets.resize(nodes.size() + 1);
            for (std::size_t i = 1; i < nodes.size(); ++i)
            {
                ++child_offsets[static_cast<std::size_t>(nodes[i].parent) + 1];
            }
            std::partial_sum(child_offsets.begin(), child_offsets.end(), child_offsets.begin());
            auto next = child_offsets;
            edges.resize(nodes.size() - 1);
            for (std::size_t i = 1; i < nodes.size(); ++i)
            {
                edges[next[nodes[i].parent]++] = static_cast<int>(i);
            }
        }

        [[nodiscard]] std::span<const int> children_of(std::size_t vertex) const noexcept
        {
            const auto first = child_offsets[vertex];
            const auto count = child_offsets[vertex + 1] - first;
            return std::span<const int>(edges).subspan(first, count);
        }

        std::string text;
        std::vector<int> suffix_array_order;
        std::vector<Node> nodes;
        std::vector<std::size_t> child_offsets;
        std::vector<int> edges;
    };

    [[nodiscard]] std::size_t vertex_index(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= data_->nodes.size())
        {
            throw std::out_of_range("SuffixTree: vertex index");
        }
#endif
        return static_cast<std::size_t>(vertex);
    }

    std::shared_ptr<const Data> data_;
};

#endif
