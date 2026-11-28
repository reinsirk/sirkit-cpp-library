#include "../String/Three_Symbol_Frequency_Counts.cpp"

#include <cassert>

int main()
{
    const auto result = sirkit::analyze_three_symbol_frequencies("AABBCC");
    assert(result.all_distinct == 4);
    assert(result.all_equal == 1);
}
