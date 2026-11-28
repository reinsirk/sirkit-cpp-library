// SPDX-License-Identifier: CC0-1.0
#include <atcoder/modint>
#include <string>
#include "Verify/current/common/fast_io.hpp"
#include "Verify/current/common/wildcard_construction.hpp"

int main()
{
    std::string text, x, y;
    verify_in >> text >> x >> y;
    const auto dfa = verify_wildcard::automaton(x, y);
    verify_out << dfa.count_matches<atcoder::modint998244353>(verify_wildcard::masks(text)).val() << '\n';
}
