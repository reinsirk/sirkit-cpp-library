// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Resource_Constrained_Paths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/resource_constrained_paths.md
//
// API一覧
// ResourcePathArc{from,to,spend,gain,time}: 資源をspend消費しgain得る有向辺。timeは所要時間。
// ResourceConstrainedPaths(n,edges,source,capacity,initial): 所持量が上限capacityの最短路を構築。
//   初期値と補充は上限で止まる。資源がspend未満なら辺を通れない。頂点は0始まり。
//   資源量・時間はuint64_t。時間が表現可能な経路だけ扱う。
// minimum_time(v,required=0): 資源をrequired以上持って到着する最短時間。なければnullopt。O(log(k+2))。
// labels(v): (残量,時間)のうち他の組に支配されないものを残量昇順で参照する。kはその組数。
//   上限をCとし、最悪 O(n+(m+1)(C+1) log((m+1)(C+1)+2)) 時間、O((n+m)(C+1)) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_RESOURCE_CONSTRAINED_PATHS_DEFINED
#define SIRKIT_RESOURCE_CONSTRAINED_PATHS_DEFINED

struct ResourcePathArc
{
    int from;
    int to;
    std::uint64_t spend;
    std::uint64_t gain;
    std::uint64_t time;
};

struct ResourcePathLabel
{
    std::uint64_t remaining;
    std::uint64_t time;
};

class ResourceConstrainedPaths
{
public:
    ResourceConstrainedPaths(
        int n, std::span<const ResourcePathArc> edges, int source, std::uint64_t capacity, std::uint64_t initial)
    {
        if (n < 0)
        {
            throw std::length_error("ResourceConstrainedPaths: negative vertex count");
        }
        labels_.resize(n);
        check_vertex(source);

        std::vector<std::vector<std::size_t>> outgoing(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            check_vertex(edges[i].from);
            check_vertex(edges[i].to);
            outgoing[edges[i].from].push_back(i);
        }

        using Key = std::tuple<std::uint64_t, std::uint64_t, int>;
        std::priority_queue<Key, std::vector<Key>, std::greater<Key>> pending;
        std::vector<std::map<std::uint64_t, std::uint64_t>> frontier(n);
        auto enqueue = [&](int v, std::uint64_t remaining, std::uint64_t time)
        {
            auto &labels = frontier[v];
            auto after = labels.lower_bound(remaining);
            if (after != labels.end())
            {
                if (after->second <= time)
                {
                    return;
                }
                if (after->first == remaining)
                {
                    after = labels.erase(after);
                }
            }
            auto first = after;
            while (first != labels.begin())
            {
                const auto before = std::prev(first);
                if (before->second < time)
                {
                    break;
                }
                first = before;
            }
            const auto hint = labels.erase(first, after);
            labels.emplace_hint(hint, remaining, time);
            pending.emplace(time, capacity - remaining, v);
        };
        enqueue(source, std::min(capacity, initial), 0);
        while (!pending.empty())
        {
            const auto [time, used, v] = pending.top();
            pending.pop();
            const auto remaining = capacity - used;
            const auto found = frontier[v].find(remaining);
            if (found == frontier[v].end() || found->second != time)
            {
                continue;
            }
            for (const auto id : outgoing[v])
            {
                const auto &edge = edges[id];
                if (remaining < edge.spend || edge.time > std::numeric_limits<std::uint64_t>::max() - time)
                {
                    continue;
                }
                const auto rest = remaining - edge.spend;
                const auto next = rest + std::min(edge.gain, capacity - rest);
                enqueue(edge.to, next, time + edge.time);
            }
        }

        for (int v = 0; v < n; ++v)
        {
            labels_[v].reserve(frontier[v].size());
            for (const auto &[remaining, time] : frontier[v])
            {
                labels_[v].push_back({remaining, time});
            }
        }
    }

    [[nodiscard]] std::span<const ResourcePathLabel> labels(int v) const
    {
        check_vertex(v);
        return labels_[v];
    }

    [[nodiscard]] std::optional<std::uint64_t> minimum_time(int v, std::uint64_t required = 0) const
    {
        const auto options = labels(v);
        const auto found = std::ranges::lower_bound(options, required, {}, &ResourcePathLabel::remaining);
        if (found == options.end())
        {
            return std::nullopt;
        }
        return found->time;
    }

private:
    void check_vertex([[maybe_unused]] int v) const
    {
    }

    std::vector<std::vector<ResourcePathLabel>> labels_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
