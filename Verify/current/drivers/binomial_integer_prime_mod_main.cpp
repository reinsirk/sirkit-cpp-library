// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>

#include <array>
int main() {
    int q{}; std::uint32_t modulus{}; verify_in >> q >> modulus;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(q);
    std::uint32_t max_n = 0;
    for (auto& [n, k] : queries) { verify_in >> n >> k; max_n = std::max(max_n, n); }
    BinomialOptions options; options.table_limit = 9'999'999;
    BinomialMod comb(modulus, max_n, options);
    const std::size_t batch = max_n < (std::uint32_t{1} << 20) ? 1 : 256;
    std::array<std::uint32_t, 256> answers;
    for (std::size_t first = 0; first < queries.size(); first += batch) {
        const auto count = std::min(batch, queries.size() - first);
        for (std::size_t i = 0; i < count; ++i) {
            const auto& [n, k] = queries[first + i];
            answers[i] = comb.COM(n, k);
        }
        for (std::size_t i = 0; i < count; ++i) verify_out << answers[i] << '\n';
    }
}
