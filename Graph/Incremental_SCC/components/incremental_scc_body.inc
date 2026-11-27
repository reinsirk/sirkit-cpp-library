#ifndef SIRKIT_INCREMENTAL_SCC_DEFINED
#define SIRKIT_INCREMENTAL_SCC_DEFINED

struct IncrementalSccMerge
{
    std::size_t time;
    int leader;
    int absorbed;
};

namespace detail
{
struct IncrementalSccEdge
{
    std::size_t inserted;
    int from;
    int to;
};

class IncrementalSccSolver
{
    std::vector<int> index_, vertices_, representative_;
    std::vector<std::pair<int, int>> prefix_;
    UnionFind groups_;
    std::vector<IncrementalSccMerge> events_;

    int index(int vertex)
    {
        auto &position = index_[vertex];
        if (position == -1)
        {
            position = static_cast<int>(vertices_.size());
            vertices_.push_back(vertex);
        }
        return position;
    }

    auto label_prefix(std::span<const IncrementalSccEdge> edges, std::size_t time)
        -> std::pair<int, std::vector<int>>
    {
        prefix_.clear();
        for (const auto &[inserted, from, to] : edges)
        {
            const int left = index(from);
            const int right = index(to);
            if (inserted <= time)
            {
                prefix_.emplace_back(left, right);
            }
        }
        return directed_component_ids(static_cast<int>(vertices_.size()), prefix_);
    }

    void clear_indices()
    {
        for (int v : vertices_)
        {
            index_[v] = -1;
        }
        vertices_.clear();
    }

    std::pair<std::size_t, std::size_t> split(std::span<IncrementalSccEdge> edges, std::size_t mid)
    {
        const auto [count, ids] = label_prefix(edges, mid);
        if (static_cast<std::size_t>(count) == vertices_.size())
        {
            clear_indices();
            return {0, edges.size()};
        }

        const auto same = [&](const IncrementalSccEdge &edge)
        {
            return ids[index_[edge.from]] == ids[index_[edge.to]];
        };
        // 左に残すのは、midですでに追加されている成分内の辺だけ。
        const auto middle = std::partition(edges.begin(), edges.end(), [&](const auto &edge)
                                           {
                                               return edge.inserted <= mid && same(edge);
                                           });
        // midの成分内にあって、追加がこれからの辺は以後の成分を変えない。
        const auto kept_end = std::remove_if(middle, edges.end(), same);

        representative_.resize(count);
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            representative_[ids[i]] = vertices_[i];
        }
        // 右へ送る辺はmidの成分代表を端点に使う。
        for (auto it = middle; it != kept_end; ++it)
        {
            it->from = representative_[ids[index_[it->from]]];
            it->to = representative_[ids[index_[it->to]]];
        }
        clear_indices();
        return {static_cast<std::size_t>(middle - edges.begin()), static_cast<std::size_t>(kept_end - edges.begin())};
    }

    void solve(std::span<IncrementalSccEdge> edges, std::size_t first, std::size_t last)
    {
        if (edges.empty())
        {
            return;
        }
        if (first == last)
        {
            for (const auto &edge : edges)
            {
                const int from = groups_.root(edge.from);
                const int to = groups_.root(edge.to);
                if (groups_.merge(from, to))
                {
                    const int leader = groups_.root(from);
                    events_.push_back({first, leader, leader == from ? to : from});
                }
            }
            return;
        }

        const auto mid = first + (last - first) / 2;
        const auto [left, kept] = split(edges, mid);
        solve(edges.first(left), first, mid);
        solve(edges.subspan(left, kept - left), mid + 1, last);
    }

public:
    explicit IncrementalSccSolver(int n)
        : index_(n, -1), groups_(n)
    {
    }

    std::vector<IncrementalSccMerge> run(std::span<IncrementalSccEdge> edges, std::size_t last)
    {
        // 最終的に異なる成分を結ぶ辺は、どの時刻の閉路にも入らない。
        const auto [count, final] = label_prefix(edges, last);
        static_cast<void>(count);
        const auto end = std::remove_if(edges.begin(), edges.end(), [&](const auto &edge)
                                        {
                                            return final[index_[edge.from]] != final[index_[edge.to]];
                                        });
        const auto kept = static_cast<std::size_t>(end - edges.begin());
        clear_indices();
        events_.reserve(std::min(kept, index_.size() - 1));
        solve(edges.first(kept), 1, last);
        return std::move(events_);
    }
};
} // namespace detail

inline std::vector<IncrementalSccMerge> incremental_scc_merges(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("incremental_scc_merges: negative vertex count");
    }
    std::vector<detail::IncrementalSccEdge> work;
    work.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("incremental_scc_merges: edge endpoint out of range");
        }
#endif
        if (from != to)
        {
            work.push_back({i + 1, from, to});
        }
    }
    if (work.empty())
    {
        return {};
    }
    detail::IncrementalSccSolver solver(n);
    return solver.run(work, edges.size());
}

#endif
