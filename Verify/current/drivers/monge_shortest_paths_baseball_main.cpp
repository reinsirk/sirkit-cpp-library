// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <compare>
#include <cstdint>
#include <numeric>
#include <vector>

namespace
{
struct State
{
    std::int64_t value;
    int edges;

    State operator+(State other) const
    {
        return {value + other.value, edges + other.edges};
    }

    auto operator<=>(const State &) const = default;
};
}

int main()
{
    int n{}, k{};
    verify_in >> n >> k;
    std::vector<std::int64_t> sum(n + 1), moment(n + 1);
    for (int i = 1; i <= n; ++i)
    {
        std::int64_t p{};
        verify_in >> p;
        sum[i] = sum[i - 1] + p;
        moment[i] = moment[i - 1] + p * i;
    }

    const auto evaluate = [&](std::int64_t penalty) {
        const auto result = sirkit::monge_shortest_paths(n + 1, [&](int from, int to) -> State {
            if (from == 0)
            {
                return {penalty + to * sum[to - 1] - moment[to - 1], 1};
            }
            const int mid = (from + to) / 2;
            const auto left = (moment[mid] - moment[from]) - from * (sum[mid] - sum[from]);
            const auto right = to * (sum[to - 1] - sum[mid]) - (moment[to - 1] - moment[mid]);
            return {penalty + left + right, 1};
        });
        // 番兵の終点へは、選んだ位置を1個以上通ってから到達する。
        const auto suffix = [&](int from) -> State {
            return {penalty + moment[n] - moment[from] - from * (sum[n] - sum[from]), 1};
        };
        State best = result.distances[1] + suffix(1);
        for (int from = 2; from <= n; ++from)
        {
            const State value = result.distances[from] + suffix(from);
            if (value < best)
            {
                best = value;
            }
        }
        return best;
    };

    std::int64_t low = -1, high = sum[n] * n + 1;
    while (high - low > 1)
    {
        const auto mid = std::midpoint(low, high);
        if (evaluate(mid).edges <= k + 1)
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }
    verify_out << evaluate(high).value - (k + 1) * high << '\n';
}
