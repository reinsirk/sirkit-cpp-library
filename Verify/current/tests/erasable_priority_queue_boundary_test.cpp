// SPDX-License-Identifier: CC0-1.0
// ヒープの小サイズ・4冪前後で、整数各幅・重複・遅延削除・複製を独立照合する。
#ifndef EPQ_SOURCE
#define EPQ_SOURCE "../../../DataStructure/erasable-priority-queue.cpp"
#endif
#include EPQ_SOURCE
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <random>
#include <set>
#include <type_traits>
#include <vector>
static std::size_t checks=0;
static void require(bool result){++checks;if(!result)std::abort();}
template <class T, bool Minimize>
static void small_boundaries() {
    using Q = ErasablePriorityQueue<T, Minimize>;
    std::mt19937 rng(20261028);
    for (int n = 0; n <= 17; ++n) {
        for (int erased = 0; erased <= n; ++erased) {
            for (int pattern = 0; pattern < 4; ++pattern) {
                std::vector<T> values;
                for (int i = 0; i < n; ++i)
                    values.push_back(static_cast<T>(pattern == 3 ? (i % 4) : (i * 5 + 3)));
                std::multiset<T> expected(values.begin(), values.end());
                Q queue(values);
                if (pattern == 1) std::reverse(values.begin(), values.end());
                if (pattern == 2) std::shuffle(values.begin(), values.end(), rng);
                for (int i = 0; i < erased; ++i) {
                    queue.erase(values[i]);
                    expected.erase(expected.find(values[i]));
                }
                require(queue.size() == expected.size());
                Q copy(queue);
                for (int pass = 0; pass < 2; ++pass) {
                    auto& current = pass == 0 ? queue : copy;
                    auto reference = expected;
                    while (!reference.empty()) {
                        auto it = Minimize ? reference.begin() : std::prev(reference.end());
                        require(current.top() == *it);
                        require(current.try_top() == *it);
                        require(current.extract_top() == *it);
                        reference.erase(it);
                    }
                    require(current.empty());
                    require(!current.try_top());
                    require(!current.extract_top());
                    current.emplace(T{7});
                    require(current.extract_top() == T{7});
                }
            }
        }
    }
}

template <class T>
static void small_both() {
    small_boundaries<T, true>(); small_boundaries<T, false>();
}

template <class T, bool Minimize>
static void larger_fourary_boundaries() {
    using Q = ErasablePriorityQueue<T, Minimize>;
    std::mt19937 rng(20261028);
    for (int n : {63, 64, 65, 255, 256, 257, 1023, 1024, 1025}) {
        const std::set<int> erased_counts{0, 1, 2, 3, 4, 15, 16, 17, 63, 64, 65,
                                         255, 256, 257, n / 2, n - 4, n - 3, n - 2, n - 1, n};
        for (int erased : erased_counts) {
            if (erased > n) continue;
            for (int pattern = 0; pattern < 4; ++pattern) {
                std::vector<T> values;
                for (int i = 0; i < n; ++i) {
                    const int raw = pattern == 3 ? i % 4 : (i * 17 + 3) % 83;
                    if constexpr (std::is_signed_v<T>) values.push_back(static_cast<T>(raw - 41));
                    else values.push_back(static_cast<T>(raw));
                }
                values[0] = std::numeric_limits<T>::lowest();
                values[1] = std::numeric_limits<T>::max();
                std::multiset<T> expected(values.begin(), values.end());
                Q queue;
                if (pattern & 1) for (T value : values) queue.push(value);
                else queue = Q(values);
                if (pattern == 1) std::reverse(values.begin(), values.end());
                if (pattern == 2) std::shuffle(values.begin(), values.end(), rng);
                for (int i = 0; i < erased; ++i) {
                    queue.erase(values[i]); expected.erase(expected.find(values[i]));
                }
                require(queue.size() == expected.size());
                Q copy(queue);
                for (int pass = 0; pass < 2; ++pass) {
                    auto& current = pass == 0 ? queue : copy;
                    auto reference = expected;
                    while (!reference.empty()) {
                        auto it = Minimize ? reference.begin() : std::prev(reference.end());
                        require(current.top() == *it);
                        require(current.try_top() == *it);
                        require(current.extract_top() == *it);
                        reference.erase(it);
                    }
                    require(current.empty());
                    require(!current.try_top());
                    require(!current.extract_top());
                    current.emplace(T{7}); require(current.extract_top() == T{7});
                }
            }
        }
    }
}
template <class T>
static void larger_fourary_both() {
    larger_fourary_boundaries<T, true>(); larger_fourary_boundaries<T, false>();
}

int main(){
 small_both<std::int8_t>();small_both<std::uint8_t>();
 small_both<std::int16_t>();small_both<std::uint16_t>();
 small_both<std::int32_t>();small_both<std::uint32_t>();
 small_both<std::int64_t>();small_both<std::uint64_t>();
 larger_fourary_both<std::int8_t>();larger_fourary_both<std::uint8_t>();
 larger_fourary_both<std::int16_t>();larger_fourary_both<std::uint16_t>();
 larger_fourary_both<std::int32_t>();larger_fourary_both<std::uint32_t>();
 larger_fourary_both<std::int64_t>();larger_fourary_both<std::uint64_t>();
 std::printf("%zu boundary checks passed\n",checks);
}
