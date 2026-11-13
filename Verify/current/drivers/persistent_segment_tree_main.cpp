// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <functional>
#include <vector>
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0, q = 0; input >> n >> q;
    std::vector<std::int64_t> values(static_cast<std::size_t>(n));
    for (auto& value : values) input >> value;
    PersistentSegmentTree<std::int64_t, std::plus<std::int64_t>> tree(values, {}, 0);
    auto root = tree.latest_root();
    for (int query = 0; query < q; ++query) {
        int type = 0; input >> type;
        if (type == 0) {
            int position = 0; std::int64_t delta = 0; input >> position >> delta;
            root = tree.transform(root, static_cast<std::size_t>(position), [delta](auto x) { return x + delta; });
        } else {
            int left = 0, right = 0; input >> left >> right;
            output << tree.prod(root, static_cast<std::size_t>(left), static_cast<std::size_t>(right)) << '\n';
        }
    }
}
