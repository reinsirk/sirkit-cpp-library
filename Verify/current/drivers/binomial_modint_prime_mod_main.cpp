// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <atcoder/modint>

#include <array>
namespace {
template<class Mint>
void answer_binomial_queries(const std::vector<std::pair<std::uint32_t, std::uint32_t>>& queries, std::uint32_t max_n, const BinomialOptions& options) {
    ModIntBinomial<Mint> comb(max_n, options);
    const std::size_t batch = max_n < (std::uint32_t{1} << 20) ? 1 : 256;
    std::array<std::uint32_t, 256> answers;
    for (std::size_t first = 0; first < queries.size(); first += batch) {
        const auto count = std::min(batch, queries.size() - first);
        for (std::size_t i = 0; i < count; ++i) {
            const auto& [n, k] = queries[first + i];
            answers[i] = comb.COM(n, k).val();
        }
        for (std::size_t i = 0; i < count; ++i) verify_out << answers[i] << '\n';
    }
}
} // namespace
int main() {
    int q{}; std::uint32_t modulus{}; verify_in >> q >> modulus;
    using Mint = atcoder::dynamic_modint<0>;
    Mint::set_mod(static_cast<int>(modulus));
    std::vector<std::pair<std::uint32_t, std::uint32_t>> queries(q);
    std::uint32_t max_n = 0;
    for (auto& [n, k] : queries) { verify_in >> n >> k; max_n = std::max(max_n, n); }
    BinomialOptions options; options.table_limit = 9'999'999;
    if (modulus == 998244353) answer_binomial_queries<atcoder::static_modint<998244353>>(queries, max_n, options);
    else if (modulus == 1000000007) answer_binomial_queries<atcoder::static_modint<1000000007>>(queries, max_n, options);
    else answer_binomial_queries<Mint>(queries, max_n, options);
}
