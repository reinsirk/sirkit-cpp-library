#include "../String/Pattern_Range_Occurrences.cpp"

#include <cassert>

int main()
{
    const sirkit::PatternRangeOccurrences occurrence("ababa", "aba");
    assert(occurrence.count(0, 5) == 2);
    const auto starts = occurrence.positions(1, 5);
    assert(starts.size() == 1 && starts[0] == 2);
}
