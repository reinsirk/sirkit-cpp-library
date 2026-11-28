// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <string>
#include <vector>

int main()
{
    std::size_t count;
    std::string pattern;
    verify_in >> count >> pattern;
    const sirkit::ConcatenatedPatternCounter counter(pattern);
    std::vector<sirkit::ConcatenatedPatternCounter::Summary> summaries;
    summaries.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        std::string text;
        verify_in >> text;
        if (text == "~")
        {
            std::size_t left, right;
            verify_in >> left >> right;
            summaries.push_back(counter.concatenate(summaries[left - 1], summaries[right - 1]));
        }
        else
        {
            summaries.push_back(counter.summarize(text));
        }
    }
    verify_out << counter.count(summaries.back()) << '\n';
}
