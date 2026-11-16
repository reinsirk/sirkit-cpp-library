// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>
#include <string>
#include <vector>

// 区間和をS[R]-S[L-1]へ変換し、版木のDFSで制約を追加・復元する。
// 合成数Kにも加減算だけで対応。O(N+Q log(N+1))時間、O(N+Q)領域。
int main() {
    int n{}, modulus{}, q{}; verify_in >> n >> modulus >> q;
    using Mint = atcoder::dynamic_modint<0>;
    using UnionFind = RollbackWeightedUnionFind<Mint>;
    Mint::set_mod(modulus);

    struct Query { int type = 0, left = 0, right = 0, weight = 0; };
    std::vector<Query> queries(q + 1);
    std::vector<std::vector<int>> children(q + 1);
    for (int i = 1; i <= q; ++i) {
        int base{}; auto& query = queries[i];
        verify_in >> query.type >> base >> query.left >> query.right;
        --query.left;
        if (query.type == 0) verify_in >> query.weight;
        children[base].push_back(i);
    }

    UnionFind uf(n + 1);
    uf.reserve_history(q);
    std::vector<std::string> answers(q + 1);
    struct Frame { int version, state; bool exit; };
    std::vector<Frame> stack;
    stack.reserve(q + 1);
    stack.push_back({0, 0, false});
    while (!stack.empty()) {
        const auto frame = stack.back(); stack.pop_back();
        if (frame.exit) {
            uf.rollback(frame.state);
            continue;
        }
        const int state = uf.get_state();
        if (frame.version != 0) {
            const auto& query = queries[frame.version];
            if (query.type == 0) {
                const auto result = uf.unite_checked(query.left, query.right, Mint(query.weight));
                answers[frame.version] = result == UnionFind::MergeResult::Contradiction ? "NO" : "YES";
            } else {
                const auto difference = uf.try_diff(query.left, query.right);
                answers[frame.version] = difference ? std::to_string(difference->val()) : "UNKNOWN";
            }
        }
        // 却下された主張も版を作る。子を処理した後、冗長・矛盾の履歴も戻す。
        stack.push_back({frame.version, state, true});
        for (int child : children[frame.version]) stack.push_back({child, 0, false});
    }
    for (int i = 1; i <= q; ++i) verify_out << answers[i] << '\n';
}
