#ifndef SIRKIT_INCREMENTAL_APSP_DEFINED
#define SIRKIT_INCREMENTAL_APSP_DEFINED

struct IncrementalAPSPEdge
{
    int from, to;
    std::uint32_t weight;
};

class IncrementalAPSP
{
public:
    explicit IncrementalAPSP(int n, std::span<const IncrementalAPSPEdge> edges = {}, bool undirected = false)
        : undirected_(undirected)
    {
        if (n < 0)
        {
            throw std::invalid_argument("incremental APSP: negative vertex count");
        }
        distance_.assign(n, std::vector<std::int64_t>(n, kAbsent));
        tail_.resize(n);
        for (int vertex = 0; vertex < n; ++vertex)
        {
            distance_[vertex][vertex] = 0;
        }
        for (const auto &[u, v, weight] : edges)
        {
            check_vertex(u);
            check_vertex(v);
            const auto w = static_cast<std::int64_t>(weight);
            distance_[u][v] = std::min(distance_[u][v], w);
            if (undirected)
            {
                distance_[v][u] = std::min(distance_[v][u], w);
            }
        }
        if (!edges.empty())
        {
            FloydWarshall(distance_, kAbsent);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return distance_.size();
    }

    void add_edge(int from, int to, std::uint32_t weight)
    {
        check_vertex(from);
        check_vertex(to);
        add_arc(from, to, weight);
        if (undirected_)
        {
            add_arc(to, from, weight);
        }
    }

    [[nodiscard]] std::optional<std::int64_t> distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto value = distance_[from][to];
        if (value == kAbsent)
        {
            return std::nullopt;
        }
        return value;
    }

private:
    static constexpr auto kAbsent = std::numeric_limits<std::int64_t>::max();

    void check_vertex([[maybe_unused]] int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
        {
            throw std::out_of_range("incremental APSP: vertex out of range");
        }
#endif
    }

    void add_arc(int from, int to, std::int64_t weight)
    {
        if (distance_[from][to] <= weight)
        {
            return;
        }
        // 新しい辺を使う最短路の後半を、行を書き換える前に保存する。
        std::ranges::copy(distance_[to], tail_.begin());
        for (auto &row : distance_)
        {
            if (row[from] >= kAbsent - weight)
            {
                continue;
            }
            const auto prefix = row[from] + weight;
            const auto limit = kAbsent - prefix;
            for (std::size_t j = 0; j < size(); ++j)
            {
                if (tail_[j] < limit)
                {
                    row[j] = std::min(row[j], prefix + tail_[j]);
                }
            }
        }
    }

    bool undirected_;
    std::vector<std::vector<std::int64_t>> distance_;
    std::vector<std::int64_t> tail_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
