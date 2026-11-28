// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SUFFIX_TREE_DEFINED
#include "../../../String/Suffix_Tree.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
std::size_t naive_repeat_length(std::string_view text)
{
    std::size_t best = 0;
    for (std::size_t left = 0; left < text.size(); ++left)
    {
        for (std::size_t right = left + 1; right < text.size(); ++right)
        {
            std::size_t length = 0;
            while (length < right - left && length < text.size() - right && text[left + length] == text[right + length])
            {
                ++length;
            }
            best = std::max(best, length);
        }
    }
    return best;
}

void compare(std::string_view text)
{
    const sirkit::SuffixTree tree(text);
    const auto nodes = tree.nodes();
    assert(!nodes.empty());
    assert(nodes.front().parent == -1 && nodes.front().depth == 0 && nodes.front().suffix_start == -1);
    assert(tree.edge_label(0).empty());
    std::vector<int> expected_order;
    std::set<std::string_view> substrings;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        expected_order.push_back(static_cast<int>(i));
        for (std::size_t length = 1; length <= text.size() - i; ++length)
        {
            substrings.insert(text.substr(i, length));
        }
    }
    std::ranges::sort(expected_order, [text](int a, int b) {
        return text.substr(static_cast<std::size_t>(a)) < text.substr(static_cast<std::size_t>(b));
    });
    assert(std::ranges::equal(tree.suffixes(0), expected_order));

    std::vector<std::string> labels(nodes.size());
    std::vector<int> visit{0};
    std::vector<bool> seen(nodes.size());
    std::size_t edge_lengths = 0;
    for (std::size_t i = 0; i < visit.size(); ++i)
    {
        const auto vertex = visit[i];
        assert(!seen[vertex]);
        seen[vertex] = true;
        const auto &node = nodes[vertex];
        const auto kids = tree.children(vertex);
        const auto &path = labels[vertex];
        assert(path.size() == static_cast<std::size_t>(node.depth));
        int first_unpartitioned = node.first_suffix;
        if (node.suffix_start >= 0)
        {
            assert(text.substr(static_cast<std::size_t>(node.suffix_start)) == path);
            assert(expected_order[first_unpartitioned] == node.suffix_start);
            ++first_unpartitioned;
        }
        else if (vertex != 0)
        {
            assert(kids.size() >= 2);
        }
        int last_byte = -1;
        for (const int child : kids)
        {
            const auto edge = tree.edge_label(child);
            assert(!edge.empty());
            const auto byte = static_cast<unsigned char>(edge.front());
            assert(byte > last_byte);
            last_byte = byte;
            edge_lengths += edge.size();
            assert(nodes[child].parent == vertex);
            assert(nodes[child].first_suffix == first_unpartitioned);
            first_unpartitioned = nodes[child].last_suffix;
            labels[child] = path + std::string(edge);
            visit.push_back(child);
        }
        assert(first_unpartitioned == node.last_suffix);

        std::vector<int> expected;
        for (const int start : expected_order)
        {
            if (text.substr(static_cast<std::size_t>(start)).starts_with(path))
            {
                expected.push_back(start);
            }
        }
        assert(std::ranges::equal(tree.suffixes(vertex), expected));
    }
    assert(visit.size() == nodes.size());
    assert(edge_lengths == substrings.size());
    const auto repeat = tree.longest_nonoverlapping_repeat();
    assert(static_cast<std::size_t>(repeat.length) == naive_repeat_length(text));
    const auto left = static_cast<std::size_t>(repeat.first_start);
    const auto right = static_cast<std::size_t>(repeat.second_start);
    const auto length = static_cast<std::size_t>(repeat.length);
    assert(left + length <= right && right + length <= text.size());
    assert(text.substr(left, length) == text.substr(right, length));
}

template <class Exception, class Function>
void expect_throw(Function action)
{
    bool threw = false;
    try
    {
        action();
    }
    catch (const Exception &)
    {
        threw = true;
    }
    assert(threw);
}
} // namespace

int main()
{
    compare("");
    compare("a");
    compare("aaa");
    compare("abcdef");
    compare("banana");
    compare("xabxa");
    compare(std::string{char(255), '\0', char(127), char(255), '\0'});
    std::mt19937 random(42375);
    for (int trial = 0; trial < 1500; ++trial)
    {
        std::string text(random() % 52, '\0');
        const unsigned alphabet = 2U << (trial % 8);
        for (auto &byte : text)
        {
            byte = static_cast<char>(random() % alphabet);
        }
        compare(text);
    }

    const std::string repeated(200'000, 'a');
    const sirkit::SuffixTree large(repeated);
    assert(large.nodes().size() == repeated.size() + 1);
    assert(large.longest_nonoverlapping_repeat().length == 100'000);
    assert(large.suffixes(0).front() == 199'999 && large.suffixes(0).back() == 0);

    std::string source = "abab";
    sirkit::SuffixTree tree(source);
    source.assign("abcd");
    const auto edge = tree.edge_label(tree.children(0).front());
    const auto copy = tree;
    tree = large;
    assert(edge == "ab");
    assert(copy.longest_nonoverlapping_repeat().length == 2);
    assert(tree.longest_nonoverlapping_repeat().length == 100'000);

    constexpr auto kLimit = static_cast<std::size_t>((std::numeric_limits<int>::max() - 1) / 2);
    sirkit::detail::suffix_tree_length_check(kLimit);
    expect_throw<std::length_error>([] { sirkit::detail::suffix_tree_length_check(kLimit + 1); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_throw<std::out_of_range>([&] { (void)copy.children(-1); });
    expect_throw<std::out_of_range>([&] { (void)copy.suffixes(static_cast<int>(copy.nodes().size())); });
#endif
}
