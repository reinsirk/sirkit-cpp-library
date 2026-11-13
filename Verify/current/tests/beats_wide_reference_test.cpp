// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Segment_Tree_Beats.cpp"
#include <bit>
#include <cassert>
#include <iostream>
#include <random>

int main() {
    using reinsirk::detail::BeatsWide;
    using reinsirk::detail::beats_multiply;
    using reinsirk::detail::beats_narrow;
    using Reference = __int128_t;
    std::mt19937_64 rng(0x128bea75);
    constexpr std::size_t base = std::size_t{1} << 42;
    const Reference squared_base = Reference(base) * base;
    auto from_reference = [&](Reference expected) {
        const auto top = static_cast<std::int64_t>(expected / squared_base);
        const auto middle = static_cast<std::int64_t>(expected % squared_base / base);
        const auto bottom = static_cast<std::int64_t>(expected % base);
        return beats_multiply(beats_multiply(BeatsWide(top), base) + BeatsWide(middle), base) + BeatsWide(bottom);
    };
    for (unsigned i = 0; i < 250000; ++i) {
        const auto a = std::bit_cast<std::int64_t>(rng()), b = std::bit_cast<std::int64_t>(rng());
        const auto n = static_cast<std::size_t>(rng() >> 2), m = static_cast<std::size_t>(rng() >> 2);
        const auto pa = beats_multiply(BeatsWide(a), n), pb = beats_multiply(BeatsWide(b), m);
        const Reference ra = Reference(a) * n, rb = Reference(b) * m;
        assert((pa < pb) == (ra < rb) && (pa == pb) == (ra == rb));
        assert(pa + pb == from_reference(ra + rb));
        assert(pa - pb == from_reference(ra - rb));
        assert(-pa == from_reference(-ra));
        assert(beats_narrow((pa + pb) - pa - pb) == 0);
        const auto expected = ra + rb;
        const bool fits = expected >= std::numeric_limits<std::int64_t>::min() && expected <= std::numeric_limits<std::int64_t>::max();
        try { const auto got = beats_narrow(pa + pb); assert(fits && got == expected); }
        catch (const std::overflow_error&) { assert(!fits); }
    }
    for (auto x : {std::numeric_limits<std::int64_t>::min(), std::int64_t{-1}, std::int64_t{0}, std::int64_t{1}, std::numeric_limits<std::int64_t>::max()})
        assert(beats_narrow(BeatsWide(x)) == x);
    std::cout << "beats wide arithmetic 250000 independent __int128 comparisons OK\n";
}
