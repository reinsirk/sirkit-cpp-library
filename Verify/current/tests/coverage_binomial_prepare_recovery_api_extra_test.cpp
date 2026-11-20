// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_BINOMIAL_SOURCE
#define SIRKIT_TEST_BINOMIAL_SOURCE "../../../Math/Combination/ModInt_Binomial.cpp"
#endif
#include SIRKIT_TEST_BINOMIAL_SOURCE

#include <atcoder/modint.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

namespace {
int remaining_allocations = -1;
}

void* operator new(std::size_t size) {
    if (remaining_allocations == 0) throw std::bad_alloc{};
    if (remaining_allocations > 0) --remaining_allocations;
    if (void* memory = std::malloc(size == 0 ? 1 : size)) return memory;
    throw std::bad_alloc{};
}

void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

namespace {
template<class Factory, class Unwrap>
void check_prepare(Factory make, Unwrap unwrap, int modulus) {
    for (int budget : {0, 1}) {
        auto choose = make();
        // 階乗表・逆階乗表、または素数冪の表の確保が片方だけ成功する場合も試す。
        bool failed = false;
        remaining_allocations = budget;
        try {
            choose.prepare(80);
        } catch (const std::bad_alloc&) {
            failed = true;
        }
        remaining_allocations = -1;
        assert(failed);
        assert(unwrap(choose.COM(10, 3)) == 120 % modulus);
        choose.prepare(80);
        constexpr int kExpected = 80 * 79 * 78 * 77 / 24;
        assert(unwrap(choose.COM(80, 4)) == kExpected % modulus);
        assert(unwrap(choose.H(76, 5)) == (kExpected * 76 / 5) % modulus);
    }
}
}

int main() {
    for (int modulus : {81, 101}) {
        check_prepare([=] { return sirkit::BinomialMod(modulus, 10, {80, 100}); },
                      [](std::uint32_t value) { return static_cast<int>(value); }, modulus);
    }
    using Mint = atcoder::static_modint<101>;
    check_prepare([] { return sirkit::ModIntBinomial<Mint>(10, {80, 100}); },
                  [](Mint value) { return value.val(); }, 101);

    sirkit::BinomialMod trivial(1, 0);
    assert(trivial.H(8, -1) == 0);
    assert(trivial.H(8, 4) == 0);
}
