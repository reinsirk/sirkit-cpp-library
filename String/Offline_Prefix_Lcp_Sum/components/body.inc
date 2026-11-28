#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

#ifndef SIRKIT_OFFLINE_PREFIX_LCP_SUM_DEFINED
#define SIRKIT_OFFLINE_PREFIX_LCP_SUM_DEFINED

struct PrefixLcpAppend
{
    std::size_t index;
    std::uint8_t byte;
};

struct PrefixLcpQuery
{
    std::size_t index;
};

using PrefixLcpOperation = std::variant<PrefixLcpAppend, PrefixLcpQuery>;

namespace reinsirk::detail
{
struct PrefixLcpNode
{
    int parent = 0;
    int child = 0;
    int sibling = 0;
    std::uint8_t byte = 0;
};

inline int prefix_lcp_checked_count(std::size_t count)
{
    if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("offline_prefix_lcp_sum: too many strings or trie nodes");
    }
    return static_cast<int>(count);
}

inline int prefix_lcp_extend(std::vector<PrefixLcpNode> &nodes, int parent, std::uint8_t byte)
{
    for (auto index = nodes[parent].child; index != 0; index = nodes[index].sibling)
    {
        if (nodes[index].byte == byte)
        {
            return index;
        }
    }
    const auto next = prefix_lcp_checked_count(nodes.size() + 1) - 1;
    nodes.push_back({parent, 0, nodes[parent].child, byte});
    nodes[parent].child = next;
    return next;
}
} // namespace detail

inline std::vector<std::uint64_t> offline_prefix_lcp_sum(std::span<const std::string_view> initial,
                                                          std::span<const PrefixLcpOperation> operations)
{
    (void)detail::prefix_lcp_checked_count(initial.size());
    std::vector<detail::PrefixLcpNode> nodes(1);
    std::vector<int> current(initial.size());
    for (std::size_t index = 0; index < initial.size(); ++index)
    {
        for (const char byte : initial[index])
        {
            current[index] = detail::prefix_lcp_extend(nodes, current[index], static_cast<std::uint8_t>(byte));
        }
    }
    const auto starts = current;

    struct Event
    {
        int node;
        bool append;
    };
    std::vector<Event> events;
    events.reserve(operations.size());
    for (const auto &operation : operations)
    {
        const auto index = std::visit([](const auto &event) { return event.index; }, operation);
#ifndef SKIP_RUNTIME_CHECKS
        if (index >= current.size())
        {
            throw std::out_of_range("offline_prefix_lcp_sum: string index");
        }
#endif
        if (const auto *append = std::get_if<PrefixLcpAppend>(&operation))
        {
            current[index] = detail::prefix_lcp_extend(nodes, current[index], append->byte);
            events.push_back({current[index], true});
        }
        else
        {
            events.push_back({current[index], false});
        }
    }

    const auto size = static_cast<int>(nodes.size());
    std::vector<std::uint64_t> frequency(size), base_sum(size);
    std::vector<int> subtree_size(size, 1);
    for (const auto node : starts)
    {
        ++frequency[node];
    }
    for (auto node = size - 1; node > 0; --node)
    {
        const auto parent = nodes[node].parent;
        frequency[parent] += frequency[node];
        subtree_size[parent] += subtree_size[node];
    }
    for (int node = 1; node < size; ++node)
    {
        base_sum[node] = base_sum[nodes[node].parent] + frequency[node];
    }

    std::vector<int> entry(size), pending{0};
    int order = 0;
    while (!pending.empty())
    {
        const auto node = pending.back();
        pending.pop_back();
        entry[node] = order++;
        for (auto child = nodes[node].child; child != 0; child = nodes[child].sibling)
        {
            pending.push_back(child);
        }
    }

    // 1字の追加で増える接頭辞の種類は一つ。その頂点の部分木の答えを+1する。
    fenwick_tree<std::int64_t> changes(size);
    std::vector<std::uint64_t> answer;
    for (const auto [node, append] : events)
    {
        const auto start = entry[node];
        if (append)
        {
            changes.add(start, 1);
            const auto end = start + subtree_size[node];
            if (end < size)
            {
                changes.add(end, -1);
            }
        }
        else
        {
            answer.push_back(base_sum[node] + static_cast<std::uint64_t>(changes.sum(0, start + 1)));
        }
    }
    return answer;
}

#endif
