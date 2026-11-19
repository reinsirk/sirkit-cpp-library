// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Suffix_Array.cpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SKIP_RUNTIME_CHECKS
namespace {

template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

} // namespace
#endif

int main() {
    const std::string input("banana");
    const std::vector<int> suffixes{5, 3, 1, 0, 4, 2};
    const std::vector<int> lcp{1, 3, 0, 0, 2};
    assert(sirkit::suffix_array(input) == suffixes);
    assert(sirkit::lcp_array(input, suffixes) == lcp);
    assert((sirkit::SuffixArrayEqualRange(input, suffixes, "ana") == std::pair{1, 3}));
    assert((sirkit::SuffixArrayEqualRange(input, suffixes, "") == std::pair{0, 6}));
    assert((sirkit::SuffixArrayEqualRange(input, suffixes, "z") == std::pair{6, 6}));
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([&] { (void)sirkit::SuffixArrayEqualRange(input, {5, 3}, "ana"); });
    for (int index : {-1, 6}) {
        auto invalid = suffixes;
        invalid[3] = index;
        rejects<std::out_of_range>([&] { (void)sirkit::SuffixArrayEqualRange(input, invalid, "ana"); });
        rejects<std::invalid_argument>([&] { (void)sirkit::lcp_array(input, invalid); });
    }
    auto duplicate = suffixes;
    duplicate[3] = duplicate[2];
    rejects<std::invalid_argument>([&] { (void)sirkit::lcp_array(input, duplicate); });
#endif
}
