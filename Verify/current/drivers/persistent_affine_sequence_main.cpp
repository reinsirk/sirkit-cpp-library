// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <atcoder/modint>
#include <vector>

int main()
{
    using Mint = atcoder::modint998244353;
    using Sequence = sirkit::PersistentAffineSequence<Mint>;
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<Mint> values(n);
    for (auto &value : values)
    {
        int x = 0;
        verify_in >> x;
        value = Mint::raw(x);
    }
    std::vector<Sequence> versions(q + 1);
    versions[0] = Sequence(values);
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, base = 0;
        verify_in >> type >> base;
        ++base;
        int source = 0;
        if (type == 1)
        {
            verify_in >> source;
            ++source;
        }
        int left = 0, right = 0;
        verify_in >> left >> right;
        if (type == 0)
        {
            int b = 0, c = 0;
            verify_in >> b >> c;
            versions[i] = versions[base].affine(left, right, Mint::raw(b), Mint::raw(c));
        }
        else if (type == 1)
        {
            versions[i] = versions[base].copy_from(left, right, versions[source]);
        }
        else
        {
            verify_out << versions[base].sum(left, right).val() << '\n';
        }
    }
}
