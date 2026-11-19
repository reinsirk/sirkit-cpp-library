// SPDX-License-Identifier: CC0-1.0
#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace sirkit_internal_hash_capacity_test {

using Table = sirkit::detail::InternalHashMap<int, int>;
struct GrowthMember {
    friend constexpr auto member_pointer(GrowthMember);
};
struct RehashMember {
    friend constexpr auto member_pointer(RehashMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<GrowthMember, &Table::doubled_capacity>;
template struct MemberAccess<RehashMember, &Table::rehash>;

template<class Operation>
void rejects_capacity(Operation operation) {
    try {
        operation();
    } catch (const std::length_error&) {
        return;
    }
    assert(false);
}

inline void run() {
    const auto grow = member_pointer(GrowthMember{});
    // サイズ上限で拒否する演算は、両方の引数だけで境界を再現できる。
    assert(grow(8, 16) == 16);
    rejects_capacity([&] { (void)grow(16, 16); });

    Table table;
    table[5] = 17;
    const auto rehash = member_pointer(RehashMember{});
    constexpr auto maximum = std::numeric_limits<std::size_t>::max();
    rejects_capacity([&] { (table.*rehash)(maximum / 2 + 1); });
    assert(table.size() == 1 && table[5] == 17 && !table.contains(9));
    table[9] = 23;
    assert(table.size() == 2 && table[9] == 23);
}

} // namespace sirkit_internal_hash_capacity_test
