// SPDX-License-Identifier: CC0-1.0
#include "../String/Wildcard_Pattern_Matching.cpp"
#include <cassert>
#include <vector>

int main()
{
    const auto matches = sirkit::wildcard_pattern_matching("ab*c", "a*");
    assert((matches == std::vector<bool>{true, false, true}));
}
