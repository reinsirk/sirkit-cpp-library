// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>
#include <vector>

// 同値/反対の条件はmod 2の差。整合なら成分ごとに2通りある。
// 全消去はrollback(0)。復元回数の総和は追加数以下なので、全体で
// O(N+Q log(N+1))時間・O(N+Q)領域。各消去でN頂点を初期化し直さない。
int main() {
    int n{}, q{}; verify_in >> n >> q;
    using Parity = atcoder::static_modint<2>;
    using Count = atcoder::modint998244353;
    using UnionFind = RollbackWeightedUnionFind<Parity>;
    std::vector<Count> powers(n + 1);
    powers[0] = 1;
    for (int i = 1; i <= n; ++i) powers[i] = powers[i - 1] + powers[i - 1];
    UnionFind uf(n);
    uf.reserve_history(q);
    bool contradiction = false;
    while (q--) {
        int type{}; verify_in >> type;
        if (type == 3) {
            uf.rollback(0);
            contradiction = false;
        } else {
            int u{}, v{}; verify_in >> u >> v;
            const auto result = uf.unite_checked(u - 1, v - 1, Parity(type - 1));
            contradiction = contradiction || result == UnionFind::MergeResult::Contradiction;
        }
        verify_out << (contradiction ? 0 : powers[uf.component_count()].val()) << '\n';
    }
}
