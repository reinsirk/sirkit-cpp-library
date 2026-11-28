// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Mixed_Radix.cpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

int main()
{
    using boost::multiprecision::cpp_int;
    using Number = sirkit::BigInteger<>;
    using Vector = std::vector<std::uint32_t>;
    const sirkit::MixedRadix empty(std::span<const std::uint32_t>{});
    assert(empty.capacity() == 1);
    assert(empty.encode({}) == 0);
    assert(empty.decode(0).empty());
    try
    {
        static_cast<void>(sirkit::MixedRadix<>(Vector{0}));
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
    const sirkit::MixedRadix fixed(Vector{3, 4, 2});
    assert(fixed.capacity() == 24);
    assert(fixed.encode(Vector{1, 2, 1}) == 13);
    assert(fixed.decode(13) == (Vector{1, 2, 1}));
    for (const auto &invalid : {Vector{}, Vector{3, 0, 1}})
    {
        try
        {
            static_cast<void>(fixed.encode(invalid));
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
    for (const auto &invalid : {Number(-1), Number(24)})
    {
        try
        {
            static_cast<void>(fixed.decode(invalid));
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }

    std::mt19937_64 random(380971);
    for (const std::size_t n : {1, 23, 24, 25, 48, 49, 101, 500, 1703})
    {
        Vector radices(n), digits(n);
        cpp_int oracle = 0, limit = 1;
        for (std::size_t i = 0; i < n; ++i)
        {
            radices[i] = static_cast<std::uint32_t>(1 + random() % std::numeric_limits<std::uint32_t>::max());
            digits[i] = static_cast<std::uint32_t>(random() % radices[i]);
            oracle *= radices[i];
            oracle += digits[i];
            limit *= radices[i];
        }
        const sirkit::MixedRadix format(radices);
        assert(format.capacity().to_string() == limit.str());
        const auto encoded = format.encode(digits);
        assert(encoded.to_string() == oracle.str());
        assert(format.decode(encoded) == digits);
        assert(format.decode(0) == Vector(n));
        for (std::size_t i = 0; i < n; ++i)
        {
            digits[i] = radices[i] - 1;
        }
        assert(format.encode(digits) == format.capacity() - 1);
        assert(format.decode(format.capacity() - 1) == digits);
    }

    Vector base_one(75, 1), all_zero(75, 0);
    base_one.front() = 17;
    all_zero.front() = 14;
    const sirkit::MixedRadix<16> base_hex(base_one);
    assert(base_hex.capacity().to_string() == "11");
    assert(base_hex.encode(all_zero).to_string() == "E");
    assert(base_hex.decode(sirkit::BigInteger<16>("E")) == all_zero);
}
