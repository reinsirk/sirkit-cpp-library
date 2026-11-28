// SPDX-License-Identifier: CC0-1.0
#include "../../../String/String_Radix_Order.cpp"

#include <cassert>
#include <random>

static std::vector<std::size_t> reference(const std::vector<std::string> &words, std::string_view alphabet)
{
    const auto byte_order = [alphabet](char byte) {
        const auto index = alphabet.find(byte);
        return index == std::string_view::npos ? alphabet.size() + static_cast<unsigned char>(byte) : index;
    };
    std::vector<std::vector<std::size_t>> keys(words.size());
    for (std::size_t i = 0; i < words.size(); ++i)
    {
        for (const auto letter : words[i])
        {
            keys[i].push_back(byte_order(letter));
        }
    }
    std::vector<std::size_t> ordered(words.size());
    std::iota(ordered.begin(), ordered.end(), std::size_t{0});
    std::stable_sort(ordered.begin(), ordered.end(), [&](auto first, auto second) {
        return keys[first] < keys[second];
    });
    return ordered;
}

int main()
{
    const std::vector<std::string> sample{"ab", "a", "ab", "ba", "b"};
    assert((sirkit::string_radix_order(sample, "ba") == std::vector<std::size_t>{4, 3, 1, 0, 2}));
    assert((sirkit::string_radix_order(sample) == std::vector<std::size_t>{1, 0, 2, 4, 3}));
    assert(sirkit::string_radix_order(std::vector<std::string>{}).empty());
    assert((sirkit::string_radix_order(std::vector<std::string>{"a"}) == std::vector<std::size_t>{0}));
    const std::array<std::string_view, 3> empty_words{"", "", ""};
    assert((sirkit::string_radix_order(empty_words) == std::vector<std::size_t>{0, 1, 2}));

    std::vector<std::string> all_bytes;
    std::string alphabet;
    for (unsigned int value = 0; value < 256; ++value)
    {
        all_bytes.emplace_back(1, static_cast<char>(value));
        alphabet.push_back(static_cast<char>(255 - value));
    }
    assert(sirkit::string_radix_order(all_bytes, alphabet) == reference(all_bytes, alphabet));
    assert(sirkit::string_radix_order(all_bytes) == reference(all_bytes, {}));
    const std::vector<std::string> preserve_duplicates{"b", "b", "", "b", "bc", "c", "g", "d", ""};
    const std::string_view partial = "b";
    assert(sirkit::string_radix_order(preserve_duplicates, partial) == reference(preserve_duplicates, partial));

    const std::string long_prefix(20000, 'p');
    const std::array<std::string_view, 3> long_words{long_prefix, long_prefix, long_prefix};
    assert((sirkit::string_radix_order(long_words) == std::vector<std::size_t>{0, 1, 2}));
#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try
    {
        (void)sirkit::string_radix_order(sample, "aa");
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
#endif

    std::mt19937 rng(2697);
    for (int test = 0; test < 1600; ++test)
    {
        const auto sigma = 1 + rng() % 38;
        std::vector<std::string> words(rng() % 30);
        for (auto &word : words)
        {
            word.resize(rng() % 18);
            for (auto &byte : word)
            {
                byte = static_cast<char>(129 + rng() % sigma);
            }
        }
        if (!words.empty())
        {
            words.back() = words.front();
        }
        std::shuffle(alphabet.begin(), alphabet.end(), rng);
        const auto order = std::string_view{alphabet}.substr(0, rng() % 257);
        assert(sirkit::string_radix_order(words, order) == reference(words, order));
        std::vector<std::string_view> views(words.begin(), words.end());
        assert(sirkit::string_radix_order(views, order) == reference(words, order));
    }
}
