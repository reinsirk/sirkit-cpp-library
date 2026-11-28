#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1848

// Library: String/Run_Length_Z.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: String/Run_Length_Z.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_length_z.md
//
// API一覧
// RunLengthZ::Run{byte,count}: byteがcount回連続する区間。countはuint64_t。
// RunLengthZ(runs): 連長圧縮された入力からO(n+1)時間・領域で構築。0回のrunは飛ばし、隣接同値は結合。
// size(): 展開後の長さ。O(1)。合計長はUINT64_MAX以下、結合後の区間数はINT_MAX以下。
// prefix_at(position): 展開後の0始まりの位置から始まる接尾辞との共通接頭辞長。position==size()なら0。O(log(n+2))。
// sum_prefix_lengths(modulus=1000000007): 全開始位置のZ値の総和を正のuint32_tの法で返す。O(n+1)、照会追加O(1)。
// nは入力区間数。同梱のZ AlgorithmはDocument/z_algorithm.mdを参照。
//
// [同梱のZ Algorithm]
// z_algorithm(const std::string&)/z_algorithm<T>(const std::vector<T>&): ACL形式のZ配列を返す。
// 追加のz_algorithm(sequence)は任意の添字アクセス可能な列に対応。空なら{}、非空ならz[0]=n。
// PrefixMatches(pattern,text): textの各開始位置とpatternの共通接頭辞長を返す。
// ZMatches(pattern,text): 完全一致する開始位置を昇順に返す。空patternは0..text.size()。
// size()/添字アクセスと同値関係の==が使える列が前提。比較O(1)ならZはO(n)、後2者はO(n+m)時間。
// 各列長はINT_MAX未満。Z/PrefixMatchesの領域は返却値とpattern長、ZMatchesはこれに一致数。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>
#include <span>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_Z_ALGORITHM_IMPL_DEFINED
#define SIRKIT_Z_ALGORITHM_IMPL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class Sequence>
std::vector<int> z_algorithm_impl(const Sequence& s) {
    if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("z_algorithm: sequence too long");
    const int n = static_cast<int>(s.size());
    std::vector<int> z(s.size());
    if (n == 0) return z;
    z[0] = n;
    int i = 1, length = 0;
    while (i < n) {
        if (length == 0) {
            // 未処理位置のz[i]は零初期化済み。不一致なら書き直さない。
            if (!static_cast<bool>(s[0] == s[i])) { ++i; continue; }
            length = 1;
        }
        while (length < n - i && s[length] == s[i + length]) ++length;
        z[i] = length;
        // 末尾まで一致した箱では、残りも確定済みの値だけで埋められる。
        if (length == n - i) {
            if (i == 1) {
                for (++i; i < n; ++i) z[i] = n - i;
            } else {
                const int left = i;
                for (++i; i < n; ++i) z[i] = std::min(n - i, z[i - left]);
            }
            break;
        }
        // 箱内で不一致まで確定する位置を写し、次の位置へ一致長を持ち越す。
        int advance = 1;
        while (advance < length && z[advance] < length - advance) {
            z[i + advance] = z[advance];
            ++advance;
        }
        i += advance;
        length = std::max(0, length - advance);
    }
    return z;
}
}
#endif

