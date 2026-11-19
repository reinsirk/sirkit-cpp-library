// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef HEADER
#define HEADER "../../../DataStructure/Hash_map.cpp"
#endif
#include HEADER
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef SKIP_RUNTIME_CHECKS
namespace {
using GenerationMap = sirkit::HashMap<int, int>;
struct GenerationField { friend constexpr auto generation_field(GenerationField); };
template<auto Member> struct GenerationAccess {
    friend constexpr auto generation_field(GenerationField) { return Member; }
};
template struct GenerationAccess<&GenerationMap::generation_>;
}
#endif

// 乗算後の上位ビットが全て1になり、衝突列が末尾から先頭へ回り込む。
struct WrapHash {
    static constexpr std::uint64_t inverse() {
        constexpr std::uint64_t multiplier = UINT64_C(11995408973635179863);
        std::uint64_t result = 1;
        for (int i = 0; i < 6; ++i) result *= 2 - multiplier * result;
        return result;
    }
    std::uint64_t operator()(int) const noexcept { return UINT64_MAX * inverse(); }
};
struct ThrowingWrapHash {
    static inline bool armed = false;
    std::uint64_t operator()(int key) const {
        if (armed && key == 17) throw std::runtime_error("hash");
        return WrapHash{}(key);
    }
};
struct Alive {
    static inline int count = 0;
    static inline bool fail = false;
    int value;
    explicit Alive(int x) : value(x) {
        if (fail) throw std::runtime_error("construct");
        ++count;
    }
    Alive(const Alive& other) : value(other.value) { ++count; }
    Alive(Alive&& other) noexcept : value(other.value) { ++count; }
    ~Alive() { --count; }
};
struct ResidueHash {
    std::uint64_t operator()(int x) const noexcept { return GenericHash(7)(x % 10); }
};
struct ResidueEqual {
    bool operator()(int x, int y) const noexcept { return x % 10 == y % 10; }
};

template<class Exception, class Action>
void throws(Action action) {
    bool caught = false;
    try { action(); } catch (const Exception&) { caught = true; }
    assert(caught);
}

