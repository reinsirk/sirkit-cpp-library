#include "../../../String/Pattern_Stamping.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
std::string binary_word(std::size_t n, unsigned bits)
{
    std::string result(n, 'A');
    for (std::size_t i = 0; i < n; ++i)
    {
        if (bits & (1U << i))
        {
            result[i] = 'B';
        }
    }
    return result;
}

std::unordered_set<std::string> forward_states(std::size_t n, std::string_view stamp)
{
    const std::string root(n, '#');
    std::unordered_set<std::string> seen{root};
    std::vector<std::string> states{root};
    for (std::size_t head = 0; head < states.size(); ++head)
    {
        const std::string state = states[head];
        for (std::size_t left = 0; left + stamp.size() <= n; ++left)
        {
            std::string next = state;
            next.replace(left, stamp.size(), stamp);
            if (seen.insert(next).second)
            {
                states.push_back(std::move(next));
            }
        }
    }
    return seen;
}

void verify(std::string_view target, std::string_view stamp, bool possible)
{
    const auto order = sirkit::pattern_stamping_order(target, stamp);
    assert(order.has_value() == possible);
    if (!order)
    {
        return;
    }
    assert(order->size() <= target.size());
    std::string built(target.size(), '\0');
    std::vector<bool> written(target.size());
    for (const auto left : *order)
    {
        assert(left <= target.size() && stamp.size() <= target.size() - left);
        built.replace(left, stamp.size(), stamp);
        for (std::size_t i = 0; i < stamp.size(); ++i)
        {
            written[left + i] = true;
        }
    }
    assert(built == target);
    assert(std::ranges::all_of(written, [](bool value) { return value; }));
}
}

int main()
{
    verify("", "", true);
    verify("", "A", true);
    verify("A", "", false);
    verify("A", "AA", false);
    verify("A", "A", true);
    verify("A", "B", false);
    verify("ABCBABC", "ABC", true);
    verify("ABBCABC", "ABC", false);
    verify("AAAAAA", "AAA", true);
    verify(std::string_view{"##\0", 3}, std::string_view{"#\0", 2}, true);
    verify(std::string_view{"\0#", 2}, std::string_view{"#\0", 2}, false);

    for (std::size_t n = 1; n <= 8; ++n)
    {
        for (std::size_t m = 1; m <= std::min(n, std::size_t{4}); ++m)
        {
            for (unsigned pattern = 0; pattern < (1U << m); ++pattern)
            {
                const auto stamp = binary_word(m, pattern);
                const auto reachable = forward_states(n, stamp);
                for (unsigned word = 0; word < (1U << n); ++word)
                {
                    const auto target = binary_word(n, word);
                    verify(target, stamp, reachable.contains(target));
                }
            }
        }
    }
}
