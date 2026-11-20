// SPDX-License-Identifier: CC0-1.0
#include "../../../String/RollingHash.cpp"

#include <cassert>
#include <cstdint>
#include <string_view>

int main() {
    constexpr std::uint64_t base = 31;
    sirkit::RollingHash source("source", base), target("target", 257);
    source.clear();
    target = source;
    assert(target.empty() && target.base() == base && target.whole() == 0);
    assert(target.get(0, 0) == 0 && target.concat(456, 12, 0) == 468);
    assert(target.equal(0, 0, source, 0, 0) && target.lcp(0, source, 0) == 0);
    assert(target.compare(0, 0, source, 0, 0) == 0);

    const char bytes[]{'a', '\0', 'b', 'c'};
    const std::string_view input(bytes, sizeof(bytes));
    target.assign(input);
    std::uint64_t expected = 0;
    for (unsigned char byte : input) expected = expected * base + byte;
    assert(target.size() == input.size() && target.whole() == expected);
    assert(target.get(1, 3) == static_cast<unsigned char>('b'));
    assert(source.empty() && source.whole() == 0);
}
