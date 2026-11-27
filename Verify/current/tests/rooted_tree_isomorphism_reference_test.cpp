// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Rooted_Tree_Isomorphism.cpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

int main()
{
    try
    {
        sirkit::detail::rooted_tree_vertex_count(static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1);
        assert(false);
    }
    catch (const std::length_error &)
    {
    }
    std::mt19937 random(907);
    for (int size = 0; size <= 46; ++size)
    {
        for (int trial = 0; trial < 60; ++trial)
        {
            std::vector<int> label(size), parent(size, -1);
            std::iota(label.begin(), label.end(), 0);
            std::ranges::shuffle(label, random);
            std::vector<std::vector<int>> children(size);
            for (int v = 1; v < size; ++v)
            {
                if (random() % 9 != 0)
                {
                    const auto p = label[random() % v];
                    parent[label[v]] = p;
                    children[p].push_back(label[v]);
                }
            }
            const auto encode = [&](auto &&self, int v) -> std::string
            {
                std::vector<std::string> terms;
                for (auto child : children[v])
                {
                    terms.push_back(self(self, child));
                }
                std::ranges::sort(terms);
                std::string result = "(";
                for (const auto &term : terms)
                {
                    result += term;
                }
                return result + ")";
            };
            std::vector<std::string> expected;
            for (int v = 0; v < size; ++v)
            {
                expected.push_back(encode(encode, v));
            }
            const auto result = sirkit::rooted_subtree_classes(parent);
            assert(static_cast<std::size_t>(result.count) == std::set(expected.begin(), expected.end()).size());
            for (int u = 0; u < size; ++u)
            {
                for (int v = 0; v < size; ++v)
                {
                    assert((result.class_id[u] == result.class_id[v]) == (expected[u] == expected[v]));
                }
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (int p : {-2, 1})
    {
        try
        {
            sirkit::rooted_subtree_classes({&p, 1});
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
    try
    {
        const int p = 0;
        sirkit::rooted_subtree_classes({&p, 1});
        assert(false);
    }
    catch (const std::invalid_argument &)
    {
    }
#endif
}
