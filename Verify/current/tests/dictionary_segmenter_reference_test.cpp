#include "../../../String/Dictionary_Segmenter.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
std::size_t brute(std::string_view text, const std::vector<std::string> &dictionary)
{
    const auto unreachable = text.size() + 1;
    std::vector<std::size_t> words(text.size() + 1, unreachable);
    words[0] = 0;
    for (std::size_t begin = 0; begin < text.size(); ++begin)
    {
        if (words[begin] == unreachable)
        {
            continue;
        }
        for (const auto &word : dictionary)
        {
            if (!word.empty() && text.substr(begin).starts_with(word))
            {
                auto &next = words[begin + word.size()];
                next = std::min(next, words[begin] + 1);
            }
        }
    }
    return words.back();
}

std::string generate_word(std::mt19937 &random, std::size_t size)
{
    std::string word(size, 'a');
    for (auto &byte : word)
    {
        byte = static_cast<char>('a' + random() % 3);
    }
    return word;
}

void verify(const sirkit::DictionarySegmenter<> &segmenter, std::string_view text,
            const std::vector<std::string> &dictionary)
{
    const auto shortest = brute(text, dictionary);
    const auto result = segmenter.minimum_segmentation(text);
    if (shortest > text.size())
    {
        assert(!result);
        return;
    }
    assert(result && result->size() == shortest);
    std::string built;
    for (const auto id : *result)
    {
        assert(id < dictionary.size() && !dictionary[id].empty());
        built += dictionary[id];
    }
    assert(built == text);
}
}

int main()
{
    const std::vector<std::string> no_words;
    verify(sirkit::DictionarySegmenter<>(no_words), "", no_words);
    verify(sirkit::DictionarySegmenter<>(no_words), "z", no_words);

    const std::vector<std::string> duplicates{"", "ab", "ab", "a", "b"};
    const sirkit::DictionarySegmenter<> with_duplicates(duplicates);
    const auto one = with_duplicates.minimum_segmentation("ab");
    assert(one && *one == std::vector<std::size_t>{1});
    verify(with_duplicates, "cabab", duplicates);
    verify(with_duplicates, "abba", duplicates);

    const std::vector<std::string> ties{"ba", "a", "ab", "b"};
    verify(sirkit::DictionarySegmenter<>(ties), "aba", ties);
    const std::vector<std::string> other_tie{"ab", "a", "b", "ba"};
    verify(sirkit::DictionarySegmenter<>(other_tie), "aba", other_tie);

    auto from_local_words = []
    {
        const std::vector<std::string> words{"", "a", "b", "ab"};
        return sirkit::DictionarySegmenter<>(words);
    }();
    const auto copy = from_local_words;
    const auto moved = std::move(from_local_words);
    assert(!from_local_words.minimum_segmentation("ab"));
    assert(from_local_words.minimum_segmentation("")->empty());
    const auto expected = std::vector<std::size_t>{3};
    assert(copy.minimum_segmentation("ab") == expected);
    assert(moved.minimum_segmentation("ab") == expected);
    from_local_words = copy;
    assert(from_local_words.minimum_segmentation("ab") == expected);

    const std::array byte_words{std::string_view{"\xff\0", 2}, std::string_view{"\0", 1}};
    const sirkit::ByteDictionarySegmenter byte_segmenter(byte_words);
    const auto bytes = byte_segmenter.minimum_segmentation(std::string_view{"\xff\0\0", 3});
    assert(bytes && (*bytes == std::vector<std::size_t>{0, 1}));

    std::mt19937 random(49065);
    for (int trial = 0; trial < 2000; ++trial)
    {
        std::vector<std::string> dictionary(random() % 13);
        for (auto &word : dictionary)
        {
            word = generate_word(random, random() % 6);
        }
        const sirkit::DictionarySegmenter<> segmenter(dictionary);
        for (int query = 0; query < 5; ++query)
        {
            verify(segmenter, generate_word(random, random() % 27), dictionary);
        }
    }

    bool caught = false;
    try
    {
        (void)sirkit::detail::DictionarySegmentationBoundaryCount(12, 12);
    }
    catch (const std::length_error &)
    {
        caught = true;
    }
    assert(caught);
}