#ifndef SIRKIT_Z_ALGORITHM_DEFINED
#define SIRKIT_Z_ALGORITHM_DEFINED
template<class T>
std::vector<int> z_algorithm(const std::vector<T>& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
inline std::vector<int> z_algorithm(const std::string& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> z_algorithm(const Sequence& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
#endif

#ifndef SIRKIT_Z_MATCHES_DEFINED
#define SIRKIT_Z_MATCHES_DEFINED
template <class Pattern, class Text>
std::vector<int> PrefixMatches(const Pattern& pattern, const Text& text) {
    if (text.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("PrefixMatches: text too long");
    const auto z = z_algorithm(pattern);
    const int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
    std::vector<int> matches(text.size());
    int left = 0, right = 0;
    for (int i = 0; i < n; ++i) {
        int length = i < right ? std::min(right - i, z[i - left]) : 0;
        while (length < m && length < n - i && pattern[length] == text[i + length]) ++length;
        matches[i] = length;
        if (i + length > right) { left = i; right = i + length; }
    }
    return matches;
}

template <class Pattern, class Text>
std::vector<int> ZMatches(const Pattern& pattern, const Text& text) {
    auto lengths = PrefixMatches(pattern, text);
    std::vector<int> positions;
    const int m = static_cast<int>(pattern.size());
    for (int i = 0; i < static_cast<int>(lengths.size()); ++i)
        if (lengths[i] == m) positions.push_back(i);
    if (m == 0) positions.push_back(static_cast<int>(text.size()));
    return positions;
}
#endif

#ifndef SIRKIT_RUN_LENGTH_Z_DEFINED
#define SIRKIT_RUN_LENGTH_Z_DEFINED

class RunLengthZ
{
public:
    struct Run
    {
        std::uint8_t byte;
        std::uint64_t count;
        friend constexpr bool operator==(const Run &left, const Run &right) noexcept
        {
            return left.byte == right.byte && left.count == right.count;
        }
    };

    explicit RunLengthZ(std::span<const Run> input)
    {
        std::vector<Run> packed;
        std::uint64_t total = 0;
        for (const auto run : input)
        {
            if (run.count == 0)
            {
                continue;
            }
            if (run.count > std::numeric_limits<std::uint64_t>::max() - total)
            {
                throw std::length_error("RunLengthZ: expanded length overflows");
            }
            total += run.count;
            if (!packed.empty() && packed.back().byte == run.byte)
            {
                packed.back().count += run.count;
            }
            else
            {
                packed.push_back(run);
            }
        }
        if (packed.empty())
        {
            return;
        }
        runs_.reserve(packed.size());
        std::uint64_t end = 0;
        for (const auto run : packed)
        {
            end += run.count;
            runs_.push_back({run, end, 0});
        }

        const auto first = packed.front();
        const auto equal_runs = z_algorithm(std::span<const Run>(packed).subspan(1));
        for (std::size_t i = 0; i < equal_runs.size(); ++i)
        {
            if (packed[i].byte != first.byte || packed[i].count < first.count)
            {
                continue;
            }
            // 最初の連続区間の長さが同じとなる開始位置だけ、次の区間へ進める。
            const auto matched = static_cast<std::size_t>(equal_runs[i]);
            auto matched_bytes = runs_[matched].end - first.count;
            const auto next = i + matched + 1;
            if (next < packed.size() && packed[next].byte == packed[matched + 1].byte)
            {
                matched_bytes += std::min(packed[next].count, packed[matched + 1].count);
            }
            runs_[i].extension = matched_bytes;
        }
    }

    [[nodiscard]] std::uint64_t size() const noexcept
    {
        return runs_.empty() ? 0 : runs_.back().end;
    }

    [[nodiscard]] std::uint64_t prefix_at(std::uint64_t position) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (position > size())
        {
            throw std::out_of_range("RunLengthZ: position outside text");
        }
#endif
        if (position == size())
        {
            return 0;
        }
        const auto it = std::upper_bound(runs_.begin(), runs_.end(), position,
                                         [](std::uint64_t value, const Summary &run) { return value < run.end; });
        const auto first = runs_.front().run;
        if (it->run.byte != first.byte)
        {
            return 0;
        }
        const auto remaining = it->end - position;
        return remaining == first.count ? first.count + it->extension : std::min(remaining, first.count);
    }

    [[nodiscard]] std::uint32_t sum_prefix_lengths(std::uint32_t modulus = 1'000'000'007) const
    {
        if (modulus == 0)
        {
            throw std::invalid_argument("RunLengthZ: modulus must be positive");
        }
        const auto product = [&](std::uint64_t left, std::uint64_t right) {
            return (left % modulus) * (right % modulus) % modulus;
        };
        std::uint64_t result = 0;
        for (const auto &summary : runs_)
        {
            const auto first = runs_.front().run;
            if (summary.run.byte != first.byte)
            {
                continue;
            }
            const auto common = std::min(summary.run.count, first.count);
            const auto triangle = product(common / 2 + (common % 2), common | std::uint64_t{1});
            const auto plateau = product(summary.run.count - common, common);
            result = (result + triangle + plateau + summary.extension % modulus) % modulus;
        }
        return static_cast<std::uint32_t>(result);
    }

private:
    struct Summary
    {
        Run run;
        std::uint64_t end;
        std::uint64_t extension;
    };
    std::vector<Summary> runs_;
};

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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<sirkit::RunLengthZ::Run> runs(n);
    for (auto &run : runs)
    {
        verify_in >> run.count;
    }
    std::string text;
    verify_in >> text;
    for (std::size_t i = 0; i < n; ++i)
    {
        runs[i].byte = static_cast<std::uint8_t>(text[i]);
    }
    verify_out << sirkit::RunLengthZ(runs).sum_prefix_lengths() << '\n';
}
