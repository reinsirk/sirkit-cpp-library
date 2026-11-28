#include "../Algorithm/Inversion_Limited_Partitions.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::int64_t, 4> values{3, 1, 4, 2};
    const auto starts = sirkit::inversion_limited_starts(values, 0);
    assert((starts == std::vector<std::size_t>{0, 1, 1, 3}));
    assert(sirkit::count_inversion_limited_partitions(values, 0) == 2);
}
