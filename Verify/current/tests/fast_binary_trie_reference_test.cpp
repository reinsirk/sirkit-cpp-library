// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#ifndef NORMAL_TRIE_SOURCE
#define NORMAL_TRIE_SOURCE "../../../DataStructure/Binary_Trie.cpp"
#endif
#ifndef FAST_TRIE_SOURCE
#define FAST_TRIE_SOURCE "../../../Fast/GCC/DataStructure/Binary_Trie.cpp"
#endif
#define BinaryTrie ReferenceBinaryTrie
#include NORMAL_TRIE_SOURCE
#undef BinaryTrie
#undef SIRKIT_BINARY_TRIE_DEFINED
#define BinaryTrie SpeedBinaryTrie
#include FAST_TRIE_SOURCE
#undef BinaryTrie

int main() {
    std::mt19937_64 random(0x735f12ac);
    std::uint64_t checks = 0;
    auto check = [&](bool value) { ++checks; assert(value); };
    for (int run = 0; run < 8; ++run) {
        ReferenceBinaryTrie<std::uint32_t, 11, 65536> normal;
        SpeedBinaryTrie<std::uint32_t, 11, 65536> fast;
        std::map<std::uint32_t, std::vector<int>> model;
        std::size_t total = 0;
        auto ids_equal = [&](const auto& actual, const std::vector<int>& stored) {
            auto a = actual;
            std::vector<int> b;
            for (int id : stored) if (id != -1) b.push_back(id);
            std::sort(a.begin(), a.end()); std::sort(b.begin(), b.end());
            check(a == b);
        };
        for (int step = 0; step < 12000; ++step) {
            const auto key = static_cast<std::uint32_t>(random() & 4095);
            const int id = random() % 3 == 0 ? -1 : static_cast<int>(random() % 31);
            switch (random() % 8) {
            case 0: case 1:
                normal.add(key, id); fast.add(key, id); model[key].push_back(id); ++total;
                break;
            case 2: {
                const bool expected = !model.contains(key);
                check(normal.insert_unique(key, id) == expected);
                check(fast.insert_unique(key, id) == expected);
                if (expected) { model[key].push_back(id); ++total; }
                break;
            }
            case 3: case 4: {
                auto item = model.find(key);
                bool expected = item != model.end();
                int removed = id;
                if (expected && id == -1) {
                    if (std::find(item->second.begin(), item->second.end(), -1) == item->second.end())
                        removed = normal.find(key).second.back();
                }
                if (expected) expected = std::find(item->second.begin(), item->second.end(), removed) != item->second.end();
                check(normal.del(key, id) == expected); check(fast.del(key, id) == expected);
                if (expected) {
                    auto& values = item->second;
                    auto at = std::find(values.begin(), values.end(), removed);
                    values.erase(at); --total;
                    if (values.empty()) model.erase(item);
                }
                break;
            }
            case 5:
                normal.operate_xor(key); fast.operate_xor(key);
                { decltype(model) next;
                  for (auto& [value, ids] : model) next.emplace(value ^ key, std::move(ids));
                  model = std::move(next); }
                break;
            default: break;
            }
            check(normal.size() == total); check(fast.size() == total);
            check(fast.empty() == model.empty());
            const auto found = model.find(key);
            const auto count = found == model.end() ? 0 : found->second.size();
            check(normal.count(key) == count); check(fast.count(key) == count);
            check(fast.contains(key) == (count != 0));
            ids_equal(fast.find(key).second, found == model.end() ? std::vector<int>{} : found->second);
            const auto bound = static_cast<std::int64_t>(random() % 8192) - 2048;
            std::size_t less = 0, xor_less = 0;
            std::uint32_t minimum_xor = 4096, maximum_xor = 0;
            for (const auto& [value, ids] : model) {
                if (static_cast<std::int64_t>(value) < bound) less += ids.size();
                if (static_cast<std::int64_t>(value ^ key) < bound) xor_less += ids.size();
                minimum_xor = std::min(minimum_xor, value ^ key);
                maximum_xor = std::max(maximum_xor, value ^ key);
            }
            check(fast.count_less(bound) == less); check(fast.count_xor_less(key, bound) == xor_less);
            if (!model.empty()) {
                check(fast.min_element().first == model.begin()->first);
                check(fast.max_element().first == model.rbegin()->first);
                check(fast.min_xor(key).first == minimum_xor); check(fast.max_xor(key).first == maximum_xor);
                const auto rank = static_cast<std::size_t>(random() % total) + 1;
                auto rest = rank;
                auto entry = model.begin();
                while (rest > entry->second.size()) { rest -= entry->second.size(); ++entry; }
                check(fast.get_kth(static_cast<std::int64_t>(rank)).first == entry->first);
                ids_equal(fast.get_kth(static_cast<std::int64_t>(rank)).second, entry->second);
            }
        }
        // 意図的に解放しない版なので、解放回数の同一性やLSanは検証条件にしない。
    }
    std::cout << "fast binary trie checks=" << checks << '\n';
}
