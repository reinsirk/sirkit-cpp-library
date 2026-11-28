#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/621

// Library: Algorithm/Narrow_Grid_Domino_Packings.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/narrow_grid_domino_packings.md
//
// API一覧
// narrow_grid_domino_packings(h,w,mode,mod): h行w列のドミノ配置の数をmodで返す。0<=h<=5、wはuint64_t、mod>0。
// DominoPackingMode::kPerfectは全マスを覆う。kMaximalはドミノを追加できない配置を数える。
// 空の盤面には配置が1通りある。縦・横のドミノは各々区別せず、回転して置ける。
// 到達可能な境界状態数をSとすると、O(S*3^h + S^3 log(w+1))時間、O(S^2 + 4^h)領域。
// Sは完全配置で2^h以下、極大配置で3^h以下。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NARROW_GRID_DOMINO_PACKINGS_DEFINED
#define SIRKIT_NARROW_GRID_DOMINO_PACKINGS_DEFINED

enum class DominoPackingMode
{
    kPerfect,
    kMaximal
};

namespace detail
{
inline std::vector<std::uint32_t> MultiplyDominoTransfers(const std::vector<std::uint32_t> &left, int rows,
                                                          const std::vector<std::uint32_t> &right, int order,
                                                          std::uint32_t modulus)
{
    std::vector<std::uint32_t> answer(rows * order);
    for (int i = 0; i < rows; ++i)
    {
        for (int k = 0; k < order; ++k)
        {
            if (left[i * order + k] == 0)
            {
                continue;
            }
            const std::uint64_t a = left[i * order + k];
            for (int j = 0; j < order; ++j)
            {
                if (right[k * order + j] != 0)
                {
                    auto &cell = answer[i * order + j];
                    cell = static_cast<std::uint32_t>((cell + a * right[k * order + j]) % modulus);
                }
            }
        }
    }
    return answer;
}
}

[[nodiscard]] inline std::uint32_t narrow_grid_domino_packings(int height, std::uint64_t width,
                                                               DominoPackingMode mode,
                                                               std::uint32_t modulus = 1000000007)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (height < 0 || height > 5 || modulus == 0)
    {
        throw std::invalid_argument("narrow_grid_domino_packings: invalid height or modulus");
    }
#endif
    if (height == 0 || width == 0)
    {
        return 1 % modulus;
    }
    const unsigned row_mask = (1U << height) - 1;
    std::vector<int> index(1U << (2 * height), -1);
    std::vector<unsigned> states{0};
    index[0] = 0;
    std::vector<std::vector<int>> edges;
    for (std::size_t i = 0; i < states.size(); ++i)
    {
        const unsigned reserved = states[i] & row_mask;
        const unsigned previous_empty = states[i] >> height;
        auto &destinations = edges.emplace_back();
        const auto fill_column = [&](const auto &self, int row, unsigned outgoing, unsigned empty) -> void
        {
            if (row == height)
            {
                const unsigned key = outgoing | (empty << height);
                if (index[key] < 0)
                {
                    index[key] = static_cast<int>(states.size());
                    states.push_back(key);
                }
                destinations.push_back(index[key]);
                return;
            }
            const unsigned bit = 1U << row;
            if ((reserved & bit) != 0)
            {
                self(self, row + 1, outgoing, empty);
                return;
            }
            self(self, row + 1, outgoing | bit, empty);
            if (row + 1 < height && (reserved & (bit << 1)) == 0)
            {
                self(self, row + 2, outgoing, empty);
            }
            // 極大配置では、未被覆マスが上下左右に隣接するとドミノを追加できてしまう。
            if (mode == DominoPackingMode::kMaximal && (previous_empty & bit) == 0 && (empty & (bit >> 1)) == 0)
            {
                self(self, row + 1, outgoing, empty | bit);
            }
        };
        fill_column(fill_column, 0, 0, 0);
    }

    const int order = static_cast<int>(states.size());
    std::vector<std::uint32_t> transfer(order * order), ways(order);
    for (int i = 0; i < order; ++i)
    {
        for (int j : edges[i])
        {
            // 突出と未被覆の位置が決まると、残りの縦ドミノの配置は一意。
            auto &cell = transfer[i * order + j];
            cell = 1 % modulus;
        }
    }
    ways[0] = 1 % modulus;
    while (width != 0)
    {
        if ((width & 1) != 0)
        {
            ways = detail::MultiplyDominoTransfers(ways, 1, transfer, order, modulus);
        }
        width >>= 1;
        if (width != 0)
        {
            transfer = detail::MultiplyDominoTransfers(transfer, order, transfer, order, modulus);
        }
    }
    std::uint64_t answer = 0;
    for (int i = 0; i < order; ++i)
    {
        if ((states[i] & row_mask) == 0)
        {
            answer += ways[i];
        }
    }
    return static_cast<std::uint32_t>(answer % modulus);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
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
    std::uint64_t width = 0;
    verify_in >> width;
    verify_out << narrow_grid_domino_packings(3, width, DominoPackingMode::kMaximal) << '\n';
}
