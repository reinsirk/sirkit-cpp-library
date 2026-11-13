// SPDX-License-Identifier: CC0-1.0
#ifndef HASH_MAP_HEADER
#define HASH_MAP_HEADER "../../../DataStructure/Hash_map.cpp"
#endif
#include HASH_MAP_HEADER
#include HASH_MAP_HEADER
#include <cassert>
#include <memory>
#include <string>
#include <string_view>

struct ConstantHash {
    template<class T> std::uint64_t operator()(const T&) const noexcept { return 0; }
};
struct BorrowedKey {
    const std::string* text = nullptr;
    friend bool operator==(BorrowedKey a, BorrowedKey b) { return *a.text == *b.text; }
};

void views(bool insert, bool through_iterator, bool keep_other) {
    const std::string keeper(128, 'c'), probe(128, 'b');
    HashMap<std::string_view, int, ConstantHash> map;
    auto erased = std::make_unique<std::string>(128, 'a');
    map.try_emplace(std::string_view(*erased), 1);
    if (keep_other) map.try_emplace(std::string_view(keeper), 3);
    if (through_iterator) map.erase(map.find(std::string_view(*erased)), false);
    else map.erase(std::string_view(*erased), false);
    erased.reset();
    assert(!map.contains(probe));
    assert(map.find(probe) == map.end());
    if (insert) {
        const auto result = map.try_emplace(std::string_view(probe), 2);
        assert(result.second && result.first->second == 2);
        assert(map.at(probe) == 2);
    }
    assert(map.size() == static_cast<std::size_t>(keep_other) + static_cast<std::size_t>(insert));
    if (keep_other) assert(map.at(keeper) == 3);
    auto copied = map;
    copied.reserve(128);
    if (insert) assert(copied.at(probe) == 2);
    if (keep_other) assert(copied.at(keeper) == 3);
    map.clear();
    map.try_emplace(probe, 7);
    assert(map.at(probe) == 7);
}

int main() {
    for (bool insert : {false, true})
        for (bool iterator : {false, true})
            for (bool keep : {false, true}) views(insert, iterator, keep);
    const std::string keeper(128, 'c'), probe(128, 'b');
    HashMap<BorrowedKey, int, ConstantHash> map;
    auto owner = std::make_unique<std::string>(128, 'a');
    map.try_emplace(BorrowedKey{owner.get()}, 1);
    map.try_emplace(BorrowedKey{&keeper}, 3);
    map.erase(BorrowedKey{owner.get()}, false);
    owner.reset();
    assert(!map.contains(BorrowedKey{&probe}));
    assert(map.try_emplace(BorrowedKey{&probe}, 2).second);
    assert(map.at(BorrowedKey{&keeper}) == 3);
}
