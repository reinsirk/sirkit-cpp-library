#ifndef SIRKIT_INCREMENTAL_MINIMUM_SPANNING_FOREST_DEFINED
#define SIRKIT_INCREMENTAL_MINIMUM_SPANNING_FOREST_DEFINED

struct IncrementalMsfInsertion
{
    std::size_t edge_id;
    bool selected;
    std::optional<std::size_t> removed_edge;
};

namespace detail
{
struct IncrementalMsfPeak
{
    std::int64_t weight = std::numeric_limits<std::int64_t>::min();
    int slot = -1;
};

struct IncrementalMsfPeakMaximum
{
    IncrementalMsfPeak operator()(const IncrementalMsfPeak &left, const IncrementalMsfPeak &right) const noexcept
    {
        if (left.weight != right.weight)
        {
            return left.weight > right.weight ? left : right;
        }
        // INT64_MIN の重みでも、実際の辺が頂点に置いた単位元より優先される。
        return left.slot > right.slot ? left : right;
    }
};

class IncrementalMsfWeight
{
    std::uint64_t low_ = 0;
    std::int64_t high_ = 0;

public:
    void add(std::int64_t weight) noexcept
    {
        const auto low = low_ + static_cast<std::uint64_t>(weight);
        high_ += static_cast<int>(low < low_) - static_cast<int>(weight < 0);
        low_ = low;
    }

    void remove(std::int64_t weight) noexcept
    {
        const auto bits = static_cast<std::uint64_t>(weight);
        high_ += static_cast<int>(weight < 0) - static_cast<int>(low_ < bits);
        low_ -= bits;
    }

    std::int64_t value() const
    {
        const auto weight = static_cast<std::int64_t>(low_);
        if (high_ != -static_cast<int>(weight < 0))
        {
            throw std::overflow_error("IncrementalMinimumSpanningForest: total weight does not fit int64_t");
        }
        return weight;
    }
};

inline std::size_t incremental_msf_node_count(int n)
{
    constexpr auto kMaxNodes = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (n < 0 || static_cast<std::size_t>(n) > (kMaxNodes + 1) / 2)
    {
        throw std::length_error("IncrementalMinimumSpanningForest: vertex count exceeds capacity");
    }
    return n == 0 ? 0 : static_cast<std::size_t>(n) * 2 - 1;
}

inline std::size_t take_incremental_msf_edge_id(std::size_t &edge_count)
{
    if (edge_count == std::numeric_limits<std::size_t>::max())
    {
        throw std::overflow_error("IncrementalMinimumSpanningForest: edge ID exhausted");
    }
    return edge_count++;
}
} // namespace detail

class IncrementalMinimumSpanningForest
{
    using Peak = detail::IncrementalMsfPeak;

    struct Edge
    {
        int from;
        int to;
        std::size_t id;
    };

    struct State
    {
        int vertices;
        int components;
        std::size_t edges = 0;
        detail::IncrementalMsfWeight total;
    };

    LinkCutTree<Peak, detail::IncrementalMsfPeakMaximum> forest_;
    std::vector<Edge> slots_;
    State state_;

    void attach(int slot, int from, int to, std::int64_t weight, std::size_t id)
    {
        const int node = state_.vertices + slot;
        slots_[slot] = {from, to, id};
        forest_.set(node, Peak{weight, slot});
        static_cast<void>(forest_.link(from, node));
        static_cast<void>(forest_.link(node, to));
    }

public:
    explicit IncrementalMinimumSpanningForest(int n)
        : forest_(std::vector<Peak>(detail::incremental_msf_node_count(n))),
          slots_(forest_.size() / 2), state_{n, n, 0, {}}
    {
    }

    IncrementalMinimumSpanningForest(const IncrementalMinimumSpanningForest &) = default;

    IncrementalMinimumSpanningForest(IncrementalMinimumSpanningForest &&other) noexcept
        : forest_(std::move(other.forest_)), slots_(std::move(other.slots_)), state_(std::exchange(other.state_, {}))
    {
    }

    IncrementalMinimumSpanningForest &operator=(IncrementalMinimumSpanningForest other) noexcept
    {
        forest_.swap(other.forest_);
        slots_.swap(other.slots_);
        std::swap(state_, other.state_);
        return *this;
    }

    int size() const noexcept
    {
        return state_.vertices;
    }

    std::size_t edge_count() const noexcept
    {
        return state_.edges;
    }

    int component_count() const noexcept
    {
        return state_.components;
    }

    std::int64_t total_weight() const
    {
        return state_.total.value();
    }

    IncrementalMsfInsertion add_edge(int from, int to, std::int64_t weight)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= size() || to < 0 || to >= size())
        {
            throw std::out_of_range("IncrementalMinimumSpanningForest: edge endpoint out of range");
        }
#endif
        const auto id = detail::take_incremental_msf_edge_id(state_.edges);
        if (from == to)
        {
            return {id, false, std::nullopt};
        }

        const auto peak = forest_.path_prod(from, to);
        if (!peak)
        {
            const int slot = state_.vertices - state_.components;
            attach(slot, from, to, weight, id);
            --state_.components;
            state_.total.add(weight);
            return {id, true, std::nullopt};
        }
        if (weight >= peak->weight)
        {
            return {id, false, std::nullopt};
        }

        // 閉路上で最も重い既存辺を外して、その枠を新しい辺に使う。
        const auto previous = slots_[peak->slot];
        const int node = state_.vertices + peak->slot;
        static_cast<void>(forest_.cut(node, previous.from));
        static_cast<void>(forest_.cut(node, previous.to));
        attach(peak->slot, from, to, weight, id);
        state_.total.remove(peak->weight);
        state_.total.add(weight);
        return {id, true, previous.id};
    }
};

#endif
