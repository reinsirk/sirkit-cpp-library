// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    using Chord = std::pair<std::int64_t, std::int64_t>;
    std::size_t circle_size{}, chord_count{}, query_count{};
    verify_in >> circle_size >> chord_count;
    std::vector<Chord> chords(chord_count);
    for (auto &[a, b] : chords)
    {
        verify_in >> a >> b;
    }
    verify_in >> query_count;
    std::vector<Chord> queries(query_count);
    for (auto &[a, b] : queries)
    {
        verify_in >> a >> b;
    }
    for (const auto count : sirkit::count_chord_crossings(chords, queries))
    {
        verify_out << count << '\n';
    }
}
