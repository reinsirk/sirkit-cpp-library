#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_HUB_SPARSE_DISTANCES
#define SIRKIT_HUB_SPARSE_DISTANCES

class HubSparseDistances
{
public:
    struct Shortcut
    {
        int from, to;
        std::uint64_t cost;
    };

    HubSparseDistances(std::span<const std::uint64_t> gates, std::span<const Shortcut> shortcuts)
    {
        UnionFind components(gates.size());
        component_.resize(gates.size());
        for (const auto &shortcut : shortcuts)
        {
            check_vertex(shortcut.from);
            check_vertex(shortcut.to);
            if (shortcut.cost == 0)
            {
                components.unite(shortcut.from, shortcut.to);
            }
        }
        gate_distance_.assign(gates.size(), kMaxGate);
        for (std::size_t v = 0; v < gates.size(); ++v)
        {
            if (gates[v] > kMaxGate)
            {
                throw std::invalid_argument("HubSparseDistances: gate cost too large");
            }
            const auto root = components.root(static_cast<int>(v));
            component_[v] = root;
            gate_distance_[root] = std::min(gate_distance_[root], gates[v]);
        }
        core_index_.resize(gates.size());
        // 行列の添字0をハブに、1以降を残った短縮辺の端点に割り当てる。
        std::vector<std::uint64_t> core_gates{0};
        for (const auto &shortcut : shortcuts)
        {
            const auto from = component_[shortcut.from];
            const auto to = component_[shortcut.to];
            if (from == to || shortcut.cost >= gate_distance_[from] + gate_distance_[to])
            {
                continue;
            }
            for (const auto vertex : {from, to})
            {
                if (core_index_[vertex] == 0)
                {
                    core_index_[vertex] = core_gates.size();
                    core_gates.push_back(gate_distance_[vertex]);
                }
            }
        }
        side_ = core_gates.size();
        distances_.resize(side_ * side_);
        for (std::size_t i = 0; i < side_; ++i)
        {
            for (std::size_t j = i + 1; j < side_; ++j)
            {
                distances_[i * side_ + j] = distances_[j * side_ + i] = core_gates[i] + core_gates[j];
            }
        }
        for (const auto &shortcut : shortcuts)
        {
            const auto from = core_index_[component_[shortcut.from]];
            const auto to = core_index_[component_[shortcut.to]];
            if (from != 0 && to != 0)
            {
                auto &d = distances_[from * side_ + to];
                d = std::min(d, shortcut.cost);
                distances_[to * side_ + from] = d;
            }
        }
        for (std::size_t pivot = 1; pivot < side_; ++pivot)
        {
            for (std::size_t from = 0; from < side_; ++from)
            {
                const auto head = distances_[from * side_ + pivot];
                for (std::size_t to = 0; to < side_; ++to)
                {
                    const auto tail = distances_[pivot * side_ + to];
                    auto &direct = distances_[from * side_ + to];
                    // 比較が先なので、head+tailがuint64_tの上限を超える場合も安全。
                    if (head < direct && tail < direct - head)
                    {
                        direct = head + tail;
                    }
                }
            }
        }
        for (std::size_t v = 0; v < gates.size(); ++v)
        {
            if (core_index_[v] != 0)
            {
                gate_distance_[v] = distances_[core_index_[v]];
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return component_.size();
    }

    [[nodiscard]] std::uint64_t distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto a = component_[from];
        const auto b = component_[to];
        if (a == b)
        {
            return 0;
        }
        const auto i = core_index_[a];
        const auto j = core_index_[b];
        if (i != 0 && j != 0)
        {
            return distances_[i * side_ + j];
        }
        return gate_distance_[a] + gate_distance_[b];
    }

private:
    static constexpr auto kMaxGate = std::numeric_limits<std::uint64_t>::max() / 2;
    std::size_t side_ = 0;
    std::vector<int> component_;
    std::vector<std::size_t> core_index_;
    std::vector<std::uint64_t> gate_distance_, distances_;

    void check_vertex(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(vertex) >= size())
        {
            throw std::out_of_range("HubSparseDistances: vertex out of range");
        }
#else
        static_cast<void>(vertex);
#endif
    }
};
#endif
