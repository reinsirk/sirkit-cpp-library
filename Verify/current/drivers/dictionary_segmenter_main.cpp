// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <array>
#include <string>
#include <string_view>

int main()
{
    std::string text;
    verify_in >> text;
    constexpr std::array<std::string_view, 4> words{"dream", "dreamer", "erase", "eraser"};
    const sirkit::DictionarySegmenter<> segmenter(words);
    verify_out << (segmenter.minimum_segmentation(text) ? "YES" : "NO") << '\n';
}
