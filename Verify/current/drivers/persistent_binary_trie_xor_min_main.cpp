// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>

int main()
{
    unsigned q;
    verify_in >> q;
    using Trie = sirkit::PersistentBinaryTrie<std::uint32_t, 29>;
    Trie trie(q);
    Trie::Version current;
    while (q-- > 0)
    {
        int type;
        std::uint32_t value;
        verify_in >> type >> value;
        if (type == 0)
        {
            if (trie.count(current, value) == 0)
            {
                current = trie.insert(current, value);
            }
        }
        else if (type == 1)
        {
            if (const auto next = trie.erase(current, value))
            {
                current = *next;
            }
        }
        else
        {
            verify_out << *trie.kth_xor({}, current, 0, value) << '\n';
        }
    }
}
