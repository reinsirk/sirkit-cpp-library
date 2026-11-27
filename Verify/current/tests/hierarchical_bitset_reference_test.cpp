// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Hierarchical_Bitset.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
constexpr auto kMaximum = std::numeric_limits<std::size_t>::max();
using sirkit::HierarchicalBitset;

template <class Exception, class Callback>
void ExpectThrow(Callback action)
{
    bool thrown = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        thrown = true;
    }
    assert(thrown);
}

void CheckEmpty()
{
    const HierarchicalBitset zero;
    assert(zero.size() == 0 && zero.empty() && zero.count() == 0);
    assert(!zero.next(0) && !zero.prev(0));
    assert(!zero.next(kMaximum) && !zero.prev(kMaximum));
    const HierarchicalBitset empty_text("");
    assert(empty_text.size() == 0);
    const HierarchicalBitset empty_bits("000000");
    assert(empty_bits.size() == 6 && empty_bits.empty());
    assert(!empty_bits.next(2) && !empty_bits.prev(2));
#ifndef SKIP_RUNTIME_CHECKS
    ExpectThrow<std::invalid_argument>([]
                                       {
                                           (void)HierarchicalBitset("010z1");
                                       });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)zero.contains(0);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       (void)empty_bits.contains(6);
                                   });
    HierarchicalBitset change(8);
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       change.insert(8);
                                   });
    ExpectThrow<std::out_of_range>([&]
                                   {
                                       change.erase(kMaximum);
                                   });
#endif
}

void CheckLayers()
{
    constexpr std::size_t kLength = 64 * 64 * 64 + 1;
    constexpr std::array<std::size_t, 8> points{0, 1, 62, 63, 64, 4096, kLength - 2, kLength - 1};
    HierarchicalBitset set(kLength);
    for (const auto point : points)
    {
        assert(set.insert(point) && !set.insert(point));
        assert(set.contains(point));
        assert(set.next(point) == point && set.prev(point) == point);
    }
    assert(set.count() == points.size());
    assert(set.next(65) == 4096);
    assert(set.next(4097) == kLength - 2);
    assert(set.prev(4097) == 4096);
    assert(set.prev(kLength - 3) == 4096);
    assert(set.prev(kMaximum) == kLength - 1);
    assert(!set.next(kMaximum));
    HierarchicalBitset copy = set;
    HierarchicalBitset assigned;
    assigned = copy;
    HierarchicalBitset moved = std::move(copy);
    assert(copy.empty() && copy.size() == 0);
    copy = std::move(moved);
    assert(moved.empty() && moved.size() == 0);
    assert(copy.count() == points.size());
    for (const auto point : points)
    {
        assert(set.erase(point) && !set.erase(point));
        assert(!set.contains(point) && assigned.contains(point));
    }
    assert(set.empty() && !set.next(0) && !set.prev(kLength - 1));

    // 1つ上の層でも境界の直後に利用できるビットがない。
    HierarchicalBitset aligned(64 * 64);
    assert(!aligned.next(4095));
    assert(!aligned.prev(0));
    aligned.insert(4095);
    assert(aligned.next(0) == 4095);
    assert(aligned.prev(4095) == 4095);
    assert(!aligned.prev(4094));
}

void CheckWordBoundaryByString()
{
    std::string source(4097, '0');
    source[63] = source[64] = source[127] = source[4096] = '1';
    HierarchicalBitset set(source);
    assert(set.count() == 4);
    assert(set.next(0) == 63 && set.next(64) == 64);
    assert(set.next(65) == 127 && set.next(128) == 4096);
    assert(set.prev(4100) == 4096 && set.prev(4095) == 127);
    assert(set.prev(126) == 64 && set.prev(63) == 63);
    assert(!set.prev(62));
    assert(set.erase(127) && set.next(65) == 4096);
    assert(set.erase(4096) && !set.next(65));
}

void CheckAgainstSet()
{
    std::mt19937_64 random(0x39159c3228039a6fULL);
    for (const std::size_t n : {1, 63, 64, 65, 4095, 4096, 4097, 262145})
    {
        std::string initial(n, '0');
        std::set<std::size_t> expected;
        for (auto i = std::size_t{0}; i < n; ++i)
        {
            if (random() % 17 == 0)
            {
                expected.insert(i);
                initial[i] = '1';
            }
        }
        HierarchicalBitset actual(initial);
        for (auto step = 0; step < 9000; ++step)
        {
            const auto at = static_cast<std::size_t>(random() % n);
            if (step % 5 == 0)
            {
                assert(actual.insert(at) == expected.insert(at).second);
            }
            else if (step % 5 == 1)
            {
                assert(actual.erase(at) == (expected.erase(at) != 0));
            }
            assert(actual.contains(at) == expected.contains(at));
            const auto next = expected.lower_bound(at);
            const auto after_previous = expected.upper_bound(at);
            const auto want_next = next == expected.end() ? std::nullopt : std::optional(*next);
            const auto want_prev = after_previous == expected.begin() ? std::nullopt : std::optional(*std::prev(after_previous));
            assert(actual.next(at) == want_next && actual.prev(at) == want_prev);
            assert(actual.count() == expected.size() && actual.empty() == expected.empty());
        }
    }
}
}

int main()
{
    CheckEmpty();
    CheckLayers();
    CheckWordBoundaryByString();
    CheckAgainstSet();
}
