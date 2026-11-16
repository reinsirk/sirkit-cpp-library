// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main() {
    int n, q; verify_in >> n >> q;
    // N<=200000、typeは0/1なのでuの上位1bitへ格納できる。
    struct Query { std::uint32_t encoded_u = 0; int v = 0, head = -1, next = 0; };
    std::vector<Query> queries(q + 1);
    for (int i = 1; i <= q; ++i) {
        int k; std::uint32_t type, u; auto& query = queries[i];
        verify_in >> type >> k >> u >> query.v;
        query.encoded_u = u | (type << 31);
        query.next = queries[k + 1].head; queries[k + 1].head = i;
    }
    RollbackWeightedUnionFind<std::int64_t> uf(n);
    struct Frame { int vertex, state; bool exit; };
    std::vector<Frame> stack{{0, 0, false}};
    while (!stack.empty()) {
        const auto frame = stack.back(); stack.pop_back();
        if (frame.exit) {
            uf.rollback(frame.state);
            continue;
        }
        const int first = queries[frame.vertex].head;
        if (frame.vertex != 0) {
            auto& query = queries[frame.vertex];
            if ((query.encoded_u >> 31) == 0) {
                if (first == -1) continue;
                const int state = uf.get_state();
                uf.unite(static_cast<int>(query.encoded_u & 0x7fffffffu), query.v, std::int64_t{0});
                stack.push_back({frame.vertex, state, true});
            } else query.v = uf.same(static_cast<int>(query.encoded_u & 0x7fffffffu), query.v);
        }
        for (int child = first; child != -1; child = queries[child].next) stack.push_back({child, 0, false});
    }
    // type1の版は参照されないため、vは判定後に答えとして再利用する。
    for (int i = 1; i <= q; ++i)
        if (queries[i].encoded_u >> 31) verify_out << queries[i].v << '\n';
}
