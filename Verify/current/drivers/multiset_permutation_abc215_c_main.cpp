// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

int main()
{
    std::string word;
    std::int64_t position{};
    verify_in >> word >> position;
    const sirkit::MultisetPermutation order(std::vector(word.begin(), word.end()));
    const auto result = order.unrank(position - 1).value();
    verify_out << std::string_view(result.data(), result.size()) << '\n';
}
