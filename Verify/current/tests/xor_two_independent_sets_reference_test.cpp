// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Xor_Two_Independent_Sets.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <ranges>
#include <span>
#include <vector>

static std::size_t exhaustive(std::span<const std::uint64_t> values)
{
    std::size_t best = 0;
    std::array<std::vector<std::uint64_t>, 2> xor_values{{{0}, {0}}};
    const auto dfs = [&](const auto &self, std::size_t index, std::size_t used) -> void
    {
        if (used + (values.size() - index) <= best)
        {
            return;
        }
        if (index == values.size())
        {
            best = std::max(best, used);
            return;
        }
        for (auto &group : xor_values)
        {
            if (std::ranges::find(group, values[index]) == group.end())
            {
                const auto old_size = group.size();
                for (std::size_t i = 0; i < old_size; ++i)
                {
                    group.push_back(group[i] ^ values[index]);
                }
                self(self, index + 1, used + 1);
                group.resize(old_size);
            }
        }
        self(self, index + 1, used);
    };
    dfs(dfs, 0, 0);
    return best;
}

static void check(std::span<const std::uint64_t> values)
{
    sirkit::XorTwoIndependentSets model;
    assert(model.rank() == 0);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        const auto before = model.rank();
        const bool inserted = model.insert(values[i]);
        const auto count = exhaustive(values.first(i + 1));
        assert(model.rank() == count && inserted == (before < count));
        const auto groups = model.groups();
        assert(groups[0].size() + groups[1].size() == count);
        std::vector<bool> seen(i + 1);
        for (const auto &group : groups)
        {
            assert(std::ranges::is_sorted(group));
            std::vector<std::uint64_t> span{0};
            for (auto id : group)
            {
                assert(id <= i && !seen[id]);
                seen[id] = true;
                assert(std::ranges::find(span, values[id]) == span.end());
                const auto prior_size = span.size();
                for (std::size_t j = 0; j < prior_size; ++j)
                {
                    span.push_back(span[j] ^ values[id]);
                }
            }
        }
    }
}

int main()
{
    check({});
    check(std::array<std::uint64_t, 9>{1, 2, 3, 4, 5, 7, 3, 5, 7});
    check(std::array<std::uint64_t, 8>{1, 1, 1, 1, 2, 2, 2, 0});
    check(std::array<std::uint64_t, 6>{0, 0, 0, 0, 0, 0});
    std::mt19937_64 rng(329452);
    for (int test = 0; test < 530; ++test)
    {
        const auto value_mask = (std::uint64_t{1} << (1 + rng() % 6)) - 1;
        const auto shift = (rng() % 3 == 0 ? 57 : 0);
        std::vector<std::uint64_t> values(rng() % 11);
        for (auto &value : values)
        {
            value = (rng() & value_mask) << shift;
        }
        check(values);
    }
    sirkit::XorTwoIndependentSets maximum;
    for (unsigned bit = 0; bit < 64; ++bit)
    {
        const auto power = std::uint64_t{1} << bit;
        assert(maximum.insert(power));
        assert(maximum.insert(power));
        assert(!maximum.insert(power));
    }
    assert(maximum.rank() == 128);
    const auto groups = maximum.groups();
    assert(groups[0].size() == 64 && groups[1].size() == 64);
    for (unsigned bit = 0; bit < 64; ++bit)
    {
        assert((groups[0][bit] / 3 == bit) && (groups[1][bit] / 3 == bit));
    }
}
