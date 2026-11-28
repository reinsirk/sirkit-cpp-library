#include "../String/Dictionary_Segmenter.cpp"

#include <array>
#include <cassert>
#include <string_view>

int main()
{
    constexpr std::array<std::string_view, 3> words{"a", "ab", "ba"};
    const sirkit::DictionarySegmenter<> segmenter(words);
    const auto pieces = segmenter.minimum_segmentation("aba");
    assert(pieces && pieces->size() == 2);
}
