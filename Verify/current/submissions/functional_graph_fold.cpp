#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: 395264d3c69831e2ca0a9cb8de9400283d614ac23442afe4dfb13d17d50035f8
// Verify: https://atcoder.jp/contests/abc241/tasks/abc241_e

// Library: Graph/Functional_Graph_Fold.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/functional_graph_fold.md
//
// API一覧
// FunctionalGraphFold<T, Combine>(next, label, identity, combine={}):
//   頂点vからnext[v]へ進む辺にlabel[v]を置く。combineは結合的でidentityは両側単位元。
//   前処理と領域はO(n log(n+1))。積と値のコピーをO(1)とする。
// walk(v, k): k本の辺を進んだ頂点と、通った辺ラベルの順序付き積を返す。
//   kはuint64_t。0本では{v, identity}。O(log(n+1)+log(k+1))。
// size(): 頂点数。O(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FUNCTIONAL_GRAPH_FOLD_DEFINED
#define SIRKIT_FUNCTIONAL_GRAPH_FOLD_DEFINED

namespace detail
{
inline int functional_graph_fold_vertex_count(std::size_t n)
{
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("FunctionalGraphFold: too many vertices");
    }
    return static_cast<int>(n);
}
} // namespace detail

template <class T, class Combine = std::plus<T>>
class FunctionalGraphFold
{
    struct Jump
    {
        int end;
        T product;
    };

public:
    FunctionalGraphFold(std::span<const int> next, std::span<const T> label, T identity, Combine combine = {})
        : identity_(std::move(identity)), combine_(std::move(combine))
    {
        const int n = detail::functional_graph_fold_vertex_count(next.size());
        if (next.size() != label.size())
        {
            throw std::invalid_argument("FunctionalGraphFold: inconsistent label count");
        }
        build_periods(next, n);
        const unsigned levels = std::bit_width(static_cast<unsigned>(n));
        jumps_.reserve(next.size() * levels);
        for (int v = 0; v < n; ++v)
        {
            jumps_.push_back({next[v], label[v]});
        }
        for (unsigned level = 1; level < levels; ++level)
        {
            const std::size_t base = static_cast<std::size_t>(n) * (level - 1);
            for (int v = 0; v < n; ++v)
            {
                const auto &first = jumps_[base + v];
                const auto &second = jumps_[base + first.end];
                jumps_.push_back({second.end, combine_(first.product, second.product)});
            }
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(depth_.size());
    }

    [[nodiscard]] std::pair<int, T> walk(int from, std::uint64_t steps) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= size())
        {
            throw std::out_of_range("FunctionalGraphFold: invalid start vertex");
        }
#endif
        T product = identity_;
        if (steps <= depth_.size())
        {
            append(from, steps, product);
            return {from, std::move(product)};
        }

        const auto tail = static_cast<std::uint64_t>(depth_[from]);
        append(from, tail, product);
        steps -= tail;
        const auto period = static_cast<std::uint64_t>(period_[from]);
        T cycle = identity_;
        append(from, period, cycle);
        for (std::uint64_t copies = steps / period; copies != 0;)
        {
            if (copies & 1)
            {
                product = combine_(product, cycle);
            }
            copies >>= 1;
            if (copies != 0)
            {
                cycle = combine_(cycle, cycle);
            }
        }
        append(from, steps % period, product);
        return {from, std::move(product)};
    }

private:
    void build_periods(std::span<const int> next, int n)
    {
        std::vector<int> indegree(n);
        for (const int v : next)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (v < 0 || v >= n)
            {
                throw std::out_of_range("FunctionalGraphFold: invalid destination");
            }
#endif
            ++indegree[v];
        }
        std::vector<int> peeled;
        peeled.reserve(n);
        for (int v = 0; v < n; ++v)
        {
            if (indegree[v] == 0)
            {
                peeled.push_back(v);
            }
        }
        for (std::size_t i = 0; i < peeled.size(); ++i)
        {
            const int to = next[peeled[i]];
            if (--indegree[to] == 0)
            {
                peeled.push_back(to);
            }
        }

        depth_.resize(n);
        period_.resize(n);
        for (int start = 0; start < n; ++start)
        {
            if (indegree[start] != 0 && period_[start] == 0)
            {
                int length = 1;
                for (int v = next[start]; v != start; v = next[v])
                {
                    ++length;
                }
                int v = start;
                do
                {
                    period_[v] = length;
                    v = next[v];
                } while (v != start);
            }
        }
        for (auto it = peeled.rbegin(); it != peeled.rend(); ++it)
        {
            const int v = *it;
            depth_[v] = depth_[next[v]] + 1;
            period_[v] = period_[next[v]];
        }
    }

    void append(int &from, std::uint64_t steps, T &product) const
    {
        while (steps != 0)
        {
            const auto level = std::countr_zero(steps);
            const auto &jump = jumps_[depth_.size() * level + from];
            product = combine_(product, jump.product);
            from = jump.end;
            steps &= steps - 1;
        }
    }

    T identity_;
    [[no_unique_address]] Combine combine_;
    std::vector<int> depth_;
    std::vector<int> period_;
    std::vector<Jump> jumps_;
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
#include <vector>

int main()
{
    int n{};
    std::uint64_t k{};
    verify_in >> n >> k;
    std::vector<int> next(n);
    std::vector<std::uint64_t> candies(n);
    for (int v = 0; v < n; ++v)
    {
        verify_in >> candies[v];
        next[v] = static_cast<int>((candies[v] + v) % n);
    }
    const sirkit::FunctionalGraphFold<std::uint64_t> moves(next, candies, 0);
    verify_out << moves.walk(0, k).second << '\n';
}
