// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include "../../../Math/Xor_Basis.cpp"
#include <bit>
#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include <vector>

void verify(const std::vector<std::uint64_t>& input, const XorBasis& basis, std::mt19937_64& rng) {
    std::set<std::uint64_t> all{0}, nonempty;
    for (std::size_t mask = 1; mask < (std::size_t{1} << input.size()); ++mask) {
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < input.size(); ++i) if (((mask >> i) & 1) != 0) value ^= input[i];
        all.insert(value); nonempty.insert(value);
    }
    assert(all.size() == (std::size_t{1} << basis.rank()));
    assert(basis.has_nonempty_zero() == nonempty.contains(0));
    std::uint64_t k = 0;
    for (auto x : all) {
        assert(basis.kth(k) == x); ++k;
        assert(basis.contains(x) && basis.contains_nonempty(x) == nonempty.contains(x));
    }
    assert(!basis.kth(k));
    for (unsigned i = 0; i < 30; ++i) {
        const auto x = i < 10 ? i : rng();
        assert(basis.contains(x) == all.contains(x));
        assert(basis.contains_nonempty(x) == nonempty.contains(x));
        auto minimum = x, maximum = x;
        for (auto v : all) { minimum = std::min(minimum, x ^ v); maximum = std::max(maximum, x ^ v); }
        assert(basis.min_xor(x) == minimum && basis.max_xor(x) == maximum);
    }
}
int main() {
    std::mt19937_64 rng(0x8ab4515);
    XorBasis empty; verify({}, empty, rng);
    for (unsigned run = 0; run < 1300; ++run) {
        std::vector<std::uint64_t> input, left_input, right_input;
        XorBasis basis, left, right;
        const auto n = rng() % 11;
        for (std::size_t i = 0; i < n; ++i) {
            const auto x = run % 2 == 0 ? rng() % 256 : rng();
            const auto previous = basis.rank();
            const bool added = basis.insert(x);
            assert(added == (basis.rank() == previous + 1));
            input.push_back(x);
            if (i % 2 == 0) { left.insert(x); left_input.push_back(x); }
            else { right.insert(x); right_input.push_back(x); }
        }
        verify(input, basis, rng);
        left.merge(right); verify(input, left, rng);
        auto copied = basis; copied.clear(); verify({}, copied, rng); verify(input, basis, rng);
        copied = basis; auto moved = std::move(copied); verify(input, moved, rng); verify(input, copied, rng);
        copied.swap(empty); verify({}, copied, rng); empty.clear();
        if (n != 0) { basis.merge(basis); assert(basis.has_nonempty_zero()); }
    }
    XorBasis full;
    for (unsigned bit = 0; bit < 64; ++bit) assert(full.insert(std::uint64_t{1} << bit));
    assert(full.rank() == 64 && !full.has_nonempty_zero() && !full.contains_nonempty(0));
    for (unsigned i = 0; i < 50000; ++i) {
        const auto k = rng(); assert(full.kth(k) == k && full.contains(k));
        assert(full.min_xor(k) == 0 && full.max_xor(k) == std::numeric_limits<std::uint64_t>::max());
    }
    assert(!full.insert(0) && full.contains_nonempty(0));
    XorBasis zero; assert(!zero.insert(0)); assert(zero.rank() == 0 && zero.has_nonempty_zero());
    full.clear(); full.merge(zero); assert(full.has_nonempty_zero() && full.rank() == 0);
    std::cout << "xor_basis independent subset reference OK\n";
}
