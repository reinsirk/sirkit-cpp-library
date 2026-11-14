#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/enumerate_palindromes

// Library: String/Manacher.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Manacher.cpp
// 参考（回文半径の出力形式とVerify先。実装は独自構成）:
// https://judge.yosupo.jp/problem/enumerate_palindromes
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/manacher.md
//
// API一覧
// PalindromeRadii(s)/assign(s): 列の奇数/偶数長回文の半径をO(n)時間・領域で構築。
// odd[i]: 中心iの奇数回文の半径（中心を含む）。even[i]: i-1とiの間の偶数回文の半分の長さ。
// size()/empty(): 元の列長/空かを返す。is_palindrome(l,r): 半開区間[l,r)をO(1)で判定、空区間はtrue。
// longest(): 最長回文の半開区間を返す。同長なら最小の左端、空なら{0,0}。O(n)。
// count(): 非空の連続回文区間の総数をuint64_tで返す。O(n)。
// PalindromicLengths(s): 文字上/文字間の2n-1中心の最長回文長を順に返す。空なら{}、O(n)。
// size()/添字アクセス/同値関係の==が使える列が前提。各比較O(1)、列長はINT_MAX未満。元の列は保持しない。
// 検査省略時もis_palindromeの添字は0<=l<=r<=size()が必要。公開半径配列は変更しないこと。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROME_RADII_DEFINED
#define SIRKIT_PALINDROME_RADII_DEFINED
class PalindromeRadii {
public:
    std::vector<int> odd, even;
    PalindromeRadii() = default;
    template <class Sequence>
    explicit PalindromeRadii(const Sequence& s) { assign(s); }

    template <class Sequence>
    void assign(const Sequence& s) {
        if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("PalindromeRadii: sequence too long");
        const int n = static_cast<int>(s.size());
        std::vector<int> next_odd(s.size()), next_even(s.size());
        int left = 0, right = -1;
        for (int i = 0; i < n; ++i) {
            int radius = i <= right ? std::min(right - i + 1, next_odd[left + (right - i)]) : 1;
            while (radius <= i && radius < n - i && s[i - radius] == s[i + radius]) ++radius;
            next_odd[i] = radius;
            if (i + radius - 1 > right) { left = i - radius + 1; right = i + radius - 1; }
        }
        left = 0; right = -1;
        for (int i = 0; i < n; ++i) {
            int radius = i <= right ? std::min(right - i + 1, next_even[left + (right - i) + 1]) : 0;
            while (radius < i && radius < n - i && s[i - radius - 1] == s[i + radius]) ++radius;
            next_even[i] = radius;
            if (i + radius - 1 > right) { left = i - radius; right = i + radius - 1; }
        }
        odd.swap(next_odd); even.swap(next_even);
    }

    [[nodiscard]] int size() const noexcept { return static_cast<int>(odd.size()); }
    [[nodiscard]] bool empty() const noexcept { return odd.empty(); }
    [[nodiscard]] bool is_palindrome(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("PalindromeRadii: range");
#endif
        const int length = r - l;
        if (length == 0) return true;
        const int center = l + length / 2;
        return length & 1 ? odd[center] >= length / 2 + 1 : even[center] >= length / 2;
    }
    [[nodiscard]] std::pair<int, int> longest() const noexcept {
        std::pair<int, int> best{0, 0};
        const auto consider = [&](int l, int r) {
            if (r - l > best.second - best.first ||
                (r - l == best.second - best.first && l < best.first)) best = {l, r};
        };
        for (int i = 0; i < size(); ++i) {
            consider(i - odd[i] + 1, i + odd[i]);
            consider(i - even[i], i + even[i]);
        }
        return best;
    }
    [[nodiscard]] std::uint64_t count() const noexcept {
        std::uint64_t result = 0;
        for (int radius : odd) result += static_cast<std::uint64_t>(radius);
        for (int radius : even) result += static_cast<std::uint64_t>(radius);
        return result;
    }
};
#endif

#ifndef SIRKIT_PALINDROMIC_LENGTHS_DEFINED
#define SIRKIT_PALINDROMIC_LENGTHS_DEFINED
template <class Sequence>
std::vector<int> PalindromicLengths(const Sequence& s) {
    if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("PalindromicLengths: sequence too long");
    const int n = static_cast<int>(s.size());
    if (n == 0) return {};
    std::vector<int> result;
    const auto count = static_cast<std::size_t>(n);
    const auto capacity = result.max_size();
    if (count > capacity / 2 + capacity % 2)
        throw std::length_error("PalindromicLengths: result too long");
    result.resize(count + (count - 1));
    // n<INT_MAXより、2n-1はintと同幅の符号なし型に収まる。
    using Center = unsigned int;
    // centerは文字上と文字間を交互に表す。rightは既知回文の右端の2倍。
    Center middle = 0, right = 0, start = 0;
    const auto centers = static_cast<Center>(result.size());
    // 先頭の同値区間内では回文長が三角列になる。
    if (n > 1 && s[0] == s[1]) {
        int run = 2;
        while (run < n && s[0] == s[run]) ++run;
        const auto prefix_centers = Center{2} * static_cast<Center>(run) - 1;
        for (Center c = 0; c < static_cast<Center>(run); ++c) result[c] = static_cast<int>(c + 1);
        for (Center c = static_cast<Center>(run); c < prefix_centers; ++c) result[c] = static_cast<int>(prefix_centers - c);
        if (run == n) return result;
        middle = static_cast<Center>(run - 1);
        right = prefix_centers - 1;
        // 最初の不一致をまたぐ文字間の中心は0（零初期化済み）。
        start = prefix_centers + 1;
    }
    for (Center center = start; center < centers; ++center) {
        int length = (center & 1) == 0 ? 1 : 0;
        if (center < right) {
            const int available = static_cast<int>(right - center + 1);
            const int reflected = result[middle - (center - middle)];
            // 右端に届かない鏡像は、不一致まで確定している。
            if (reflected < available) { result[center] = reflected; continue; }
            length = available;
        }
        int l = static_cast<int>((center + 1 - static_cast<Center>(length)) / 2) - 1;
        int r = static_cast<int>((center + static_cast<Center>(length) + 1) / 2);
        while (l >= 0 && r < n && s[l] == s[r]) { --l; ++r; length += 2; }
        result[center] = length;
        const auto end = center + static_cast<Center>(length) - 1;
        if (end > right) { middle = center; right = end; }
    }
    return result;
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

#include <string>
int main() {
    std::string s; verify_in >> s;
    const auto lengths = PalindromicLengths(s);
    if (!lengths.empty()) verify_out << lengths[0];
    for (std::size_t i = 1; i < lengths.size(); ++i) verify_out << ' ' << lengths[i];
    verify_out << '\n';
}
