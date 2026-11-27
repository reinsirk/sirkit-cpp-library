#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/stern_brocot_tree

// Library: Math/Stern_Brocot_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Math/Stern_Brocot_Tree.cpp
// 参考: https://nyaannyaan.github.io/library/math/stern-brocot-tree-binary-search.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/stern_brocot_tree.md
//
// API一覧
// [SternBrocotTree]
// Fraction: uint64_tの{分子,分母}。正の分数を受け取り、既約分数を返す。
// Run: {方向,回数}。方向は'L'か'R'、回数は正。長さ0のパスは根1/1。
// encode(f): 連長圧縮した根からfへのパス。decode(path): パスから分数。
// ancestor(f,depth): 深さdepthの祖先。根は深さ0。深すぎる場合はnullopt。
// lca(a,b): a,bの最小共通祖先。range(f): f以下の部分木が表す開区間。
// approximate(f,max_numerator,max_denominator): 分子・分母が上限以下の最近接の下側・上側の分数。
// 範囲や近似に有限な端点がなければ0/1または1/0。
// 時間は扱うラン数Rに対してO(R+1)。分数からのラン数はO(log(max(分子,分母)+1))。
// encodeの出力以外の追加領域はO(1)。
// decodeは結果がuint64_tに収まらない場合にstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STERN_BROCOT_TREE_DEFINED
#define SIRKIT_STERN_BROCOT_TREE_DEFINED

class SternBrocotTree
{
public:
    using Fraction = std::pair<std::uint64_t, std::uint64_t>;
    using Run = std::pair<char, std::uint64_t>;

private:
    struct Interval
    {
        Fraction lower{0, 1};
        Fraction upper{1, 0};
    };

    static std::uint64_t AddScaled(std::uint64_t value, std::uint64_t other, std::uint64_t count)
    {
        if (other != 0 && count > (std::numeric_limits<std::uint64_t>::max() - value) / other)
        {
            throw std::overflow_error("SternBrocotTree: fraction");
        }
        return value + other * count;
    }

    static void Move(Interval &interval, char direction, std::uint64_t count)
    {
        auto &[a, b] = direction == 'L' ? interval.upper : interval.lower;
        const auto [c, d] = direction == 'L' ? interval.lower : interval.upper;
        a = AddScaled(a, c, count);
        b = AddScaled(b, d, count);
    }

    static Fraction Mediant(const Interval &interval)
    {
        const auto [a, b] = interval.lower;
        const auto [c, d] = interval.upper;
        return {AddScaled(a, c, 1), AddScaled(b, d, 1)};
    }

    static void CheckFraction([[maybe_unused]] Fraction fraction)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (fraction.first == 0 || fraction.second == 0)
        {
            throw std::invalid_argument("SternBrocotTree: positive fraction required");
        }
#endif
    }

    static Run TakeRun(Fraction &fraction)
    {
        auto &[a, b] = fraction;
        if (a < b)
        {
            const auto count = (b - 1) / a;
            b -= count * a;
            return {'L', count};
        }
        const auto count = (a - 1) / b;
        a -= count * b;
        return {'R', count};
    }

