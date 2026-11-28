// SPDX-License-Identifier: CC0-1.0
#include "../String/Concatenated_Pattern_Count.cpp"
#include <cassert>

int main()
{
    const sirkit::ConcatenatedPatternCounter counter("aba");
    const auto left = counter.summarize("ab");
    const auto right = counter.summarize("aba");
    const auto joined = counter.concatenate(left, right);
    assert(counter.count(joined) == 2);
}
