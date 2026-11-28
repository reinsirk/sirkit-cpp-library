#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: a9f8259ba5abd9eb422988bb093152c85e66426689382173c9017af7483559fb
// Verify: https://atcoder.jp/contests/abc179/tasks/abc179_e

// Library: Algorithm/State_Cycle.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/state_cycle.md
//
// API一覧
// StateCycle{prefix, period}: 周期が始まる0始まりの位置と最短周期。
// find_state_cycle(start, next, transition_limit, equal={}): Brent法でStateCycleを求める。
// nextをtransition_limit回以内に呼び、結果まで求まらなければnullopt。周期を発見できる列でO(min(limit, prefix+period)+1)時間。
// fold_state_sequence(start, count, next, project, combine, identity, equal={}): 先頭count状態を順番に集約する。
// combineは結合的、identityは単位元。周期がある列でO(min(count, prefix+period)+log(count+1)+1)時間。
// 周期がなくても指定した個数まで処理できる。各O(1)追加領域。状態・集約値のコピーと関数呼び出しはO(1)とする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATE_CYCLE_DEFINED
#define SIRKIT_STATE_CYCLE_DEFINED

struct StateCycle
{
    std::uint64_t prefix;
    std::uint64_t period;
};

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
template<class State>
class BrentCycleTracker
{
public:
    explicit BrentCycleTracker(const State &initial) : checkpoint_(initial)
    {
    }

    template<class Equal>
    std::uint64_t observe(const State &state, Equal &equal)
    {
        ++length_;
        if (std::invoke(equal, std::as_const(checkpoint_), state))
        {
            return length_;
        }
        if (length_ == window_)
        {
            checkpoint_ = state;
            // 比較区間を倍にする。残りの観測回数で表現できる上限を越えないようにする。
            window_ += std::min(window_, std::numeric_limits<std::uint64_t>::max() - window_);
            length_ = 0;
        }
        return 0;
    }

private:
    State checkpoint_;
    std::uint64_t window_ = 1;
    std::uint64_t length_ = 0;
};
} // namespace reinsirk::detail

template<class State, class Next, class Equal = std::equal_to<>>
std::optional<StateCycle> find_state_cycle(State start, Next next, std::uint64_t transition_limit, Equal equal = {})
{
    detail::BrentCycleTracker<State> tracker(start);
    auto current = start;
    while (transition_limit != 0)
    {
        current = std::invoke(next, std::as_const(current));
        --transition_limit;
        const auto period = tracker.observe(current, equal);
        if (period == 0)
        {
            continue;
        }
        if (transition_limit < period)
        {
            return std::nullopt;
        }

        // 同じ初期状態から、片方だけ周期分先行させると周期の入口で一致する。
        current = start;
        for (std::uint64_t step = 0; step < period; ++step)
        {
            current = std::invoke(next, std::as_const(current));
        }
        transition_limit -= period;
        std::uint64_t prefix = 0;
        while (!std::invoke(equal, std::as_const(start), std::as_const(current)))
        {
            if (transition_limit < 2)
            {
                return std::nullopt;
            }
            start = std::invoke(next, std::as_const(start));
            current = std::invoke(next, std::as_const(current));
            transition_limit -= 2;
            ++prefix;
        }
        return StateCycle{prefix, period};
    }
    return std::nullopt;
}

template<class State, class Next, class Project, class Combine, class Value, class Equal = std::equal_to<>>
Value fold_state_sequence(State start, std::uint64_t count, Next next, Project project, Combine combine, Value identity,
                          Equal equal = {})
{
    detail::BrentCycleTracker<State> tracker(start);
    auto result = identity;
    while (count != 0)
    {
        result = std::invoke(combine, std::as_const(result), std::invoke(project, std::as_const(start)));
        if (--count == 0)
        {
            break;
        }
        start = std::invoke(next, std::as_const(start));
        const auto period = tracker.observe(start, equal);
        if (period == 0)
        {
            continue;
        }

        auto copies = count / period;
        if (copies != 0)
        {
            auto power = identity;
            auto current = start;
            for (std::uint64_t step = 0; step < period; ++step)
            {
                power = std::invoke(combine, std::as_const(power), std::invoke(project, std::as_const(current)));
                current = std::invoke(next, std::as_const(current));
            }
            for (; copies != 0; copies >>= 1)
            {
                if ((copies & 1) != 0)
                {
                    result = std::invoke(combine, std::as_const(result), std::as_const(power));
                }
                if (copies > 1)
                {
                    power = std::invoke(combine, std::as_const(power), std::as_const(power));
                }
            }
        }

        const auto tail = count % period;
        for (std::uint64_t step = 0; step < tail; ++step)
        {
            result = std::invoke(combine, std::as_const(result), std::invoke(project, std::as_const(start)));
            start = std::invoke(next, std::as_const(start));
        }
        break;
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

#include <cstdint>
#include <functional>

int main()
{
    std::uint64_t count = 0, start = 0, modulus = 0;
    verify_in >> count >> start >> modulus;
    const auto next = [modulus](std::uint64_t state) { return state * state % modulus; };
    verify_out << sirkit::fold_state_sequence(start, count, next, std::identity{}, std::plus<>{}, std::uint64_t{0}) << '\n';
}
