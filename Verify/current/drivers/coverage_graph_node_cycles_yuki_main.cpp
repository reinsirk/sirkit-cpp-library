// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <memory>
#include <vector>
int main() {
    int n{}; verify_in >> n;
    std::vector<int> next(n), visited(n);
    std::vector<std::shared_ptr<Node>> nodes(n);
    for (auto& node : nodes) node = make_node();
    for (int i = 0; i < n; ++i) { verify_in >> next[i]; --next[i]; nodes[i]->n = nodes[next[i]]; }
    int answer = 0;
    for (int start = 0; start < n; ++start) {
        if (visited[start]) continue;
        const int stamp = start + 1;
        int vertex = start, previous = -1;
        while (!visited[vertex]) { visited[vertex] = stamp; previous = vertex; vertex = next[vertex]; }
        // 既に処理した接尾列への接続を切り、各ノードの走査を定数回にする。
        if (visited[vertex] != stamp) nodes[previous]->n.reset();
        answer += break_cycle(nodes[start]);
    }
    verify_out << answer << '\n';
}
