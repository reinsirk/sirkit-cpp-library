// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Binary_Trie.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#ifndef SKIP_RUNTIME_CHECKS
namespace {
template<class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const std::out_of_range&) {
        return;
    }
    assert(false);
}
} // namespace
#endif

namespace {

using CountTrie = sirkit::BinaryTrie<std::uint8_t, 0, 3>;
struct RootMember {
    friend constexpr auto member_pointer(RootMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<RootMember, &CountTrie::root_>;

void check_count_overflow() {
    CountTrie trie;
    trie.add(0);
    constexpr auto maximum = std::numeric_limits<CountTrie::size_type>::max();
    // 同じキーを最大回数追加したときと同じ根・葉の状態を、巨大な反復なしで作る。
    auto* root = trie.*member_pointer(RootMember{});
    root->count = root->data.child[0]->count = maximum;
    assert(trie.size() == maximum && trie.count(0) == maximum);
    assert(!trie.insert_unique(0));
    const auto rejects = [](auto operation) {
        try {
            operation();
        } catch (const std::overflow_error&) {
            return;
        }
        assert(false);
    };
    rejects([&] { (void)trie.insert_unique(1); });
    rejects([&] { trie.add(0); });
    assert(trie.size() == maximum && trie.count(0) == maximum && trie.count(1) == 0);
    assert(trie.del(0) && trie.size() == maximum - 1);
    trie.add(0);
    assert(trie.size() == maximum && trie.count(0) == maximum);
}

} // namespace

int main() {
    check_count_overflow();
    sirkit::BinaryTrie<std::int16_t, 7, 512> trie;
#ifndef SKIP_RUNTIME_CHECKS
    rejects([&] { (void)trie.min_xor(0); });
    rejects([&] { trie.add(-1); });
    rejects([&] { trie.operate_xor(256); });
    assert(trie.empty());
#endif
    trie.add(7);
    assert(!trie.del(7, 42));
    trie.add(7, 12);
    assert(!trie.del(7, 42));
    assert(trie.del(7));
    assert(trie.find(7).first == 1 && trie.find(7).second == std::vector{12});
    assert(trie.del(7));
    assert(trie.find(7).first == 0 && trie.find(7).second.empty());

    const std::vector<std::int16_t> original{0, 7, 7, 127, 128, 255};
    for (const auto value : original) trie.add(value);
    constexpr std::int16_t mask = 129;
    trie.operate_xor(mask);
    auto expected = original;
    for (auto& value : expected) value ^= mask;
    std::sort(expected.begin(), expected.end());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        assert(trie.get_kth(static_cast<std::int64_t>(i + 1)).first == expected[i]);
    }
    assert(trie.size() == original.size());
    assert(trie.count_less(256) == expected.size());
#ifndef SKIP_RUNTIME_CHECKS
    rejects([&] { (void)trie.get_kth(0); });
    rejects([&] { (void)trie.get_kth(static_cast<std::int64_t>(expected.size() + 1)); });
    assert(trie.size() == expected.size());
#endif
}
