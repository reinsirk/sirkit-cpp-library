// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    unsigned n, q;
    verify_in >> n >> q;
    using Trie = sirkit::PersistentBinaryTrie<std::uint32_t, 29>;
    Trie trie(n);
    std::vector<Trie::Version> prefix(n + 1);
    for (unsigned i = 0; i < n; ++i)
    {
        std::uint32_t value;
        verify_in >> value;
        prefix[i + 1] = trie.insert(prefix[i], value);
    }
    while (q-- > 0)
    {
        unsigned first, last;
        std::uint32_t k;
        verify_in >> first >> last >> k;
        verify_out << *trie.kth_xor(prefix[first], prefix[last], k) << '\n';
    }
}
