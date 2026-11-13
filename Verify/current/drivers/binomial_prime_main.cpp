// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>
int main() {
    int q{}; std::uint32_t modulus{}; verify_in >> q >> modulus;
    using Mint = atcoder::dynamic_modint<0>;
    Mint::set_mod(static_cast<int>(modulus));
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(q);
    std::uint32_t max_n = 0;
    for (auto& [n, k] : queries) { verify_in >> n >> k; max_n = std::max(max_n, n); }
    PrimeBinomial<Mint> comb(max_n);
    for (const auto& [n, k] : queries) verify_out << comb.COM(n, k).val() << '\n';
}
