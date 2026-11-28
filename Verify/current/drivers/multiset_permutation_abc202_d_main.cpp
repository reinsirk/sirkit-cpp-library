// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

int main()
{
    std::size_t a{}, b{};
    std::int64_t position{};
    verify_in >> a >> b >> position;
    std::vector<char> word(a, 'a');
    word.insert(word.end(), b, 'b');
    const sirkit::MultisetPermutation order(std::move(word));
    const auto result = order.unrank(position - 1).value();
    verify_out << std::string_view(result.data(), result.size()) << '\n';
}
