// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Palindromic_Tree.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

int main() {
    assert(sirkit::detail::StringTreeGrowth(7, 8, 12) == 12);
    sirkit::detail::CheckStringTreeSlots(1, 3, 2, 1, "test");
    try {
        sirkit::detail::CheckStringTreeSlots(2, 3, 2, 1, "test");
        assert(false);
    } catch (const std::length_error&) {
    }
    sirkit::PalindromicTree<> tree("ababa");
    assert(tree.length(-1) == -1 && tree.length(0) == 0);
    for (const std::int32_t root : {-1, 0}) {
        assert(tree.parent(root) == -1);
        assert(tree.suffix_link(root) == -1);
        assert(tree.first_position(root) == 0);
    }
    assert(!tree.find("{"));
    tree.reserve(0);
    tree.reserve(tree.size());

    const auto& same = tree;
    tree = same;
    assert(tree.text() == "ababa" && tree.occurrences("aba") == 2);
    sirkit::PalindromicTree<> other("bbb");
    other = std::move(tree);
    assert(tree.empty() && other.text() == "ababa" && other.occurrences("aba") == 2);

    try {
        other.reserve(std::numeric_limits<std::size_t>::max());
        assert(false);
    } catch (const std::length_error&) {
        assert(other.text() == "ababa");
    }

#ifndef SKIP_RUNTIME_CHECKS
    try {
        other.append(static_cast<std::uint8_t>('{'));
        assert(false);
    } catch (const std::invalid_argument&) {
        assert(other.text() == "ababa");
    }
    try {
        (void)other.length(-2);
        assert(false);
    } catch (const std::out_of_range&) {
    }
#endif
}