int main() {
    static_assert(std::bidirectional_iterator<HashMap<int, int>::iterator>);
    static_assert(std::bidirectional_iterator<HashMap<int, int>::const_iterator>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(HashMap<int, int>{}.begin()->first)>>);
    // 墓石の再利用と同一キーの再登録を、回り込む衝突列で繰り返す。
    HashMap<int, int, WrapHash> table(0, WrapHash{});
    std::map<int, int> reference;
    for (int cycle = 0; cycle < 60; ++cycle) {
        for (int key = 0; key < 80; ++key) {
            table[key] = cycle + key;
            reference[key] = cycle + key;
        }
        for (int key = cycle % 3; key < 80; key += 3) {
            table.erase(key, false);
            reference.erase(key);
        }
        for (int key = 79; key >= 0; --key) {
            const auto got = table.try_emplace(key, -key);
            const auto want = reference.try_emplace(key, -key);
            assert(got.second == want.second && got.first->second == want.first->second);
        }
        assert(table.size() == reference.size());
        const auto& view = table;
        for (const auto& [key, value] : reference) {
            assert(view.at(key) == value && view.find(key)->second == value);
            assert(view.contain(key) && view.count(key) == 1);
        }
        std::vector<int> forward, backward;
        for (const auto& [key, value] : table) { (void)value; forward.push_back(key); }
        auto reverse = table.cend();
        while (reverse != table.cbegin()) { --reverse; backward.push_back(reverse->first); }
        assert(std::equal(forward.begin(), forward.end(), backward.rbegin(), backward.rend()));
        for (auto it = table.begin(); it != table.end();) it = table.erase(it);
        reference.clear();
        assert(table.empty() && table.begin() == table.end());
    }
    throws<std::out_of_range>([&] { (void)table.at(-1); });
    const auto& empty_view = table;
    assert(empty_view.count(-1) == 0);
    throws<std::out_of_range>([&] { (void)empty_view.at(-1); });
    throws<std::length_error>([&] { table.reserve(std::numeric_limits<std::size_t>::max()); });
    throws<std::length_error>([&] { table.rehash(std::numeric_limits<std::size_t>::max()); });
    assert(table.empty());
    HashMap<int, int> never_inserted;
    never_inserted.clear();
    assert(never_inserted.empty() && never_inserted.capacity() == 0);

    // 自明な値でも、再配置中のハッシュ例外は元の要素・容量を保つ。
    HashMap<int, int, ThrowingWrapHash> risky;
    for (int key = 0; key < 50; ++key) risky[key] = key * 3;
    const auto old_capacity = risky.capacity();
    ThrowingWrapHash::armed = true;
    throws<std::runtime_error>([&] { risky.reserve(1000); });
    throws<std::runtime_error>([&] { risky.rehash(1); });
    ThrowingWrapHash::armed = false;
    assert(risky.size() == 50 && risky.capacity() == old_capacity);
    for (int key = 0; key < 50; ++key) assert(risky.at(key) == key * 3);
    risky.rehash(1);
    for (int key = 0; key < 50; ++key) assert(risky.at(key) == key * 3);

    // 同値判定を変更した場合も、登録済みのキーは一つに保つ。
    HashMap<int, int, ResidueHash, ResidueEqual> residues;
    residues[1] = 7;
    assert(!residues.try_emplace(11, 8).second && residues.size() == 1);
    residues.insert_or_assign(21, 9);
    assert(residues.at(31) == 9);
    residues.erase(41, false);
    assert(residues.empty());

    // 構築失敗・墓石・再配置・clearを通じて所有オブジェクト数を確認する。
    {
        HashMap<int, Alive, WrapHash> objects;
        objects.try_emplace(1, 3);
        Alive::fail = true;
        assert(!objects.try_emplace(1, 4).second);
        throws<std::runtime_error>([&] { objects.try_emplace(2, 5); });
        assert(objects.size() == 1 && Alive::count == 1 && !objects.contains(2));
        Alive::fail = false;
        objects.try_emplace(2, 6);
        objects.erase(1, false);
        assert(Alive::count == 1);
        Alive::fail = true;
        throws<std::runtime_error>([&] { objects.try_emplace(3, 7); });
        assert(objects.size() == 1 && objects.at(2).value == 6 && Alive::count == 1);
        Alive::fail = false;
        objects.try_emplace(3, 7);
        objects.reserve(100);
        assert(Alive::count == 2 && objects.at(3).value == 7);
        objects.clear();
        assert(Alive::count == 0);
    }
    assert(Alive::count == 0);
    HashMap<int, std::unique_ptr<int>> owned;
    for (int i = 0; i < 100; ++i) owned.try_emplace(i, std::make_unique<int>(i));
    owned.reserve(1000);
    owned.insert_or_assign(7, std::make_unique<int>(99));
    auto moved = std::move(owned);
    assert(owned.empty() && moved.size() == 100 && *moved.at(7) == 99);
    owned.try_emplace(1, std::make_unique<int>(3));
    moved.swap(owned);
    assert(moved.size() == 1 && *moved.at(1) == 3 && owned.size() == 100);

    // GenericHashの等値契約を、符号付きゼロ・複合キー・順序の違う集合で確認する。
    const GenericHash hash(5);
    assert(hash(0.0) == hash(-0.0));
    HashMap<double, int> floats;
    floats[0.0] = 8;
    assert(floats.at(-0.0) == 8 && !floats.try_emplace(-0.0, 9).second);
    using Nested = std::tuple<std::string, std::vector<int>, std::array<int, 2>>;
    HashMap<Nested, int> nested;
    const Nested key{"x", {1, 2}, {3, 4}};
    nested[key] = 9;
    assert(nested.at(Nested{"x", {1, 2}, {3, 4}}) == 9);
    const std::unordered_set<int> set_a{1, 3, 5, 7}, set_b{7, 5, 3, 1};
    assert(set_a == set_b && hash(set_a) == hash(set_b));
    HashMap<std::unordered_set<int>, int> sets;
    sets[set_a] = 11;
    assert(sets.at(set_b) == 11);
    const std::unordered_map<int, std::vector<int>> map_a{{1, {2, 3}}, {4, {5}}};
    const std::unordered_map<int, std::vector<int>> map_b{{4, {5}}, {1, {2, 3}}};
    assert(map_a == map_b && hash(map_a) == hash(map_b));
#ifndef SKIP_RUNTIME_CHECKS
    // 世代番号が一周した後にも、縮小前の添字は現在の表を越える。
    GenerationMap wrapped_generation;
    wrapped_generation.rehash(1024);
    auto& generation = wrapped_generation.*generation_field(GenerationField{});
    generation = 0;
    const auto stale_end = wrapped_generation.end();
    wrapped_generation.rehash(8);
    generation = std::numeric_limits<std::size_t>::max();
    wrapped_generation.clear();
    assert(generation == 0);
    throws<std::out_of_range>([&] { (void)*stale_end; });
    throws<std::out_of_range>([&] { (void)wrapped_generation.erase(stale_end); });
    HashMap<int, int> checked;
    checked[1] = 4;
    const HashMap<int, int>::iterator unbound;
    throws<std::out_of_range>([&] { (void)*unbound; });
    throws<std::out_of_range>([&] { (void)*checked.end(); });
    throws<std::out_of_range>([&] { --checked.begin(); });
    HashMap<int, int> foreign;
    assert(checked.end() != foreign.end());
    throws<std::out_of_range>([&] { checked.erase(foreign.end()); });
    const auto erased = checked.begin();
    checked.erase(erased, false);
    throws<std::out_of_range>([&] { (void)*erased; });
    throws<std::out_of_range>([&] { checked.erase(erased); });
    checked[2] = 5;
    auto stale = checked.begin();
    checked.clear();
    throws<std::out_of_range>([&] { (void)*stale; });
    throws<std::out_of_range>([&] { checked.erase(stale); });
#endif
    ::sirkit::detail::CheckHashCapacity(8, 8, "test");
    throws<std::length_error>([] { ::sirkit::detail::CheckHashCapacity(16, 8, "test"); });
    std::cout << "PASS wraparound/tombstones, reverse iterators, generic keys, ownership/construction failure, capacity bounds\n";
}