public:
    static std::vector<Run> encode(Fraction fraction)
    {
        CheckFraction(fraction);
        std::vector<Run> path;
        while (fraction.first != fraction.second)
        {
            path.push_back(TakeRun(fraction));
        }
        return path;
    }

    static Fraction decode(std::span<const Run> path)
    {
        Interval interval;
        for (const auto &[direction, count] : path)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if ((direction != 'L' && direction != 'R') || count == 0)
            {
                throw std::invalid_argument("SternBrocotTree: invalid path");
            }
#endif
            Move(interval, direction, count);
        }
        return Mediant(interval);
    }

    static std::optional<Fraction> ancestor(Fraction fraction, std::uint64_t depth)
    {
        CheckFraction(fraction);
        Interval interval;
        while (depth != 0 && fraction.first != fraction.second)
        {
            const auto [direction, count] = TakeRun(fraction);
            const auto steps = std::min(depth, count);
            Move(interval, direction, steps);
            depth -= steps;
        }
        if (depth != 0)
        {
            return std::nullopt;
        }
        return Mediant(interval);
    }

    static Fraction lca(Fraction a, Fraction b)
    {
        CheckFraction(a);
        CheckFraction(b);
        Interval interval;
        while (a.first != a.second && b.first != b.second)
        {
            const auto [left_direction, left_count] = TakeRun(a);
            const auto [right_direction, right_count] = TakeRun(b);
            if (left_direction != right_direction)
            {
                break;
            }
            Move(interval, left_direction, std::min(left_count, right_count));
            if (left_count != right_count)
            {
                break;
            }
        }
        return Mediant(interval);
    }

    static std::pair<Fraction, Fraction> approximate(Fraction fraction, std::uint64_t max_numerator, std::uint64_t max_denominator)
    {
        CheckFraction(fraction);
        Interval interval;
        if (max_numerator == 0 || max_denominator == 0)
        {
            return {interval.lower, interval.upper};
        }
        while (interval.lower.first <= max_numerator - interval.upper.first &&
               interval.lower.second <= max_denominator - interval.upper.second)
        {
            if (fraction.first == fraction.second)
            {
                const auto exact = Mediant(interval);
                return {exact, exact};
            }
            const auto [direction, count] = TakeRun(fraction);
            const auto &[a, b] = direction == 'L' ? interval.upper : interval.lower;
            const auto &[c, d] = direction == 'L' ? interval.lower : interval.upper;
            auto steps = count;
            if (c != 0)
            {
                steps = std::min(steps, (max_numerator - a) / c);
            }
            if (d != 0)
            {
                steps = std::min(steps, (max_denominator - b) / d);
            }
            Move(interval, direction, steps);
        }
        return {interval.lower, interval.upper};
    }

    static std::pair<Fraction, Fraction> range(Fraction fraction)
    {
        CheckFraction(fraction);
        Interval interval;
        while (fraction.first != fraction.second)
        {
            const auto [direction, count] = TakeRun(fraction);
            Move(interval, direction, count);
        }
        return {interval.lower, interval.upper};
    }
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

#include <cstdint>
#include <string>
#include <vector>

int main()
{
    using Tree = SternBrocotTree;
    int queries{};
    verify_in >> queries;
    std::vector<Tree::Run> path;
    std::string operation;
    const auto read_fraction = []
    {
        Tree::Fraction fraction{};
        verify_in >> fraction.first >> fraction.second;
        return fraction;
    };
    const auto write_fraction = [](Tree::Fraction fraction, char separator = '\n')
    {
        verify_out << fraction.first << ' ' << fraction.second << separator;
    };
    while (queries-- > 0)
    {
        verify_in >> operation;
        if (operation == "ENCODE_PATH")
        {
            const auto encoded = Tree::encode(read_fraction());
            verify_out << encoded.size();
            for (const auto &[direction, count] : encoded)
            {
                verify_out << ' ' << direction << ' ' << count;
            }
            verify_out << '\n';
        }
        else if (operation == "DECODE_PATH")
        {
            int runs{};
            verify_in >> runs;
            path.resize(runs);
            for (auto &[direction, count] : path)
            {
                verify_in >> direction >> count;
            }
            write_fraction(Tree::decode(path));
        }
        else if (operation == "LCA")
        {
            const auto a = read_fraction();
            write_fraction(Tree::lca(a, read_fraction()));
        }
        else if (operation == "ANCESTOR")
        {
            std::uint64_t depth{};
            verify_in >> depth;
            const auto ancestor = Tree::ancestor(read_fraction(), depth);
            if (ancestor)
            {
                write_fraction(*ancestor);
            }
            else
            {
                verify_out << "-1\n";
            }
        }
        else
        {
            const auto [lower, upper] = Tree::range(read_fraction());
            write_fraction(lower, ' ');
            write_fraction(upper);
        }
    }
}
