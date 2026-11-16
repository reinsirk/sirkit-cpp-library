// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto& value : values) verify_in >> value;
    auto keys = values;
    std::sort(keys.begin(), keys.end()); keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    for (auto& value : values) value = static_cast<int>(std::lower_bound(keys.begin(), keys.end(), value) - keys.begin());
    Mo queries(n);
    for (int i = 0; i < q; ++i) { int l{}, r{}; verify_in >> l >> r; queries.add_query(l, r); }
    // 独立した頻度集計。左側と右側で転倒数の増減が異なる。
    std::vector<int> frequency(keys.size() + 1);
    const auto prefix = [&](int end) { int total = 0; for (; end; end &= end - 1) total += frequency[end]; return total; };
    const auto update = [&](int value, int delta) { for (auto i = static_cast<std::size_t>(value + 1); i < frequency.size(); i += i & -i) frequency[i] += delta; };
    std::int64_t inversions = 0;
    int active = 0;
    std::vector<std::int64_t> answers(q);
    const int block = std::max(1, static_cast<int>(n * std::sqrt(2.0 / std::max(1, q))));
    queries.run(
        [&](int i) { inversions += prefix(values[i]); update(values[i], 1); ++active; },
        [&](int i) { inversions += active - prefix(values[i] + 1); update(values[i], 1); ++active; },
        [&](int i) { update(values[i], -1); --active; inversions -= prefix(values[i]); },
        [&](int i) { update(values[i], -1); --active; inversions -= active - prefix(values[i] + 1); },
        [&](std::size_t id) { answers[id] = inversions; }, block);
    for (const auto answer : answers) verify_out << answer << '\n';
}
