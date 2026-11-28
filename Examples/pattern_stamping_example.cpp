#include "../String/Pattern_Stamping.cpp"

#include <cassert>
#include <string>
#include <string_view>

int main()
{
    constexpr std::string_view target = "ABAAB";
    constexpr std::string_view stamp = "AB";
    const auto order = sirkit::pattern_stamping_order(target, stamp);
    assert(order);
    std::string result(target.size(), ' ');
    for (const auto position : *order)
    {
        result.replace(position, stamp.size(), stamp);
    }
    assert(result == target);
}
