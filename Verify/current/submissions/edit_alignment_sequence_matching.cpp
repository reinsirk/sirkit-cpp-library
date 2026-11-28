#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 12daf2fa93d5f867af39c606a3d7d945a7a3f14584fac720a95ffd8483bbee50
// Verify: https://atcoder.jp/contests/abc185/tasks/abc185_e

// Library: String/Edit_Alignment.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（線形領域での分割復元）: https://doi.org/10.1145/360825.360861
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/edit_alignment.md
//
// API一覧
// edit_alignment(source,target,equal={}): spanまたはstring_viewを受け取り、最小編集回数と操作列を返す。
// spanではequal(a,b)で元の値aと先の値bの一致を判定する。既定比較は==。
// EditAlignment{distance,steps}: distanceは挿入・削除・置換の合計、stepsは先頭から処理する操作列。
// EditAction::kMatch/kReplaceは両列、kEraseは元の列、kInsertは先の列を1要素進める。
// n,mを長さとしてO(n*m+n+m+1)時間、結果を除きO(1+min(n,m)+log(max(n,m)+1))領域。
// 空の列にも対応。同じコストの操作列が複数あればいずれかを返す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EDIT_ALIGNMENT_DEFINED
#define SIRKIT_EDIT_ALIGNMENT_DEFINED

enum class EditAction : unsigned char
{
    kMatch,
    kReplace,
    kErase,
    kInsert,
};

struct EditAlignment
{
    std::size_t distance = 0;
    std::vector<EditAction> steps;
};

namespace detail
{
inline std::size_t checked_edit_alignment_size(std::size_t source, std::size_t target, std::size_t capacity)
{
    if (source > capacity || target > capacity - source)
    {
        throw std::length_error("edit_alignment: operation capacity exceeded");
    }
    return source + target;
}

template <bool Reverse, class A, class B, class Equal>
std::vector<std::size_t> edit_alignment_cost_row(std::span<const A> source, std::span<const B> target, Equal &equal)
{
    std::vector<std::size_t> length(target.size() + 1);
    std::iota(length.begin(), length.end(), std::size_t{0});
    for (std::size_t i = 0; i < source.size(); ++i)
    {
        const auto &a = source[Reverse ? source.size() - 1 - i : i];
        auto diagonal = length[0]++;
        for (std::size_t j = 0; j < target.size(); ++j)
        {
            const auto &b = target[Reverse ? target.size() - 1 - j : j];
            const auto previous = length[j + 1];
            length[j + 1] = std::min({length[j] + 1, previous + 1, diagonal + !std::invoke(equal, a, b)});
            diagonal = previous;
        }
    }
    return length;
}

template <class A, class B, class Equal>
std::size_t edit_alignment_split(std::span<const A> left, std::span<const A> right, std::span<const B> target,
                                 Equal &equal)
{
    const auto start = edit_alignment_cost_row<false>(left, target, equal);
    const auto end = edit_alignment_cost_row<true>(right, target, equal);
    std::size_t split = 0;
    auto best = start[0] + end[target.size()];
    for (std::size_t j = 1; j <= target.size(); ++j)
    {
        const auto cost = start[j] + end[target.size() - j];
        if (cost < best)
        {
            best = cost;
            split = j;
        }
    }
    return split;
}

template <class A, class B, class Equal>
void append_edit_alignment(std::span<const A> a, std::span<const B> b, Equal &equal, EditAlignment &result,
                           EditAction erase, EditAction insert)
{
    const auto append = [&](EditAction kind, std::size_t count)
    {
        result.steps.insert(result.steps.end(), count, kind);
        if (kind != EditAction::kMatch)
        {
            result.distance += count;
        }
    };
    if (b.empty())
    {
        append(erase, a.size());
        return;
    }
    if (a.size() == 1)
    {
        const auto found = std::ranges::find_if(b, [&](const B &value)
                                                {
                                                    return std::invoke(equal, a.front(), value);
                                                });
        if (found == b.end())
        {
            append(EditAction::kReplace, 1);
            append(insert, b.size() - 1);
        }
        else
        {
            const auto position = static_cast<std::size_t>(found - b.begin());
            append(insert, position);
            append(EditAction::kMatch, 1);
            append(insert, b.size() - 1 - position);
        }
        return;
    }
    const auto middle = a.size() / 2;
    const auto left = a.first(middle), right = a.subspan(middle);
    const auto split = edit_alignment_split(left, right, b, equal);
    append_edit_alignment(left, b.first(split), equal, result, erase, insert);
    append_edit_alignment(right, b.subspan(split), equal, result, erase, insert);
}
}

template <class A, std::size_t NX, class B, std::size_t NY, class Equal = std::equal_to<>>
[[nodiscard]] EditAlignment edit_alignment(std::span<A, NX> source, std::span<B, NY> target, Equal equal = {})
{
    const std::span<const std::remove_const_t<A>> a = source;
    const std::span<const std::remove_const_t<B>> b = target;
    EditAlignment result;
    result.steps.reserve(detail::checked_edit_alignment_size(a.size(), b.size(), result.steps.max_size()));
    if (a.size() >= b.size())
    {
        detail::append_edit_alignment(a, b, equal, result, EditAction::kErase, EditAction::kInsert);
    }
    else
    {
        const auto reverse = [&](const B &right, const A &left)
        {
            return std::invoke(equal, left, right);
        };
        detail::append_edit_alignment(b, a, reverse, result, EditAction::kInsert, EditAction::kErase);
    }
    return result;
}

[[nodiscard]] inline EditAlignment edit_alignment(std::string_view source, std::string_view target)
{
    return edit_alignment(std::span<const char>(source.data(), source.size()),
                          std::span<const char>(target.data(), target.size()));
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


#include <span>
#include <vector>

int main()
{
    int n = 0, m = 0;
    verify_in >> n >> m;
    std::vector<int> a(n), b(m);
    for (auto &value : a)
    {
        verify_in >> value;
    }
    for (auto &value : b)
    {
        verify_in >> value;
    }
    verify_out << sirkit::edit_alignment(std::span{a}, std::span{b}).distance << '\n';
}
