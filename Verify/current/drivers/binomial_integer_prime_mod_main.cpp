// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>

int main() {
    int q{}; std::uint32_t modulus{}; verify_in >> q >> modulus;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(q);
    std::uint32_t max_n = 0;
    for (auto& [n, k] : queries) { verify_in >> n >> k; max_n = std::max(max_n, n); }
    BinomialOptions options; options.table_limit = 9'999'999;
    BinomialMod comb(modulus, max_n, options);
    for (const auto& [n, k] : queries) verify_out << comb.COM(n, k) << '\n';
}
