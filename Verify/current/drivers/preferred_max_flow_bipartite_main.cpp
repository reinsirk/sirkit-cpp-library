// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

int main() {
    int left{}, right{}, m{};
    if (!(verify_in >> left >> right >> m)) return 0;
    // 辺に比べて頂点が多いときは、マッチングへ参加できない孤立点を除く。
    const bool compress = left + right > 8 * m;
    std::vector<std::pair<int, int>> input;
    std::vector<int> left_ids, right_ids;
    if (compress) {
        input.resize(m); left_ids.reserve(m); right_ids.reserve(m);
        for (auto& [from, to] : input) {
            verify_in >> from >> to;
            left_ids.push_back(from); right_ids.push_back(to);
        }
        std::sort(left_ids.begin(), left_ids.end());
        left_ids.erase(std::unique(left_ids.begin(), left_ids.end()), left_ids.end());
        std::sort(right_ids.begin(), right_ids.end());
        right_ids.erase(std::unique(right_ids.begin(), right_ids.end()), right_ids.end());
        left = static_cast<int>(left_ids.size()); right = static_cast<int>(right_ids.size());
    }
    const int source = left + right, sink = source + 1;
    mf_graph<int> network(sink + 1);
    network.reserve_edges(static_cast<std::size_t>(left) + right + m);
    for (int i = 0; i < left; ++i) network.add_edge(source, i, 1);
    for (int i = 0; i < right; ++i) network.add_edge(left + i, sink, 1);
    const int first = left + right;
    for (int i = 0; i < m; ++i) {
        int from{}, to{};
        if (compress) {
            from = static_cast<int>(std::lower_bound(left_ids.begin(), left_ids.end(), input[i].first) - left_ids.begin());
            to = static_cast<int>(std::lower_bound(right_ids.begin(), right_ids.end(), input[i].second) - right_ids.begin());
        } else verify_in >> from >> to;
        network.add_edge(from, left + to, 1);
    }
    verify_out << network.flow(source, sink, std::min(left, right)) << '\n';
    for (int i = 0; i < m; ++i) {
        const auto edge = network.get_edge(first + i);
        if (edge.flow) {
            if (compress) verify_out << left_ids[edge.from] << ' ' << right_ids[edge.to - left] << '\n';
            else verify_out << edge.from << ' ' << edge.to - left << '\n';
        }
    }
}
