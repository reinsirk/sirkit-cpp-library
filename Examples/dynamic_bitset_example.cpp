// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Dynamic_Bitset.cpp"
#include <iostream>
#include <utility>
int main() {
    DynamicBitset bits("0010101");
    bits[1] = true;
    bits[2] = bits[0];
    bits[1].flip();
    bits[3] |= true;
    std::cout << bits.to_string() << ' ' << bits.count() << ' ' << bits.count(1, 5) << '\n';
    for (auto pos = bits.find_first(); pos != bits.size(); pos = bits.find_next(pos)) std::cout << pos << ' ';
    std::cout << '\n';
    std::cout << bits.find_last() << ' ' << bits.find_prev(bits.find_last()) << '\n';
    auto other = ~bits;
    std::cout << bits.intersection_count(other) << ' ' << bits.intersection_parity(other) << ' ' << (bits | other).all() << '\n';
    std::cout << (bits << 2).to_string() << ' ' << (bits >> 2).to_string() << '\n';
    DynamicBitset reachable(11);
    reachable.set(0);
    reachable.shift_or(2).shift_or(3).shift_or(5);
    std::cout << reachable.count() << '\n';
    bits.resize(65, true);
    std::cout << bits.size() << ' ' << bits.words().size() << ' ' << bits.test(64) << '\n';
    bits.resize(4);
    bits.resize(8);
    std::cout << bits.to_string() << '\n';
    auto copy = bits;
    copy.flip(0);
    std::cout << (copy == bits) << ' ' << bits.test(0) << '\n';
    auto moved = std::move(copy);
    std::cout << copy.empty() << ' ' << moved.size() << '\n';
    copy.reserve_bits(1000);
    copy.resize(3, true);
    copy.reset(1);
    std::cout << copy.all() << ' ' << copy.any() << ' ' << copy.none() << '\n';
    copy.reset().set(1).flip();
    std::cout << copy.to_string() << '\n';
    copy.clear();
}
