#ifndef SIRKIT_DYNAMIC_COLOR_COMPONENT_SUM_DEFINED
#define SIRKIT_DYNAMIC_COLOR_COMPONENT_SUM_DEFINED

namespace detail
{
inline void check_color_component_capacity(std::size_t n)
{
    // 差分列の末尾に1要素を置く。
    if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("DynamicColorComponentSum: too many vertices");
    }
}
}

class DynamicColorComponentSum
{
public:
    using Edge = std::pair<int, int>;

    DynamicColorComponentSum(std::span<const std::int64_t> weights, std::span<const int> colors, std::span<const Edge> edges)
    {
        const std::size_t n = weights.size();
        detail::check_color_component_capacity(n);
#ifndef SKIP_RUNTIME_CHECKS
        if (colors.size() != n)
        {
            throw std::invalid_argument("DynamicColorComponentSum: invalid color count");
        }
#endif
        layout_.build(MakeGraph(n, edges));
        weights_.reserve(n);
        colors_.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            CheckColor(colors[i]);
            weights_.push_back(static_cast<Storage>(weights[i]));
            colors_.push_back(static_cast<unsigned char>(colors[i]));
        }

        std::array<std::vector<Storage>, 2> initial{weights_, weights_};
        const auto &order = layout_.order();
        for (std::size_t i = n; i > 1; --i)
        {
            const int v = order[i - 1];
            auto &potential = initial[colors_[v]];
            potential[layout_.parent(v)] += potential[v];
        }
        std::vector<int> counts(n);
        std::vector<Storage> differences(n + 1);
        for (int color = 0; color < 2; ++color)
        {
            Storage previous{};
            for (std::size_t i = 0; i < n; ++i)
            {
                const int v = order[i];
                counts[i] = colors_[v] == color;
                const Storage sum = initial[color][v];
                differences[i] = sum - previous;
                previous = sum;
            }
            differences[n] = Storage{} - previous;
            occurrences_[color] = fenwick_tree<int>(counts);
            potentials_[color] = fenwick_tree<Storage>(differences);
        }
    }

    DynamicColorComponentSum(const DynamicColorComponentSum &) = default;

    DynamicColorComponentSum(DynamicColorComponentSum &&other) noexcept
    {
        Swap(other);
    }

    DynamicColorComponentSum &operator=(DynamicColorComponentSum other) noexcept
    {
        Swap(other);
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return layout_.size();
    }

    [[nodiscard]] int color(int vertex) const
    {
        CheckVertex(vertex);
        return colors_[vertex];
    }

    [[nodiscard]] std::int64_t weight(int vertex) const
    {
        CheckVertex(vertex);
        return static_cast<std::int64_t>(weights_[vertex]);
    }

    void set_color(int vertex, int new_color)
    {
        CheckVertex(vertex);
        CheckColor(new_color);
        if (colors_[vertex] != new_color)
        {
            Recolor(vertex, new_color);
        }
    }

    int toggle(int vertex)
    {
        CheckVertex(vertex);
        const int next = colors_[vertex] ^ 1;
        Recolor(vertex, next);
        return next;
    }

    void set_weight(int vertex, std::int64_t value)
    {
        CheckVertex(vertex);
        AddWeight(vertex, static_cast<Storage>(value) - weights_[vertex]);
    }

    void add_weight(int vertex, std::int64_t delta)
    {
        CheckVertex(vertex);
        AddWeight(vertex, static_cast<Storage>(delta));
    }

    [[nodiscard]] std::int64_t component_sum(int vertex) const
    {
        CheckVertex(vertex);
        const int color = colors_[vertex];
        return static_cast<std::int64_t>(Potential(Highest(vertex, color), color));
    }

