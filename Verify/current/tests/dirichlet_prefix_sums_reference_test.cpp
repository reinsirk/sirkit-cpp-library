// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Dirichlet_Prefix_Sums.cpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
std::vector<int> Quotients(int n)
{
    std::vector<int> values;
    for (int i = n; i >= 1; --i)
    {
        const auto quotient = n / i;
        if (values.empty() || values.back() != quotient)
        {
            values.push_back(quotient);
        }
    }
    return values;
}

std::vector<std::int64_t> PrefixTable(const std::vector<int> &quotients, const std::vector<std::int64_t> &values)
{
    std::vector<std::int64_t> out;
    std::size_t i = 1;
    std::int64_t sum = 0;
    for (int quotient : quotients)
    {
        while (i <= static_cast<std::size_t>(quotient))
        {
            sum += values[i++];
        }
        out.push_back(sum);
    }
    return out;
}

std::vector<std::int64_t> DirectProduct(const std::vector<std::int64_t> &f, const std::vector<std::int64_t> &g)
{
    std::vector<std::int64_t> out(f.size());
    for (std::size_t i = 1; i < f.size(); ++i)
    {
        for (std::size_t j = 1; i * j < f.size(); ++j)
        {
            out[i * j] += f[i] * g[j];
        }
    }
    return out;
}

void CheckRandom(int n, std::mt19937 &random)
{
    const auto quotients = Quotients(n);
    const sirkit::DirichletPrefixSums<std::int64_t> layout(n);
    assert(layout.size() == quotients.size());
    for (std::size_t index = 0; index < quotients.size(); ++index)
    {
        assert(layout.quotient(index) == static_cast<std::uint64_t>(quotients[index]));
    }
    std::vector<std::int64_t> f(n + 1), g(n + 1);
    for (int i = 1; i <= n; ++i)
    {
        f[i] = static_cast<int>(random() % 7) - 3;
        g[i] = static_cast<int>(random() % 7) - 3;
    }
    f[1] = n % 2 == 0 ? 1 : -1;
    const auto f_table = PrefixTable(quotients, f);
    const auto g_table = PrefixTable(quotients, g);
    const auto product = PrefixTable(quotients, DirectProduct(f, g));
    assert(layout.convolution(f_table, g_table) == product);
    assert(layout.divide(product, f_table, f[1]) == g_table);
    const auto inverse = layout.inverse(f_table, f[1]);
    assert(layout.convolution(f_table, inverse) == std::vector<std::int64_t>(layout.size(), 1));
}
}

int main()
{
    std::mt19937 random(21181);
    for (int n = 1; n <= 300; ++n)
    {
        CheckRandom(n, random);
    }
    for (int n : {1023, 1024, 1025, 4095, 4096, 4097, 100003})
    {
        CheckRandom(n, random);
    }
    using Layout = sirkit::DirichletPrefixSums<std::int64_t>;
    const Layout largest(std::numeric_limits<std::uint64_t>::max());
    assert(largest.size() == 8589934590ULL);
    assert(largest.quotient(largest.size() - 1) == std::numeric_limits<std::uint64_t>::max());
    assert(largest.quotient(4294967294ULL) == 4294967295ULL);
    assert(largest.quotient(4294967295ULL) == 4294967297ULL);
#ifndef SKIP_RUNTIME_CHECKS
    const Layout layout(5);
    try
    {
        static_cast<void>(Layout(0));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        static_cast<void>(layout.quotient(layout.size()));
        assert(false);
    }
    catch (const std::out_of_range &)
    {
    }
    try
    {
        static_cast<void>(layout.convolution({}, {}));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    try
    {
        const std::vector<std::int64_t> wrong_inverse(layout.size(), 2);
        static_cast<void>(layout.inverse(wrong_inverse, 1));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
