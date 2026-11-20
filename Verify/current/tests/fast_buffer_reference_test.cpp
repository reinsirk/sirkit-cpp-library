// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include "../../../Fast/Source/Internal/Buffer.cpp"
#include "../../../Fast/Source/Internal/Buffer.cpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

namespace {
std::uint64_t checks = 0;
void check(bool value) { ++checks; assert(value); }
template <class T> T make_value(std::uint64_t value) {
    if constexpr (std::is_same_v<T, std::string>) return std::string(40, static_cast<char>('a' + value % 26)) + std::to_string(value);
    else return static_cast<T>(value);
}
template <class T> void test_type(std::mt19937_64& random) {
    ::sirkit::detail::FastBuffer<T> actual;
    std::vector<T> expected;
    for (int step = 0; step < 12000; ++step) {
        T value = make_value<T>(random() % 10000);
        switch (random() % 8) {
        case 0: actual.push_back(value); expected.push_back(value); break;
        case 1: actual.emplace_back(value); expected.emplace_back(value); break;
        case 2: if (!expected.empty()) { actual.pop_back(); expected.pop_back(); } break;
        case 3: {
            const auto count = static_cast<std::size_t>(random() % 128);
            actual.resize(count, value); expected.resize(count, value); break;
        }
        case 4: actual.reserve(actual.size() + random() % 256); break;
        case 5: actual.clear(); expected.clear(); break;
        case 6:
            if (!expected.empty()) {
                actual.emplace_back(actual.front()); expected.emplace_back(expected.front());
            }
            break;
        case 7: {
            auto copy = actual;
            ::sirkit::detail::FastBuffer<T> assigned; assigned = copy;
            auto moved = std::move(copy); check(copy.empty());
            assigned = std::move(moved); check(moved.empty());
            check(assigned.size() == expected.size());
            for (std::size_t i = 0; i < expected.size(); ++i) check(assigned[i] == expected[i]);
            actual.swap(assigned); break;
        }
        }
        check(actual.size() == expected.size()); check(actual.capacity() >= actual.size());
        for (std::size_t i = 0; i < expected.size(); ++i) check(actual[i] == expected[i]);
    }
    for (std::size_t count : {1,2,4,8,16,32,64,128}) {
        ::sirkit::detail::FastBuffer<T> alias(count, make_value<T>(42));
        alias.emplace_back(alias.front()); check(alias.back() == make_value<T>(42));
        alias.resize(alias.capacity() + 17, alias.front());
        for (const auto& value : alias) check(value == make_value<T>(42));
        alias.assign(alias.begin(), alias.end());
        check(alias.front() == make_value<T>(42));
    }
}
struct alignas(128) Wide {
    int value;
    explicit Wide(int v) : value(v) {}
};
struct DestructorProbe {
    static inline int destroyed = 0;
    int value;
    explicit DestructorProbe(int v) : value(v) {}
    ~DestructorProbe() { ++destroyed; }
};
}
int main() {
    std::mt19937_64 random(0x713eb213a2);
    test_type<std::int64_t>(random); test_type<bool>(random); test_type<std::string>(random);
    ::sirkit::detail::FastBuffer<int> empty(static_cast<const int*>(nullptr), static_cast<const int*>(nullptr));
    check(empty.empty());
    ::sirkit::detail::FastBuffer<Wide> wide;
    for (int i = 0; i < 1000; ++i) wide.emplace_back(i);
    check(reinterpret_cast<std::uintptr_t>(wide.data()) % alignof(Wide) == 0);
    for (int i = 0; i < 1000; ++i) check(wide[i].value == i);
    const auto before = DestructorProbe::destroyed;
    {
        ::sirkit::detail::FastBuffer<DestructorProbe> values;
        values.reserve(8); values.emplace_back(1); values.emplace_back(2);
        values.clear(); check(DestructorProbe::destroyed == before);
    }
    check(DestructorProbe::destroyed == before);
    std::cout << "fast buffer checks=" << checks << '\n';
}
