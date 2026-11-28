#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/tdpc/tasks/tdpc_grid

// Library: DataStructure/Grid_Connectivity_Frontier.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Grid_Connectivity_Frontier.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/grid_connectivity_frontier.md
//
// API一覧
// GridConnectivityFrontier(h): 1<=h<=14の盤面で、列ごとに上から走査する境界を扱う。
// State{}: まだ黒マスのない境界。Stateは同じ高さで生成した値を使う。
// place(state, row, black, mark=false): 1マス処理して{state,closed_group}。消えた成分がなければclosed_group=0。
// label(state,row): 白は0、黒は連結成分の番号。印のある成分はkMarkedGroup。O(1)。
// 黒マス1個に印を付けると、併合で印を引き継ぐ。placeはO(h)時間・領域。
// grid_corner_connected_colorings(h,w,mod): 左上と右下が黒で上下左右に連結な塗り方の数をmodで返す。
// H=min(h,w), W=max(h,w)、途中の境界状態数の最大をSとすると期待O(H^2 W S)時間、O(S)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRID_CONNECTIVITY_FRONTIER_DEFINED
#define SIRKIT_GRID_CONNECTIVITY_FRONTIER_DEFINED

class GridConnectivityFrontier
{
public:
    using State = std::uint64_t;
    static constexpr int kMaxHeight = 14;
    static constexpr int kMarkedGroup = 15;

    struct Transition
    {
        State state;
        int closed_group;
    };

    explicit GridConnectivityFrontier(int height)
        : height_(height)
    {
    }

    [[nodiscard]] int label(State state, int row) const
    {
        CheckRow(row);
        return Decode(state, row);
    }

    [[nodiscard]] Transition place(State state, int row, bool black, bool mark = false) const
    {
        CheckRow(row);
        std::array<int, kMaxHeight> groups{};
        const auto end = groups.begin() + height_;
        for (int i = 0; i < height_; ++i)
        {
            groups[i] = Decode(state, i);
        }
        const int above = row == 0 ? 0 : groups[row - 1];
        const int previous = groups[row];
        int closed = 0;
        if (black)
        {
            const int group = mark || previous == kMarkedGroup || above == kMarkedGroup
                                  ? kMarkedGroup
                              : previous != 0 ? previous
                              : above != 0    ? above
                                              : row + 1;
            for (auto it = groups.begin(); it != end; ++it)
            {
                if (*it != 0 && (*it == previous || *it == above))
                {
                    *it = group;
                }
            }
            groups[row] = group;
        }
        else
        {
            groups[row] = 0;
            if (previous != 0 && std::find(groups.begin(), end, previous) == end)
            {
                closed = previous;
            }
        }

        // 普通の成分番号は境界に現れる最初の行から決める。
        std::array<int, kMarkedGroup + 1> canonical{};
        canonical[kMarkedGroup] = kMarkedGroup;
        State next = 0;
        for (int i = 0; i < height_; ++i)
        {
            const int group = groups[i];
            if (group == 0)
            {
                continue;
            }
            if (canonical[group] == 0)
            {
                canonical[group] = i + 1;
            }
            next |= static_cast<State>(canonical[group]) << (4 * i);
        }
        return {next, closed};
    }

private:
    int height_;

    [[nodiscard]] static int Decode(State state, int row) noexcept
    {
        return static_cast<int>((state >> (4 * row)) & 15U);
    }

    void CheckRow([[maybe_unused]] int row) const
    {
    }
};

[[nodiscard]] inline std::uint32_t grid_corner_connected_colorings(int height, int width,
                                                                   std::uint32_t modulus = 1000000007)
{
    const int smaller = std::min(height, width), larger = std::max(height, width);
    const GridConnectivityFrontier grid(smaller);
    std::unordered_map<GridConnectivityFrontier::State, std::uint32_t> current{{0, 1 % modulus}}, next;
    for (int column = 0; column < larger; ++column)
    {
        for (int row = 0; row < smaller; ++row)
        {
            const bool first = column == 0 && row == 0;
            const bool last = column + 1 == larger && row + 1 == smaller;
            next.clear();
            const auto add = [&](GridConnectivityFrontier::State state, std::uint32_t count)
            {
                auto &value = next[state];
                const std::uint64_t sum = static_cast<std::uint64_t>(value) + count;
                value = static_cast<std::uint32_t>(sum >= modulus ? sum - modulus : sum);
            };
            for (const auto &[state, count] : current)
            {
                const auto black = grid.place(state, row, true, first);
                if (!last || grid.label(black.state, row) == GridConnectivityFrontier::kMarkedGroup)
                {
                    add(black.state, count);
                }
                if (!first && !last)
                {
                    const auto white = grid.place(state, row, false);
                    if (white.closed_group != GridConnectivityFrontier::kMarkedGroup)
                    {
                        add(white.state, count);
                    }
                }
            }
            current.swap(next);
        }
    }
    std::uint64_t answer = 0;
    for (const auto &[state, count] : current)
    {
        answer += count;
        if (answer >= modulus)
        {
            answer -= modulus;
        }
    }
    return static_cast<std::uint32_t>(answer);
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


int main()
{
    int h = 0, w = 0;
    verify_in >> h >> w;
    verify_out << grid_corner_connected_colorings(h, w) << '\n';
}
