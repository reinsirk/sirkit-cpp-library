// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Manacher.cpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
struct RepeatedText {
    std::size_t length;
    std::size_t* reads;

    std::size_t size() const { return length; }
    char operator[](int index) const {
        assert(index >= 0 && static_cast<std::size_t>(index) < length);
        ++*reads;
        return 'x';
    }
};
}

int main() {
    static_assert(sizeof(void*) == 4);
    const auto capacity = std::vector<int>{}.max_size();
    const auto result_overflow = capacity / 2 + capacity % 2 + 1;
    assert(result_overflow < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    std::size_t reads = 0;
    bool rejected = false;
    try {
        (void)sirkit::PalindromicLengths(RepeatedText{result_overflow, &reads});
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected && reads == 0);
    const std::vector<int> expected{1, 2, 3, 2, 1};
    assert(sirkit::PalindromicLengths(RepeatedText{3, &reads}) == expected);
    assert(reads > 0);
}
