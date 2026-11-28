// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
struct BinaryPolynomial
{
    std::uint64_t bits{};

    BinaryPolynomial &operator+=(BinaryPolynomial right)
    {
        bits ^= right.bits;
        return *this;
    }
    BinaryPolynomial &operator-=(BinaryPolynomial right)
    {
        return *this += right;
    }
    BinaryPolynomial &operator*=(BinaryPolynomial right)
    {
        // F_2[x] の積は次数62以下なので、既約多項式での剰余を取っても変化しない。
        bits = sirkit::multiply_f2_64(bits, right.bits);
        return *this;
    }
};
} // namespace

int main()
{
    unsigned dimensions{};
    verify_in >> dimensions;
    const auto length = std::size_t{1} << dimensions;
    std::vector<BinaryPolynomial> left(length), right(length);
    for (auto *input : {&left, &right})
    {
        for (auto &polynomial : *input)
        {
            for (unsigned bit = 0; bit < 32; ++bit)
            {
                std::uint64_t coefficient{};
                verify_in >> coefficient;
                polynomial.bits |= coefficient << bit;
            }
        }
    }
    auto product = sirkit::xor_convolution_without_division<true>(std::move(left), std::move(right));
    for (const auto polynomial : product)
    {
        for (unsigned bit = 0; bit < 63; ++bit)
        {
            verify_out << static_cast<char>('0' + ((polynomial.bits >> bit) & 1u));
            verify_out << (bit == 62 ? '\n' : ' ');
        }
    }
}
