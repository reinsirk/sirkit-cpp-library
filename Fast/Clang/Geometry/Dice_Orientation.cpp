// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Dice_Orientation.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dice_orientation.md
//
// API一覧
// DiceFace: Top, Bottom, North, South, East, West。ラベルの配列はこの順。
// DiceRoll: North, South, East, West。Northへの移動で旧上面が北面へ移る。
// DiceOrientation(): 初期姿勢。from_faces(top,north): 初期状態のどの面が上・北かで復元。両面が隣り合わなければnullopt。
// rolled(direction): その方向に一度転がした姿勢。original_face(face): 指定方向に来た初期状態の面。
// apply(labels): 現在の上・下・北・南・東・西の面ラベル。ラベルの重複可。
// id(): 0..23の番号。all(): 24姿勢を番号順に返す。==: 姿勢の一致。
// canonical_dice_faces(labels): 24姿勢のラベル列の辞書順最小。回転で一致するサイコロは同じ値になる。
// 全操作O(1)時間・領域。面は6個、姿勢は24通りで固定。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DICE_ORIENTATION
#define SIRKIT_DICE_ORIENTATION

enum class DiceFace : std::uint8_t
{
    Top,
    Bottom,
    North,
    South,
    East,
    West
};

enum class DiceRoll : std::uint8_t
{
    North,
    South,
    East,
    West
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
constexpr std::uint8_t kDiceOrientations = 24;
constexpr std::uint8_t kDiceFaces = 6;
using DiceAxis = std::array<int, 3>;

[[nodiscard]] constexpr std::size_t dice_face_index(DiceFace value)
{
    return static_cast<std::size_t>(value);
}

struct DiceOrientationTables
{
    std::array<std::array<DiceFace, kDiceFaces>, kDiceOrientations> faces{};
    std::array<std::array<std::uint8_t, kDiceFaces>, kDiceFaces> by_top_and_north{};
    std::array<std::array<std::uint8_t, 4>, kDiceOrientations> rolls{};
};

[[nodiscard]] consteval DiceOrientationTables make_dice_orientation_tables()
{
    constexpr std::array<DiceAxis, kDiceFaces> axes{{{0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}}};
    using F = DiceFace;
    constexpr std::array<std::array<F, 2>, 4> after_roll{{{F::South, F::Top}, {F::North, F::Bottom}, {F::West, F::North}, {F::East, F::North}}};
    DiceOrientationTables result;
    for (auto &row : result.by_top_and_north)
    {
        row.fill(kDiceOrientations);
    }
    std::uint8_t id = 0;
    for (std::size_t top = 0; top < kDiceFaces; ++top)
    {
        for (std::size_t north = 0; north < kDiceFaces; ++north)
        {
            if (top / 2 == north / 2)
            {
                continue;
            }
            const auto &a = axes[north];
            const auto &b = axes[top];
            const DiceAxis east_axis{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
            const auto east = std::ranges::find(axes, east_axis) - axes.begin();
            result.by_top_and_north[top][north] = id;
            result.faces[id++] = {static_cast<F>(top), static_cast<F>(top ^ 1), static_cast<F>(north),
                                  static_cast<F>(north ^ 1), static_cast<F>(east), static_cast<F>(east ^ 1)};
        }
    }
    for (std::size_t i = 0; i < result.faces.size(); ++i)
    {
        for (std::size_t direction = 0; direction < after_roll.size(); ++direction)
        {
            const auto top = result.faces[i][dice_face_index(after_roll[direction][0])];
            const auto north = result.faces[i][dice_face_index(after_roll[direction][1])];
            result.rolls[i][direction] = result.by_top_and_north[dice_face_index(top)][dice_face_index(north)];
        }
    }
    return result;
}

constexpr auto kDiceOrientationTables = make_dice_orientation_tables();
static_assert(kDiceOrientationTables.by_top_and_north[0][2] == 0);
} // namespace reinsirk::detail

class DiceOrientation
{
public:
    constexpr DiceOrientation() = default;

    [[nodiscard]] static constexpr std::optional<DiceOrientation> from_faces(DiceFace top, DiceFace north)
    {
        const auto id = detail::kDiceOrientationTables.by_top_and_north[detail::dice_face_index(top)][detail::dice_face_index(north)];
        if (id == detail::kDiceOrientations)
        {
            return std::nullopt;
        }
        return DiceOrientation(id);
    }

    [[nodiscard]] static constexpr std::array<DiceOrientation, detail::kDiceOrientations> all()
    {
        std::array<DiceOrientation, detail::kDiceOrientations> result;
        for (std::uint8_t i = 0; i < detail::kDiceOrientations; ++i)
        {
            result[i] = DiceOrientation(i);
        }
        return result;
    }

    [[nodiscard]] constexpr std::size_t id() const noexcept
    {
        return id_;
    }

    [[nodiscard]] constexpr DiceOrientation rolled(DiceRoll direction) const noexcept
    {
        return DiceOrientation(detail::kDiceOrientationTables.rolls[id_][static_cast<std::size_t>(direction)]);
    }

    [[nodiscard]] constexpr DiceFace original_face(DiceFace side) const noexcept
    {
        return detail::kDiceOrientationTables.faces[id_][detail::dice_face_index(side)];
    }

    template <class T>
    [[nodiscard]] constexpr std::array<T, detail::kDiceFaces> apply(const std::array<T, detail::kDiceFaces> &labels) const
    {
        using F = DiceFace;
        return {labels[detail::dice_face_index(original_face(F::Top))], labels[detail::dice_face_index(original_face(F::Bottom))],
                labels[detail::dice_face_index(original_face(F::North))], labels[detail::dice_face_index(original_face(F::South))],
                labels[detail::dice_face_index(original_face(F::East))], labels[detail::dice_face_index(original_face(F::West))]};
    }

    constexpr bool operator==(const DiceOrientation &) const = default;

private:
    constexpr explicit DiceOrientation(std::uint8_t id) : id_(id)
    {
    }

    std::uint8_t id_ = 0;
};

template <class T>
[[nodiscard]] constexpr std::array<T, detail::kDiceFaces> canonical_dice_faces(const std::array<T, detail::kDiceFaces> &labels)
{
    auto best = labels;
    for (const auto orientation : DiceOrientation::all())
    {
        best = std::min(best, orientation.apply(labels));
    }
    return best;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
