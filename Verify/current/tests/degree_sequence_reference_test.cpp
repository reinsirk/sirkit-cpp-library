// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Degree_Sequence.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Degrees = std::vector<int>;

std::set<Degrees> AllGraphical(int n)
{
    std::vector<std::pair<int, int>> allowed;
    for (int u = 0; u < n; ++u)
    {
        for (int v = u + 1; v < n; ++v)
        {
            allowed.emplace_back(u, v);
        }
    }
    std::set<Degrees> possible;
    const auto graphs = std::uint32_t{1} << allowed.size();
    for (std::uint32_t mask = 0; mask < graphs; ++mask)
    {
        Degrees degrees(static_cast<std::size_t>(n));
        for (std::size_t edge = 0; edge < allowed.size(); ++edge)
        {
            if (((mask >> edge) & 1u) != 0)
            {
                const auto [u, v] = allowed[edge];
                ++degrees[static_cast<std::size_t>(u)];
                ++degrees[static_cast<std::size_t>(v)];
            }
        }
        std::sort(degrees.begin(), degrees.end());
        possible.insert(degrees);
    }
    return possible;
}

void CheckWitness(const Degrees &degrees, bool possible)
{
    assert(sirkit::is_graphical_degree_sequence(degrees) == possible);
    const auto graph = sirkit::realize_degree_sequence(degrees);
    assert(graph.has_value() == possible);
    if (!graph)
    {
        return;
    }
    Degrees seen(degrees.size());
    std::set<std::pair<int, int>> used;
    for (const auto &[u, v] : graph.value())
    {
        assert(0 <= u && 0 <= v && u != v);
        assert(static_cast<std::size_t>(u) < seen.size() && static_cast<std::size_t>(v) < seen.size());
        assert(used.emplace(std::min(u, v), std::max(u, v)).second);
        ++seen[static_cast<std::size_t>(u)];
        ++seen[static_cast<std::size_t>(v)];
    }
    assert(seen == degrees);
}

void CheckAllDegrees(Degrees &degrees, std::size_t count, int minimum, const std::set<Degrees> &possible)
{
    if (count == degrees.size())
    {
        CheckWitness(degrees, possible.contains(degrees));
        return;
    }
    for (auto next = minimum; next < static_cast<int>(degrees.size()); ++next)
    {
        degrees[count] = next;
        CheckAllDegrees(degrees, count + 1, next, possible);
    }
}

template <class Call>
void CheckLengthError(Call call)
{
    bool rejected = false;
    try
    {
        call();
    }
    catch (const std::length_error &)
    {
        rejected = true;
    }
    assert(rejected);
}
} // namespace

int main()
{
    for (int n = 0; n <= 6; ++n)
    {
        const auto possible = AllGraphical(n);
        Degrees degrees(static_cast<std::size_t>(n));
        CheckAllDegrees(degrees, 0, 0, possible);
    }

    CheckWitness({-1, 2, 0}, false);
    CheckWitness({3, 0, 1}, false);
    CheckWitness({0, 0, 1}, false);
    CheckWitness({0, 1, 3, 3}, false);

    std::mt19937 random(1519);
    for (int trial = 0; trial < 100; ++trial)
    {
        Degrees degrees(1 + random() % 100u);
        for (std::size_t i = 0; i < degrees.size(); ++i)
        {
            for (std::size_t j = 0; j < i; ++j)
            {
                if (random() % 3u == 0)
                {
                    ++degrees[i];
                    ++degrees[j];
                }
            }
        }
        CheckWitness(degrees, true);
    }

    CheckLengthError([]
                     {
                         (void)sirkit::detail::DegreeSequenceVertexCount(static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) + 1);
                     });
    CheckLengthError([]
                     {
                         (void)sirkit::detail::DegreeSequenceEdgeCapacity(std::numeric_limits<std::uint64_t>::max());
                     });
}
