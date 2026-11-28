// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <string>

int main()
{
    std::string text, pattern;
    verify_in >> text >> pattern;
    const auto distances = sirkit::hamming_distances(text, pattern);
    verify_out << *std::min_element(distances.begin(), distances.end()) << '\n';
}
