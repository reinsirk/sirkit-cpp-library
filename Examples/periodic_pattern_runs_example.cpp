#include "../String/Periodic_Pattern_Runs.cpp"

#include <cassert>

int main()
{
    const auto finite = sirkit::longest_periodic_pattern_run("abcabab", "ab");
    assert(finite.start == 3 && finite.repetitions == 3);
    const auto unbounded = sirkit::longest_periodic_pattern_run("abab", "ab");
    assert(unbounded.start == 0 && !unbounded.repetitions);
}
