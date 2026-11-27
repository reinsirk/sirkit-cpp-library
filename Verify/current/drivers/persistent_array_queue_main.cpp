// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>

int main()
{
    int q{};
    verify_in >> q;
    using Array = PersistentArray<std::int32_t, 3>;
    struct Version
    {
        Array::Node *root = nullptr;
        int head = 0;
        int tail = 0;
    };
    Array entries(q);
    std::vector<Version> versions(q + 1);
    for (int i = 1; i <= q; ++i)
    {
        int type{}, previous{};
        verify_in >> type >> previous;
        auto version = versions[previous + 1];
        if (type == 0)
        {
            std::int32_t value{};
            verify_in >> value;
            version.root = entries.update(version.root, version.tail++, value);
        }
        else
        {
            verify_out << entries.get(version.root, version.head++) << '\n';
        }
        versions[i] = version;
    }
}
