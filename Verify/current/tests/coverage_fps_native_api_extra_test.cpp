// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_TEST_FPS_SOURCE
#define SIRKIT_TEST_FPS_SOURCE "../../../Math/Formal_Power_Series.cpp"
#endif
#include SIRKIT_TEST_FPS_SOURCE

namespace {
template<class Error>
bool rejects(auto operation) {
    try {
        operation();
    } catch (const Error&) {
        return true;
    }
    return false;
}
}

int main() {
    using Series = sirkit::FormalPowerSeries;
    using Residues = std::vector<std::uint32_t>;
    // 既定精度は末尾のゼロを含めた係数数。定義式から計算した先頭4項と照合する。
    assert((Series{998244352} + Series{2, 3, 4} == Series{1, 3, 4}));
    assert((Series{1} - Series{2, 3, 4} == Series{-1, -3, -4}));
    assert((Series{1, 1, 0, 0}.log() == Series{0, 1, -499122177, 332748118}));
    assert((Series{0, 1, 0, 0}.exp() == Series{1, 1, 499122177, 166374059}));
    assert((Series{1, -1, 0, 0}.pow(3) == Series{1, -3, 3, -1}));
    assert((Series{1, 2, 1, 0}.sqrt() == Series{1, 1, 0, 0}));

    sirkit::NTT<> ntt;
    auto volatile read_modulus = &sirkit::NTT<>::modulus;
    assert(read_modulus() == 998244353u);
    assert(rejects<std::length_error>([] {
        sirkit::detail::fps_prepare_inverses(998244353);
    }));
    assert(rejects<std::length_error>([] {
        (void)sirkit::detail::convolution_result_size(
            std::numeric_limits<std::size_t>::max(), 2, "NTT: convolution size overflow");
    }));
    ntt.prepare(0);
    assert(ntt.roots_capacity() == 0);
    assert(rejects<std::length_error>([] { sirkit::NTT<17, 3>{}.prepare(32); }));
    // 定数多項式の周波数成分は全部同じ。長さ16の変換と逆変換を照合する。
    Residues impulse(16);
    impulse.front() = 7;
    auto frequencies = impulse;
    ntt.transform(frequencies);
    assert(frequencies == Residues(16, 7));
    ntt.transform(frequencies, true);
    assert(frequencies == impulse);

    // 片側だけが大きい畳み込みで、ゼロ補完した短い側をまとめて処理する。
    const Residues smaller(33, 1);
    const Residues larger(2048, 1);
    const auto product = ntt.convolution_normalized(smaller, larger);
    assert(product.size() == smaller.size() + larger.size() - 1);
    for (std::size_t i = 0; i < product.size(); ++i) {
        assert(product[i] == std::min({i + 1, smaller.size(), product.size() - i}));
    }

#ifndef SKIP_RUNTIME_CHECKS
    constexpr auto kMod = sirkit::NTT<>::modulus();
    impulse.back() = kMod;
    assert(rejects<std::invalid_argument>([&] { ntt.transform(impulse); }));
    // 直接畳み込みの短い側と、長い側の8係数に満たない末尾を別々に検査する。
    Residues long_input(65, 1);
    assert(rejects<std::invalid_argument>([&] {
        (void)ntt.convolution_normalized({1, kMod}, long_input);
    }));
    long_input.back() = kMod;
    assert(rejects<std::invalid_argument>([&] {
        (void)ntt.convolution_normalized({1, 1}, long_input);
    }));
#endif

#ifdef SIRKIT_TEST_POLYNOMIAL_PRODUCT_TREE
    sirkit::ProductTree first(std::vector<int>{1, 2});
    sirkit::ProductTree second(std::vector<int>{3});
    swap(first, second);
    assert((first.polynomial() == Series{-3, 1}));
    assert((second.polynomial() == Series{2, -3, 1}));
    assert(first.evaluate(Series{0, 1}) == Residues({3}));
    assert(second.evaluate(Series{0, 1}) == Residues({1, 2}));

    // 点数と精度の上限は入力検査を省略しても維持する。
    const auto oversized = Series::max_precision() + 1;
    const Residues coefficients(oversized);
    assert(rejects<std::length_error>([&] { (void)sirkit::ProductTree(coefficients); }));
    assert(rejects<std::length_error>([&] { (void)first.evaluate(coefficients); }));
    assert(rejects<std::length_error>([&] { (void)sirkit::TaylorShift(coefficients, 1); }));
    const Series series(oversized);
    assert(rejects<std::length_error>([&] { (void)first.evaluate(series); }));
    assert(rejects<std::length_error>([&] { (void)sirkit::TaylorShift(series, 1); }));
#endif
}
