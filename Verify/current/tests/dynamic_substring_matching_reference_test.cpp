// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Dynamic_Substring_Matching.cpp"
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
void check(const sirkit::DynamicSubstringMatching &index, std::string_view text, std::string_view pattern)
{
    std::vector<std::size_t> expected;
    for (std::size_t position = 0; position + pattern.size() <= text.size(); ++position)
    {
        if (text.substr(position, pattern.size()) == pattern)
        {
            expected.push_back(position);
        }
    }
    assert(index.size() == text.size());
    assert(index.contains(pattern) == !expected.empty());
    assert(index.count(pattern) == expected.size());
    assert(index.occurrences(pattern) == expected);
}
} // namespace

int main()
{
    sirkit::DynamicSubstringMatching empty("");
    check(empty, "", "");
    check(empty, "", "a");
    bool failed = false;
    try
    {
        empty.set(0, 'a');
    }
    catch (const std::out_of_range &)
    {
        failed = true;
    }
    assert(failed);

    std::mt19937 rng(247628);
    const std::array alphabet{'\0', 'c', static_cast<char>(0xff), 'a'};
    for (const auto length : std::array<std::size_t, 12>{1, 2, 62, 63, 64, 65, 66, 127, 128, 129, 191, 256})
    {
        std::string bytes(length, 'a');
        sirkit::DynamicSubstringMatching index(bytes);
        check(index, bytes, "a");
        check(index, bytes, std::string(64, 'a'));
        check(index, bytes, std::string(65, 'a'));
        check(index, bytes, std::string(66, 'a'));
        check(index, bytes, "ab");
        for (int trial = 0; trial < 150; ++trial)
        {
            const auto change = static_cast<std::size_t>(rng() % length);
            const auto byte = alphabet[rng() % alphabet.size()];
            bytes[change] = byte;
            index.set(change, byte);
            const auto start = static_cast<std::size_t>(rng() % length);
            const auto count = static_cast<std::size_t>(rng() % (length - start + 1));
            auto pattern = bytes.substr(start, count);
            check(index, bytes, pattern);
            const auto prefix = static_cast<std::size_t>(rng() % 129);
            pattern.assign(prefix, alphabet[rng() % alphabet.size()]);
            pattern.push_back(alphabet[rng() % alphabet.size()]);
            check(index, bytes, pattern);
        }
        const auto duplicate = index;
        check(duplicate, bytes, bytes);
    }
    sirkit::DynamicSubstringMatching original("ababa");
    auto moved = std::move(original);
    check(original, "", "");
    check(original, "", "ab");
    original = moved;
    moved.set(0, 'x');
    check(original, "ababa", "ab");
    original = std::move(moved);
    check(original, "xbaba", "ab");
    check(moved, "", "xb");
}
