// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Dynamic_Prefix_Filter.cpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <random>
#include <set>
#include <string>

int main()
{
    sirkit::DynamicPrefixFilter filter;
    assert(filter.size() == 0 && filter.allowed_count() == 0);
    assert(filter.count("") == 0 && !filter.is_blocked(""));
    assert(!filter.unblock("x") && !filter.erase("x"));
    assert(filter.insert("a", 0) == 0 && filter.erase("a", 0));
    assert(filter.insert("") == 1);
    assert(filter.insert("app", 3) == 3);
    assert(filter.insert("apple", 2) == 2);
    assert(filter.insert("banana", 4) == 4);
    assert(filter.size() == 10 && filter.allowed_count() == 10);
    assert(!filter.erase("apple", 3));
    assert(filter.block("ap"));
    assert(filter.block("apple") && !filter.block("apple"));
    assert(filter.is_blocked("apt") && filter.is_blocked("apple"));
    assert(!filter.is_blocked("banana") && !filter.is_blocked("carrot"));
    assert(filter.count("apple") == 2 && filter.allowed_count() == 5);
    assert(filter.insert("apple", 2) == 4);
    assert(filter.erase("app", 2) && filter.count("app") == 1);
    assert(filter.unblock("ap") && !filter.unblock("ap"));
    assert(filter.allowed_count() == 6);
    assert(filter.block("") && filter.allowed_count() == 0);
    assert(filter.is_blocked("") && filter.is_blocked("missing"));
    assert(filter.erase("") && filter.insert("banana", 3) == 7);
    assert(filter.unblock("apple") && filter.allowed_count() == 0);
    assert(filter.unblock("") && filter.allowed_count() == 12 && filter.size() == 12);
    assert(!filter.unblock("absent") && !filter.is_blocked("apple"));

    assert(filter.block("app"));
    auto copy = filter;
    assert(copy.unblock("app") && copy.allowed_count() == 12);
    assert(filter.is_blocked("apple") && filter.allowed_count() == 7);
    auto moved = std::move(copy);
    assert(copy.allowed_count() == 0 && copy.size() == 0);
    assert(moved.allowed_count() == 12);
    copy = filter;
    assert(copy.is_blocked("app") && copy.allowed_count() == 7);
    copy = std::move(moved);
    assert(copy.allowed_count() == 12 && moved.size() == 0);
    auto &alias = copy;
    copy = std::move(alias);
    assert(copy.allowed_count() == 12);
    assert(moved.insert("reused") == 1);

    const std::string binary{static_cast<char>(0), static_cast<char>(255), static_cast<char>(205)};
    assert(copy.insert(binary) == 1 && copy.allowed_count() == 13);
    assert(copy.block(std::string_view{binary}.substr(0, 2)));
    assert(copy.is_blocked(binary) && copy.allowed_count() == 12);
    assert(copy.unblock(std::string_view{binary}.substr(0, 2)) && copy.allowed_count() == 13);

    sirkit::DynamicPrefixFilter large_count;
    constexpr auto kMax = std::numeric_limits<std::size_t>::max();
    assert(large_count.insert("x", kMax - 1) == kMax - 1);
    assert(large_count.insert("y") == 1 && large_count.allowed_count() == kMax);
    assert(large_count.block("x") && large_count.allowed_count() == 1);
    bool overflow = false;
    try
    {
        (void)large_count.insert("x");
    }
    catch (const std::length_error &)
    {
        overflow = true;
    }
    assert(overflow && large_count.size() == kMax && large_count.count("x") == kMax - 1);
    assert(large_count.unblock("x") && large_count.allowed_count() == kMax);
    assert(large_count.erase("x", kMax - 1) && large_count.allowed_count() == 1);

    sirkit::DynamicPrefixFilter deep;
    const std::string chain(120000, 'a');
    assert(deep.insert(chain) == 1);
    assert(deep.block(std::string_view{chain}.substr(0, chain.size() - 1)) && deep.allowed_count() == 0);
    assert(deep.insert(chain) == 2 && deep.erase(chain));
    assert(deep.unblock(std::string_view{chain}.substr(0, chain.size() - 1)) && deep.allowed_count() == 1);

    sirkit::DynamicPrefixFilter actual;
    std::map<std::string, std::size_t> counts;
    std::set<std::string> blocked;
    const auto is_blocked = [&](std::string_view word) {
        return std::any_of(blocked.begin(), blocked.end(), [word](std::string_view prefix) {
            return word.starts_with(prefix);
        });
    };
    std::mt19937 rng(44139);
    for (int query = 0; query < 6000; ++query)
    {
        std::string key(rng() % 5, 'a');
        for (auto &byte : key)
        {
            byte = static_cast<char>('a' + rng() % 3);
        }
        std::size_t copies = rng() % 6;
        switch (rng() % 4)
        {
        case 0:
            counts[key] += copies;
            assert(actual.insert(key, copies) == counts[key]);
            break;
        case 1:
            if (counts[key] >= copies)
            {
                counts[key] -= copies;
                assert(actual.erase(key, copies));
            }
            else
            {
                assert(!actual.erase(key, copies));
            }
            break;
        case 2:
            assert(actual.block(key) == blocked.insert(key).second);
            break;
        default:
            assert(actual.unblock(key) == (blocked.erase(key) != 0));
            break;
        }
        assert(actual.count(key) == counts[key] && actual.is_blocked(key) == is_blocked(key));
        std::size_t total = 0, allowed = 0;
        for (const auto &[word, count] : counts)
        {
            total += count;
            if (!is_blocked(word))
            {
                allowed += count;
            }
        }
        assert(actual.size() == total && actual.allowed_count() == allowed);
    }
}
