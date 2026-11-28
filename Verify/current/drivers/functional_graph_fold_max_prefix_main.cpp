// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int main()
{
    int n{};
    std::uint64_t k{};
    verify_in >> n >> k;
    std::vector<int> next(n);
    std::vector<std::int64_t> score(n);
    for (auto &v : next)
    {
        verify_in >> v;
        --v;
    }
    for (auto &value : score)
    {
        verify_in >> value;
    }
    using Value = std::pair<std::int64_t, std::int64_t>;
    const auto combine = [](const Value &left, const Value &right)
    {
        return Value{left.first + right.first, std::max(left.second, left.first + right.second)};
    };
    std::vector<Value> edge(n);
    for (int v = 0; v < n; ++v)
    {
        const auto value = score[next[v]];
        edge[v] = {value, std::max(std::int64_t{}, value)};
    }
    const sirkit::FunctionalGraphFold<Value, decltype(combine)> moves(next, edge, {0, 0}, combine);
    auto best = std::numeric_limits<std::int64_t>::min();
    for (int v = 0; v < n; ++v)
    {
        best = std::max(best, score[next[v]] + moves.walk(next[v], k - 1).second.second);
    }
    verify_out << best << '\n';
}
