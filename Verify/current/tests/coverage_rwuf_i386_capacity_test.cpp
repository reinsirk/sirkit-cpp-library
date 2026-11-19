// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/rollback-union-findw.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using Weight = std::int64_t;
using UnionFind = sirkit::RollbackWeightedUnionFind<Weight>;

template<class Operation>
void rejects_capacity(Operation operation) {
    try {
        operation();
    } catch (const std::length_error&) {
        return;
    }
    assert(false);
}

} // namespace

int main() {
    static_assert(sizeof(std::size_t) == 4 && sizeof(int) == 4);
    const auto parent_limit = std::vector<int>{}.max_size();
    const auto weight_limit = std::vector<Weight>{}.max_size();
    const auto history_limit = std::vector<std::array<int, 2>>{}.max_size();
    static_assert(sizeof(std::array<int, 2>) == 2 * sizeof(int));
    assert(weight_limit < parent_limit && parent_limit < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    assert(history_limit < static_cast<std::size_t>(std::numeric_limits<int>::max()));

    // 上限を超える最小の頂点数を渡し、巨大配列の確保前に拒否させる。
    const int oversized_parents = static_cast<int>(parent_limit + 1);
    const int oversized_weights = static_cast<int>(weight_limit + 1);
    rejects_capacity([&] { (void)UnionFind(oversized_parents); });
    rejects_capacity([&] { (void)UnionFind(oversized_weights); });

    UnionFind uf(3);
    assert(uf.unite(0, 1, 37));
    uf.snapshot();
    rejects_capacity([&] { uf.reset(oversized_parents); });
    rejects_capacity([&] { uf.reset(oversized_weights); });
    rejects_capacity([&] { uf.reserve_history(history_limit + 1); });
    assert(uf.size() == 3 && uf.get_state() == 1 && uf.diff(0, 1) == 37);
    assert(uf.unite(1, 2, -4));
    assert(uf.diff(0, 2) == 33);
    uf.rollback();
    assert(!uf.same(0, 2) && uf.diff(0, 1) == 37);
}
