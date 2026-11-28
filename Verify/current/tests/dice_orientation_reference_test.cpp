// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Dice_Orientation.cpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <vector>

namespace
{
using Faces = std::array<int, 6>;
using sirkit::DiceFace;
using sirkit::DiceOrientation;
using sirkit::DiceRoll;
constexpr Faces kIdentity{0, 1, 2, 3, 4, 5};
constexpr std::array kDirections{DiceRoll::North, DiceRoll::South, DiceRoll::East, DiceRoll::West};

Faces rotated(const Faces &old, std::size_t direction)
{
    constexpr std::array<Faces, 4> shifts{{{3, 2, 0, 1, 4, 5}, {2, 3, 1, 0, 4, 5}, {5, 4, 2, 3, 0, 1}, {4, 5, 2, 3, 1, 0}}};
    const auto &map = shifts[direction];
    return {old[map[0]], old[map[1]], old[map[2]], old[map[3]], old[map[4]], old[map[5]]};
}

std::set<Faces> enumerate(const Faces &initial)
{
    std::set<Faces> visited{initial};
    std::vector<Faces> queue{initial};
    for (std::size_t i = 0; i < queue.size(); ++i)
    {
        for (std::size_t d = 0; d < kDirections.size(); ++d)
        {
            auto new_faces = rotated(queue[i], d);
            if (visited.insert(new_faces).second)
            {
                queue.push_back(new_faces);
            }
        }
    }
    return visited;
}
}

int main()
{
    constexpr auto states = DiceOrientation::all();
    static_assert(states.size() == 24);
    static_assert(states[0] == DiceOrientation{});
    static_assert(DiceOrientation{}.rolled(DiceRoll::North).original_face(DiceFace::Top) == DiceFace::South);
    const auto expected = enumerate(kIdentity);
    std::set<Faces> actual;
    for (std::size_t i = 0; i < states.size(); ++i)
    {
        const auto orientation = states[i];
        const auto faces = orientation.apply(kIdentity);
        const auto by_faces = DiceOrientation::from_faces(static_cast<DiceFace>(faces[0]), static_cast<DiceFace>(faces[2]));
        if (orientation.id() != i || by_faces != orientation || !actual.insert(faces).second)
        {
            return 1;
        }
        for (std::size_t dir = 0; dir < kDirections.size(); ++dir)
        {
            if (orientation.rolled(kDirections[dir]).apply(kIdentity) != rotated(faces, dir))
            {
                return 2;
            }
        }
    }
    if (expected != actual || DiceOrientation::from_faces(DiceFace::Top, DiceFace::Bottom) ||
        DiceOrientation::from_faces(DiceFace::East, DiceFace::East))
    {
        return 3;
    }
    auto mirror = kIdentity;
    std::swap(mirror[4], mirror[5]);
    if (sirkit::canonical_dice_faces(kIdentity) == sirkit::canonical_dice_faces(mirror))
    {
        return 4;
    }
    for (int labels = 0; labels < 729; ++labels)
    {
        auto digits = labels;
        Faces face_labels{};
        for (auto &face : face_labels)
        {
            face = digits % 3;
            digits /= 3;
        }
        const auto expected_labels = enumerate(face_labels);
        if (sirkit::canonical_dice_faces(face_labels) != *expected_labels.begin())
        {
            return 5;
        }
    }
}
