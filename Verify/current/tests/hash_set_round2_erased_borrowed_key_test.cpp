// SPDX-License-Identifier: CC0-1.0
// eraseしたキーの参照元を解放後、衝突するキーを追加して独立した集合と照合する。
#ifndef HEADER
#define HEADER "../../../DataStructure/Hash_set.cpp"
#endif
#include HEADER
#include HEADER

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>

namespace {
std::size_t checks = 0;
void require(bool value) {
    ++checks;
    if (!value) std::abort();
}

struct ConstantHash {
    template <class T>
    std::uint64_t operator()(const T&) const noexcept { return 0; }
};

template <class Equal>
void string_view_lifetime() {
    HashSet<std::string_view, ConstantHash, Equal> set;
    std::unordered_set<std::string> reference;
    for (int round = 0; round < 64; ++round) {
        auto erased = std::make_unique<std::string>(128, 'a');
        auto next = std::make_unique<std::string>(128, 'b');
        require(set.insert_checked(std::string_view(*erased)).second);
        require(reference.insert(*erased).second);
        require(set.erase(std::string_view(*erased), false) == set.end());
        require(set.empty());
        require(reference.erase(*erased) == 1);
        erased.reset();
        require(set.insert_checked(std::string_view(*next)).second);
        require(reference.insert(*next).second);
        require(set.contains(std::string_view(*next)));
        require(set.size() == reference.size());
        require(set.erase(std::string_view(*next), false) == set.end());
        require(reference.erase(*next) == 1);
        next.reset();
        require(set.empty() && reference.empty());
    }
}

struct LookupEqual {
    int* const* records;
    bool operator()(int a, int b) const { return *records[a] == *records[b]; }
};

void integer_custom_equal_lifetime() {
    for (int round = 0; round < 128; ++round) {
        auto first = std::make_unique<int>(17);
        auto second = std::make_unique<int>(29);
        std::array<int*, 2> records{first.get(), second.get()};
        HashSet<int, ConstantHash, LookupEqual> set(0, {}, {records.data()});
        std::unordered_set<int> reference;
        require(set.insert_checked(0).second);
        require(reference.insert(17).second);
        require(set.erase(0, false) == set.end());
        require(set.empty());
        require(reference.erase(17) == 1);
        first.reset();
        require(set.insert_checked(1).second);
        require(reference.insert(29).second);
        require(set.contains(1));
        require(set.size() == reference.size());
    }
}

enum class BorrowedInteger : std::uintptr_t {};
bool operator==(BorrowedInteger a, BorrowedInteger b) {
    const auto* left = reinterpret_cast<const int*>(static_cast<std::uintptr_t>(a));
    const auto* right = reinterpret_cast<const int*>(static_cast<std::uintptr_t>(b));
    return *left == *right;
}

template <class Equal>
void enum_standard_equal_lifetime() {
    for (int round = 0; round < 128; ++round) {
        auto first = std::make_unique<int>(17);
        auto second = std::make_unique<int>(29);
        const auto a = static_cast<BorrowedInteger>(reinterpret_cast<std::uintptr_t>(first.get()));
        const auto b = static_cast<BorrowedInteger>(reinterpret_cast<std::uintptr_t>(second.get()));
        HashSet<BorrowedInteger, ConstantHash, Equal> set;
        std::unordered_set<int> reference;
        require(set.insert_checked(a).second);
        require(reference.insert(17).second);
        require(set.erase(a, false) == set.end());
        require(set.empty());
        require(reference.erase(17) == 1);
        first.reset();
        require(set.insert_checked(b).second);
        require(reference.insert(29).second);
        require(set.contains(b));
        require(set.size() == reference.size());
    }
}

static_assert(std::is_trivially_copy_constructible_v<std::string_view>);
static_assert(std::is_trivially_destructible_v<std::string_view>);
static_assert(std::is_enum_v<BorrowedInteger> && !std::is_integral_v<BorrowedInteger>);
} // namespace

int main(int argc, char** argv) {
    const std::string_view suite = argc == 1 ? "all" : argv[1];
    if (suite == "all" || suite == "string_view") {
        string_view_lifetime<std::equal_to<std::string_view>>();
        string_view_lifetime<std::equal_to<void>>();
    }
    if (suite == "all" || suite == "integer") integer_custom_equal_lifetime();
    if (suite == "all" || suite == "enum") {
        enum_standard_equal_lifetime<std::equal_to<BorrowedInteger>>();
        enum_standard_equal_lifetime<std::equal_to<void>>();
    }
    require(checks != 0);
    std::cout << "PASS " << checks << " erased borrowed-key checks\n";
}
