// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

namespace
{
struct ModularAdd
{
    int modulus;

    int operator()(int a, int b) const
    {
        const int sum = a + b;
        return sum < modulus ? sum : sum - modulus;
    }
};

struct ModularSubtract
{
    int modulus;

    int operator()(int a, int b) const
    {
        const int difference = a - b;
        return difference >= 0 ? difference : difference + modulus;
    }
};
} // namespace

int main()
{
    int n = 0, modulus = 0, q = 0;
    verify_in >> n >> modulus >> q;
    using Weighted =
        sirkit::PersistentWeightedUnionFind<int, ModularAdd, ModularSubtract>;
    Weighted uf(n + 1, 0, ModularAdd{modulus}, ModularSubtract{modulus});
    std::vector<Weighted::Version> versions(q + 1, uf.version());
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, base = 0, l = 0, r = 0;
        verify_in >> type >> base >> l >> r;
        if (type == 0)
        {
            int score = 0;
            verify_in >> score;
            const auto status = uf.unite_checked(l - 1, r, score, versions[base]);
            verify_out << (status == Weighted::MergeResult::Contradiction ? "NO\n"
                                                                          : "YES\n");
            versions[i] = uf.version();
        }
        else
        {
            const auto value = uf.try_diff(l - 1, r, versions[base]);
            if (value)
            {
                verify_out << *value << '\n';
            }
            else
            {
                verify_out << "UNKNOWN\n";
            }
            versions[i] = versions[base];
        }
    }
}
