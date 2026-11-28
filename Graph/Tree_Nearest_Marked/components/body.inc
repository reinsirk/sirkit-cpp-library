#ifndef SIRKIT_TREE_NEAREST_MARKED_DEFINED
#define SIRKIT_TREE_NEAREST_MARKED_DEFINED

class TreeNearestMarked
{
public:
    struct Result
    {
        int vertex;
        int distance;
        friend constexpr bool operator==(const Result &a, const Result &b) noexcept
        {
            return a.vertex == b.vertex && a.distance == b.distance;
        }
    };

    explicit TreeNearestMarked(const Graph &graph)
    {
        const CentroidDecomposition decomposition(graph);
        const auto n = decomposition.size();
        marked_.resize(n);
        visits_begin_.resize(n + 1);
        tree_begin_.resize(n + 1);
        std::vector<std::uint32_t> next_leaf(n);
        for (std::size_t v = 0; v < n; ++v)
        {
            const auto count = static_cast<std::uint32_t>(decomposition.component_size(static_cast<int>(v)));
            next_leaf[v] = count;
            tree_begin_[v + 1] = tree_begin_[v] + 2 * static_cast<std::size_t>(count);
        }
        minima_.assign(tree_begin_.back(), kUnmarked);
        visits_.reserve(tree_begin_.back() / 2);
        for (std::size_t v = 0; v < n; ++v)
        {
            visits_begin_[v] = visits_.size();
            for (const auto &[centroid, distance] : decomposition.ancestors(static_cast<int>(v)))
            {
                visits_.push_back({centroid, distance, next_leaf[centroid]++});
            }
        }
        visits_begin_[n] = visits_.size();
    }

    TreeNearestMarked(const TreeNearestMarked &) = default;
    TreeNearestMarked(TreeNearestMarked &&) noexcept = default;

    TreeNearestMarked &operator=(TreeNearestMarked other) noexcept
    {
        marked_.swap(other.marked_);
        visits_begin_.swap(other.visits_begin_);
        tree_begin_.swap(other.tree_begin_);
        visits_.swap(other.visits_);
        minima_.swap(other.minima_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return marked_.size();
    }

    [[nodiscard]] bool marked(int v) const
    {
        check_vertex(v);
        return marked_[v] != 0;
    }

    void set_marked(int v, bool marked = true)
    {
        check_vertex(v);
        write_marked(v, marked);
    }

    bool toggle(int v)
    {
        check_vertex(v);
        const bool marked = marked_[v] == 0;
        write_marked(v, marked);
        return marked;
    }

    [[nodiscard]] std::optional<Result> nearest(int v) const
    {
        check_vertex(v);
        auto best = kUnmarked;
        for (const auto &[centroid, distance, leaf] : visits(v))
        {
            const auto nearest_to_centroid = minima_[tree_begin_[centroid] + 1];
            if (nearest_to_centroid != kUnmarked)
            {
                best = std::min(best, nearest_to_centroid + pack_distance(distance));
            }
        }
        if (best == kUnmarked)
        {
            return std::nullopt;
        }
        return Result{static_cast<int>(static_cast<std::uint32_t>(best)), static_cast<int>(best >> 32)};
    }

private:
    struct Visit
    {
        int centroid;
        int distance;
        std::uint32_t leaf;
    };
    static constexpr auto kUnmarked = std::numeric_limits<std::uint64_t>::max();

    static std::uint64_t pack_distance(int distance) noexcept
    {
        return static_cast<std::uint64_t>(distance) << 32;
    }

    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= size())
        {
            throw std::out_of_range("TreeNearestMarked: vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    std::span<const Visit> visits(int v) const noexcept
    {
        const auto first = visits_begin_[v];
        return std::span<const Visit>(visits_).subspan(first, visits_begin_[v + 1] - first);
    }

    void write_marked(int v, bool marked) noexcept
    {
        if (marked_[v] == marked)
        {
            return;
        }
        marked_[v] = marked;
        for (const auto &[centroid, distance, leaf] : visits(v))
        {
            // 上位32ビットを距離、下位を頂点番号にすると最小値だけで同距離の判定ができる。
            const auto packed = marked ? pack_distance(distance) + static_cast<std::uint32_t>(v) : kUnmarked;
            const auto begin = tree_begin_[centroid];
            minima_[begin + leaf] = packed;
            for (auto node = leaf / 2; node != 0; node /= 2)
            {
                const auto next = std::min(minima_[begin + 2 * node], minima_[begin + 2 * node + 1]);
                if (minima_[begin + node] == next)
                {
                    break;
                }
                minima_[begin + node] = next;
            }
        }
    }

    std::vector<std::uint8_t> marked_;
    std::vector<std::size_t> visits_begin_, tree_begin_;
    std::vector<Visit> visits_;
    std::vector<std::uint64_t> minima_;
};

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
