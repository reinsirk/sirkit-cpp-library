// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
int main() {
    int q{}; verify_in >> q;
    BinaryTrie<std::uint32_t, 29> values;
    std::vector<std::uint32_t> original(q);
    std::uint32_t mask = 0;
    for (int id = 0; id < q; ++id) {
        int type{}; std::uint32_t x{}; verify_in >> type >> x;
        if (type == 0) { original[id] = x; values.insert_unique(x ^ mask, id); }
        else if (type == 1) {
            const auto found = values.find(x ^ mask);
            if (found.first) { const int erased_id = found.second.front(); values.del(x ^ mask, erased_id); }
        } else {
            // 照会ごとにXOR後の座標系へ切り替え、最小要素のIDから元の値を復元する。
            values.operate_xor(mask ^ x); mask = x;
            const auto best = values.min_element();
            verify_out << (original[best.second.front()] ^ x) << '\n';
        }
    }
}
