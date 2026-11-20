// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Convolution.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

int main() {
    const auto max_size = std::numeric_limits<std::size_t>::max();
    using sirkit::detail::convolution_result_size;
    assert(convolution_result_size(max_size, 1, "size") == max_size);
    assert(convolution_result_size(max_size, 0, "size") == 0);
    bool size_overflow = false;
    try {
        (void)convolution_result_size(max_size, 2, "size");
    } catch (const std::length_error&) {
        size_overflow = true;
    }
    assert(size_overflow);
    sirkit::NTT<17> small_ntt;
    bool length_overflow = false;
    try {
        small_ntt.prepare(32);
    } catch (const std::length_error&) {
        length_overflow = true;
    }
    assert(length_overflow && small_ntt.roots_capacity() == 0);
#ifndef SKIP_RUNTIME_CHECKS
    // 短い方の係数を読むAVX2経路と標準経路で、同じ入力を拒否する。
    sirkit::NTT<> ntt;
    bool rejected_coefficient = false;
    try {
        (void)ntt.convolution_normalized({ntt.modulus()}, std::vector<std::uint32_t>(256, 1));
    } catch (const std::invalid_argument&) {
        rejected_coefficient = true;
    }
    assert(rejected_coefficient && ntt.roots_capacity() == 0);
#endif

    sirkit::CRTConvolution convolution;
    convolution.prepare(0);
    assert(convolution.roots_capacity() == 0);
    convolution.prepare(128);
    const auto prepared = convolution.roots_capacity();
    const std::vector<int> pair{1, 2};
    const std::vector<std::uint8_t> large(convolution.max_length(), 1);
    bool rejected = false;
    try {
        (void)convolution.modular(pair, large, 97);
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected && convolution.roots_capacity() == prepared);
    const std::vector<int> empty;
    assert(convolution.modular(pair, empty, 97).empty());
    const std::vector<std::uint32_t> expected{1, 4, 4};
    assert(convolution.modular_square(pair, 97) == expected);
}