private:
    using Storage = std::uint64_t;

    HeavyLightDecomposition layout_;
    std::vector<Storage> weights_;
    std::vector<unsigned char> colors_;
    // fenwick_tree::sumはACL互換の宣言上は非constだが、照会で内容を変えない。
    mutable std::array<fenwick_tree<int>, 2> occurrences_;
    mutable std::array<fenwick_tree<Storage>, 2> potentials_;

    static Graph MakeGraph(std::size_t n, std::span<const Edge> edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (edges.size() != n - (n != 0))
        {
            throw std::invalid_argument("DynamicColorComponentSum: invalid edge count");
        }
#endif
        std::vector<std::size_t> degree(n);
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (u < 0 || static_cast<std::size_t>(u) >= n || v < 0 || static_cast<std::size_t>(v) >= n)
            {
                throw std::out_of_range("DynamicColorComponentSum: invalid edge endpoint");
            }
#endif
            ++degree[u];
            ++degree[v];
        }
        Graph graph(n);
        for (std::size_t v = 0; v < n; ++v)
        {
            graph[v].reserve(degree[v]);
        }
        for (const auto &[u, v] : edges)
        {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        return graph;
    }

    void CheckVertex(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= size())
        {
            throw std::out_of_range("DynamicColorComponentSum: invalid vertex");
        }
#else
        (void)vertex;
#endif
    }

    static void CheckColor(int color)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<unsigned>(color) > 1)
        {
            throw std::invalid_argument("DynamicColorComponentSum: invalid color");
        }
#else
        (void)color;
#endif
    }

    void Swap(DynamicColorComponentSum &other) noexcept
    {
        layout_.swap(other.layout_);
        weights_.swap(other.weights_);
        colors_.swap(other.colors_);
        occurrences_.swap(other.occurrences_);
        potentials_.swap(other.potentials_);
    }

    int Highest(int vertex, int color) const
    {
        auto &opposite = occurrences_[color ^ 1];
        int highest = vertex;
        bool stopped = false;
        layout_.for_each_path(vertex, 0, [&](int first, int last, bool) {
            if (stopped)
            {
                return;
            }
            if (colors_[layout_.vertex_at(last - 1)] != color)
            {
                stopped = true;
                return;
            }
            if (last - first > 1)
            {
                const int rank = opposite.sum(0, last);
                if (rank != 0)
                {
                    const int before = opposite.lower_bound(rank);
                    if (before >= first)
                    {
                        highest = layout_.vertex_at(before + 1);
                        stopped = true;
                        return;
                    }
                }
            }
            highest = layout_.vertex_at(first);
        });
        return highest;
    }

    Storage Potential(int vertex, int color) const
    {
        return potentials_[color].sum(0, layout_.index(vertex) + 1);
    }

    void AddRange(int first, int last, int color, Storage delta)
    {
        potentials_[color].add(first, delta);
        potentials_[color].add(last, Storage{} - delta);
    }

    void AddAncestorPath(int vertex, int color, Storage delta)
    {
        // 最初に色が異なる祖先の状態にも加え、その上へは伝えない。
        int limit = vertex;
        if (colors_[vertex] == color)
        {
            const int top = Highest(vertex, color);
            const int parent = layout_.parent(top);
            limit = parent == -1 ? top : parent;
        }
        if (limit == vertex)
        {
            const int index = layout_.index(vertex);
            AddRange(index, index + 1, color, delta);
            return;
        }
        layout_.for_each_path(vertex, limit, [&](int first, int last, bool) {
            AddRange(first, last, color, delta);
        });
    }

    void Recolor(int vertex, int new_color)
    {
        const int old_color = colors_[vertex];
        const int parent = layout_.parent(vertex);
        if (parent != -1)
        {
            AddAncestorPath(parent, old_color, Storage{} - Potential(vertex, old_color));
            AddAncestorPath(parent, new_color, Potential(vertex, new_color));
        }
        const int position = layout_.index(vertex);
        occurrences_[old_color].add(position, -1);
        occurrences_[new_color].add(position, 1);
        colors_[vertex] = static_cast<unsigned char>(new_color);
    }

    void AddWeight(int vertex, Storage delta)
    {
        const int color = colors_[vertex];
        const int position = layout_.index(vertex);
        weights_[vertex] += delta;
        AddAncestorPath(vertex, color, delta);
        AddRange(position, position + 1, color ^ 1, delta);
    }
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
