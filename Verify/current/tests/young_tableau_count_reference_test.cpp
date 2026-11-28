// SPDX-License-Identifier: CC0-1.0
#include "../../../Math/Young_Tableau_Count.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

namespace
{
using Shape = std::vector<std::uint32_t>;
using Cache = std::map<Shape, std::uint64_t>;

std::uint64_t RemoveBoxes(const Shape &shape, bool domino, Cache &cache)
{
    if (shape.empty() || shape.front() == 0)
    {
        return 1;
    }
    if (const auto known = cache.find(shape); known != cache.end())
    {
        return known->second;
    }
    std::uint64_t ways = 0;
    const auto horizontal = domino ? 2u : 1u;
    for (std::size_t i = 0; i < shape.size(); ++i)
    {
        const auto below = i + 1 < shape.size() ? shape[i + 1] : 0;
        if (shape[i] - below >= horizontal)
        {
            auto next = shape;
            next[i] -= horizontal;
            ways += RemoveBoxes(next, domino, cache);
        }
        if (domino && i + 1 < shape.size() && below != 0 && shape[i] == below)
        {
            const auto after = i + 2 < shape.size() ? shape[i + 2] : 0;
            if (below > after)
            {
                auto next = shape;
                --next[i];
                --next[i + 1];
                ways += RemoveBoxes(next, domino, cache);
            }
        }
    }
    cache.emplace(shape, ways);
    return ways;
}

template <std::uint32_t Mod>
void CheckMod(const Shape &shape, std::uint64_t ordinary, std::uint64_t domino)
{
    assert(sirkit::standard_young_tableau_count<Mod>(shape) == ordinary % Mod);
    assert(sirkit::domino_tableau_count<Mod>(shape) == domino % Mod);
}

void CheckPartition(Shape &shape, std::uint32_t cells, std::uint32_t maximum, Cache &ordinary, Cache &domino)
{
    if (cells == 0)
    {
        const auto count = RemoveBoxes(shape, false, ordinary);
        const auto tilings = RemoveBoxes(shape, true, domino);
        CheckMod<998244353>(shape, count, tilings);
        CheckMod<1>(shape, count, tilings);
        CheckMod<2>(shape, count, tilings);
        CheckMod<7>(shape, count, tilings);
        CheckMod<12>(shape, count, tilings);
        CheckMod<30>(shape, count, tilings);
        CheckMod<4294967295>(shape, count, tilings);
        return;
    }
    for (auto part = std::min(cells, maximum); part != 0; --part)
    {
        shape.push_back(part);
        CheckPartition(shape, cells - part, part, ordinary, domino);
        shape.pop_back();
    }
}

template <class Error, class Call>
void Reject(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const Error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    Cache ordinary, domino;
    Shape shape;
    for (std::uint32_t cells = 0; cells <= 19; ++cells)
    {
        CheckPartition(shape, cells, cells, ordinary, domino);
    }
    CheckMod<998244353>(Shape{3, 3, 0, 0}, 5, 3);
    assert(sirkit::domino_tableau_count(Shape{1000, 1000, 1000, 1000}) == 55019937);
    assert(sirkit::domino_tableau_count(Shape{3, 2, 1}) == 0);
    Reject<std::invalid_argument>([]
                                  {
                                      (void)sirkit::standard_young_tableau_count(Shape{2, 3});
                                  });
    Reject<std::length_error>([]
                              {
                                  (void)sirkit::domino_tableau_count(Shape{std::numeric_limits<std::uint32_t>::max(), 1});
                              });
}
