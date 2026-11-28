#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://onlinejudge.u-aizu.ac.jp/problems/ITP1_11_C

// Library: Geometry/Dice_Orientation.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
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


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif


#include <array>

int main()
{
    using Faces = std::array<int, 6>;
    const auto read = []
    {
        int top{}, south{}, east{}, west{}, north{}, bottom{};
        verify_in >> top >> south >> east >> west >> north >> bottom;
        return Faces{top, bottom, north, south, east, west};
    };
    const auto first = sirkit::canonical_dice_faces(read());
    const auto second = sirkit::canonical_dice_faces(read());
    verify_out << (first == second ? "Yes\n" : "No\n");
}
