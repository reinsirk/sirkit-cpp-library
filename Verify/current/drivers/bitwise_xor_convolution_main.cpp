// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <atcoder/modint>
#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    unsigned dimension{};
    verify_in >> dimension;
    const auto size = std::size_t{1} << dimension;
    using Mint = atcoder::modint998244353;
    std::vector<Mint> left(size), right(size);
    for (auto &value : left)
    {
        int input{};
        verify_in >> input;
        value = Mint::raw(input);
    }
    for (auto &value : right)
    {
        int input{};
        verify_in >> input;
        value = Mint::raw(input);
    }
    const auto result = sirkit::bitwise_xor_convolution(std::move(left), std::move(right));
    for (std::size_t i = 0; i < size; ++i)
    {
        verify_out << result[i].val() << (i + 1 == size ? '\n' : ' ');
    }
}
