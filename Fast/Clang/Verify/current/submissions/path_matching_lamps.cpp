#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc218/tasks/abc218_h

// Library: Graph/Path_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Path_Matching.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/path_matching.md
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/matching_on_path
//
// API一覧
// PathMatching<T>(weights): パスの辺 (i,i+1) の重み weights[i] を順に入力。
//   ちょうど k 本の互いに接しない辺を選ぶ最大重みを、すべての k について前処理。
//   重みは負・ゼロも可。T は 0・加減算・順序を備え、途中の交代和や解の総和を表現できること。
//   辺数 m に対して O((m+1) log(m+2)) 時間、O(m+1) 領域。
// max_cardinality()/best(k)/values(): 最大本数/ちょうど k 本の最大重み/本数順の const span。O(1)。
//   values()[0] は 0。空の辺列にも対応。不正な k は例外。
// edge_indices(k): ちょうど k 本で best(k) を達成する 0 始まりの辺番号を昇順に返す。O(m+1) 時間・領域。
// maximum_path_matching_weight<T>(weights,k): 1 つの k での最大重み。平均 O(m+1) 時間、O(m+1) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PATH_MATCHING_DEFINED
#define SIRKIT_PATH_MATCHING_DEFINED

namespace detail
{
template <class T>
struct PathMatchingChange
{
    T gain;
    std::size_t first, end, serial;
};

template <class T>
bool path_gain_descending(const PathMatchingChange<T> &a, const PathMatchingChange<T> &b)
{
    if (b.gain < a.gain)
    {
        return true;
    }
    return !(a.gain < b.gain) && a.serial < b.serial;
}

template <class T>
std::vector<PathMatchingChange<T>> path_matching_changes(std::span<const T> weights)
{
    const auto n = weights.size() + 1;
    std::vector<T> potential(n, T{0});
    for (std::size_t i = 0; i < weights.size(); ++i)
    {
        potential[i + 1] = weights[i] - potential[i];
    }

    std::vector<std::size_t> stack;
    stack.reserve(n);
    std::vector<PathMatchingChange<T>> changes;
    changes.reserve(n / 2);
    // スタック内の候補値は左から狭義単調増加。右の新候補より小さくなるまで末尾の局所最大を取り出す。
    for (std::size_t next = 0; next < n; ++next)
    {
        while (stack.size() >= 2)
        {
            const auto size = stack.size();
            const auto first = stack[size - 2];
            const auto second = stack[size - 1];
            if (potential[first] < potential[next])
            {
                break;
            }
            changes.push_back({potential[first] + potential[second], first, second, changes.size()});
            stack.resize(size - 2);
        }
        stack.push_back(next);
    }
    for (auto size = stack.size(); size >= 2; size -= 2)
    {
        const auto first = stack[size - 2];
        const auto second = stack[size - 1];
        changes.push_back({potential[first] + potential[second], first, second, changes.size()});
    }
    return changes;
}
} // namespace detail

template <class T>
class PathMatching
{
public:
    explicit PathMatching(std::span<const T> weights)
        : edge_count_(weights.size()), changes_(detail::path_matching_changes(weights)), values_(1, T{0})
    {
        std::sort(changes_.begin(), changes_.end(), detail::path_gain_descending<T>);
        values_.reserve(changes_.size() + 1);
        for (const auto &change : changes_)
        {
            values_.push_back(values_.back() + change.gain);
        }
    }

    [[nodiscard]] std::size_t max_cardinality() const
    {
        return changes_.size();
    }

    [[nodiscard]] std::span<const T> values() const
    {
        return values_;
    }

    [[nodiscard]] T best(std::size_t k) const
    {
        return values_.at(k);
    }

    [[nodiscard]] std::vector<std::size_t> edge_indices(std::size_t k) const
    {
        if (k > max_cardinality())
        {
            throw std::out_of_range("PathMatching: cardinality");
        }
        if (k == 0)
        {
            return {};
        }

        std::vector<unsigned char> boundaries(edge_count_ + 1);
        for (std::size_t i = 0; i < k; ++i)
        {
            boundaries[changes_[i].first] ^= 1U;
            boundaries[changes_[i].end] ^= 1U;
        }
        std::vector<std::size_t> selected;
        selected.reserve(k);
        unsigned char flipped = 0;
        for (std::size_t edge = 0; edge < edge_count_; ++edge)
        {
            flipped ^= boundaries[edge];
            if (flipped != 0)
            {
                selected.push_back(edge);
            }
        }
        return selected;
    }

private:
    std::size_t edge_count_;
    std::vector<detail::PathMatchingChange<T>> changes_;
    std::vector<T> values_;
};

template <class T>
[[nodiscard]] T maximum_path_matching_weight(std::span<const T> weights, std::size_t k)
{
    const auto max_cardinality = (weights.size() + 1) / 2;
    if (k > max_cardinality)
    {
        throw std::out_of_range("maximum_path_matching_weight: cardinality");
    }
    if (k == 0)
    {
        return T{0};
    }
    auto changes = detail::path_matching_changes(weights);
    if (k < max_cardinality)
    {
        std::nth_element(changes.begin(), changes.begin() + k, changes.end(), detail::path_gain_descending<T>);
    }
    T total{0};
    for (std::size_t i = 0; i < k; ++i)
    {
        total = total + changes[i].gain;
    }
    return total;
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


#include <algorithm>
#include <cstdint>
#include <vector>

int main()
{
    int n{}, red{};
    verify_in >> n >> red;
    std::vector<std::int64_t> rewards(n);
    for (int i = 1; i < n; ++i)
    {
        std::int64_t value{};
        verify_in >> value;
        rewards[i - 1] += value;
        rewards[i] += value;
    }
    verify_out << sirkit::maximum_path_matching_weight<std::int64_t>(
                      rewards, std::min(red, n - red))
               << '\n';
}
