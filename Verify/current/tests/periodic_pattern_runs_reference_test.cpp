#include "../../../String/Periodic_Pattern_Runs.cpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace
{
std::string binary_word(std::size_t length, unsigned bits)
{
    std::string word(length, 'a');
    for (std::size_t i = 0; i < length; ++i)
    {
        if (bits & (1U << i))
        {
            word[i] = 'b';
        }
    }
    return word;
}

sirkit::PeriodicPatternRun brute(std::string_view period, std::string_view pattern)
{
    std::size_t best = 0, start = 0;
    for (std::size_t i = 0; i < period.size(); ++i)
    {
        std::size_t copies = 0;
        for (; copies < period.size(); ++copies)
        {
            bool matches = true;
            for (std::size_t j = 0; j < pattern.size(); ++j)
            {
                if (period[(i + copies * pattern.size() + j) % period.size()] != pattern[j])
                {
                    matches = false;
                    break;
                }
            }
            if (!matches)
            {
                break;
            }
        }
        if (copies == period.size())
        {
            return {i, std::nullopt};
        }
        if (copies > best)
        {
            best = copies;
            start = i;
        }
    }
    return {start, best};
}

void verify(std::string_view period, std::string_view pattern, std::size_t start, std::optional<std::size_t> copies)
{
    const auto run = sirkit::longest_periodic_pattern_run(period, pattern);
    assert(run.start == start && run.repetitions == copies);
}
}

int main()
{
    verify("", "", 0, std::nullopt);
    verify("abc", "", 0, std::nullopt);
    verify("", "a", 0, 0);
    verify("abcabab", "ab", 3, 3);
    verify("aabaaa", "a", 3, 5);
    verify("abca", "aa", 3, 1);
    verify("aba", "baaab", 0, 0);
    verify("aa", "aaaaaaa", 0, std::nullopt);
    verify("ab", "ba", 1, std::nullopt);
    verify(std::string_view{"a\0", 2}, std::string_view{"\0a", 2}, 1, std::nullopt);
    verify(std::string_view{"a\0", 2}, std::string_view{"\0\0", 2}, 0, 0);

    for (std::size_t n = 1; n <= 7; ++n)
    {
        for (unsigned encoded_period = 0; encoded_period < (1U << n); ++encoded_period)
        {
            const auto period = binary_word(n, encoded_period);
            for (std::size_t m = 1; m <= 6; ++m)
            {
                for (unsigned encoded_pattern = 0; encoded_pattern < (1U << m); ++encoded_pattern)
                {
                    const auto pattern = binary_word(m, encoded_pattern);
                    const auto expected = brute(period, pattern);
                    verify(period, pattern, expected.start, expected.repetitions);
                }
            }
        }
    }
}
