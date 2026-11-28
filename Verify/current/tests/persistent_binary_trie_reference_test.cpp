// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Persistent_Binary_Trie.cpp"
#include <cassert>
#include <random>
#include <string_view>
#include <utility>

namespace
{
using Trie = sirkit::PersistentBinaryTrie<std::uint32_t, 4>;
using Version = Trie::Version;

void test_branches()
{
    Trie trie(30);
    const Version empty;
    const auto one = trie.insert(empty, 9);
    const auto twice = trie.insert(one, 9);
    const auto other = trie.insert(one, 12);
    const auto gone = trie.erase(one, 9);
    assert(gone && trie.size(*gone) == 0);
    assert(!trie.erase(one, 7));
    assert(trie.count(twice, 9) == 2);
    assert(trie.count(other, 9) == 1);
    const auto one_removed = trie.erase(other, 12);
    assert(one_removed && trie.count(*one_removed, 12) == 0 && trie.count(*one_removed, 9) == 1);
    assert(trie.kth_xor(*one_removed, twice, 0, 31) == 22);
    assert(!trie.kth_xor(one, one, 0));
    assert(!trie.kth_xor(one, twice, 1));
    assert(trie.count_xor_less(one, twice, 40) == 1);
    assert(trie.count_xor_less(one, other, 12) == 0);
    assert(trie.count_xor_less(one, other, 13) == 1);

    sirkit::PersistentBinaryTrie<std::uint64_t> wide;
    const auto high = wide.insert({}, std::uint64_t{1} << 63);
    const auto all = wide.insert(high, std::numeric_limits<std::uint64_t>::max());
    assert(wide.kth_xor({}, all, 0, std::uint64_t{1} << 63) == 0);
    assert(wide.kth_xor({}, all, 1) == std::numeric_limits<std::uint64_t>::max());
    assert(wide.count_xor_less({}, all, std::numeric_limits<std::uint64_t>::max()) == 1);
}

void test_random()
{
    std::mt19937 random(14901);
    Trie trie;
    std::vector<Version> versions(1);
    std::vector<std::vector<std::uint32_t>> copies(1);
    for (int update = 0; update < 550; ++update)
    {
        const auto source = random() % versions.size();
        std::uint32_t value = random() % 32;
        auto model = copies[source];
        if (update % 3 == 0 && !model.empty())
        {
            auto old = model.begin() + random() % model.size();
            versions.push_back(trie.erase(versions[source], *old).value());
            model.erase(old);
        }
        else
        {
            versions.push_back(trie.insert(versions[source], value));
            model.push_back(value);
        }
        copies.push_back(model);
        const auto last = versions.back();
        assert(trie.size(last) == model.size());
        assert(trie.count(last, value) == std::count(model.begin(), model.end(), value));
        const auto mask = random() % 32;
        const auto bound = random() % 35;
        std::vector<std::uint32_t> xors;
        for (const auto key : model)
        {
            xors.push_back(key ^ mask);
        }
        std::sort(xors.begin(), xors.end());
        for (std::uint32_t k = 0; k < xors.size(); ++k)
        {
            assert(trie.kth_xor({}, last, k, mask) == xors[k]);
        }
        assert(trie.count_xor_less({}, last, bound, mask) == std::count_if(xors.begin(), xors.end(), [&](auto x)
                                                                           {
                                                                               return x < bound;
                                                                           }));
        const auto old = versions[source];
        assert(trie.size(old) == copies[source].size());
        if (copies[source].size() < model.size())
        {
            assert(trie.kth_xor(old, last, 0, mask) == (value ^ mask));
        }
    }
}

template <class Error, class Function>
void expect_failure(Function fn)
{
    bool rejected = false;
    try
    {
        fn();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        const Trie too_large(std::numeric_limits<std::size_t>::max());
        return 1;
    }
    expect_failure<std::length_error>([]
                                      {
                                          const Trie t(std::numeric_limits<std::size_t>::max());
                                      });
#ifndef SKIP_RUNTIME_CHECKS
    Trie t(2);
    const auto version = t.insert({}, 10);
    const Trie different;
    expect_failure<std::out_of_range>([&]
                                      {
                                          different.size(version);
                                      });
    expect_failure<std::out_of_range>([&]
                                      {
                                          t.insert(version, 32);
                                      });
    expect_failure<std::invalid_argument>([&]
                                          {
                                              t.kth_xor(version, {}, 0);
                                          });
#endif
    test_branches();
    test_random();
}
