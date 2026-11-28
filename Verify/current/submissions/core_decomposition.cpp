#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: eb2a2a6a9e1743fead3d3596544eddd8d9bb2e6fce29d42d48b6cf2b0e2d83f4
// Verify: https://atcoder.jp/contests/abc267/tasks/abc267_e

// Library: Graph/Core_Decomposition.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/core_decomposition.md
//
// API一覧
// core_decomposition(n, edges): 0 始まりの無向グラフを k-core 分解する。O(n+m+1) 時間。
// weighted_core_decomposition<T>(n, edges, weights): 次数を残っている隣接頂点の重みの和に置き換える。
//   T は bool 以外の整数型。非負重みを頂点数分渡し、隣接重みの総和は T に収まること。
//   O((n+m+1) log(n+m+2)) 時間。どちらも O(n+m+1) 領域。多重辺可、自己辺不可。
// CoreDecompositionResult<T>{number, order, degeneracy}:
//   number[v] は v が属する最大の k-core の k。order は削除順、degeneracy は最大のコア番号。
//   この順での各削除コストは number[v] 以下。重み付きでは最大削除コストを最小にする。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CORE_DECOMPOSITION_DEFINED
#define SIRKIT_CORE_DECOMPOSITION_DEFINED

template <class T>
struct CoreDecompositionResult
{
    std::vector<T> number;
    std::vector<int> order;
    T degeneracy;
};

namespace detail
{
inline std::vector<std::vector<int>> core_decomposition_graph(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("core_decomposition: negative vertex count");
    }
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("core_decomposition: edge endpoint");
        }
        if (u == v)
        {
            throw std::invalid_argument("core_decomposition: self-loop");
        }
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    return graph;
}

template <std::integral T>
void add_core_weight(T &total, T weight)
{
    if (weight > std::numeric_limits<T>::max() - total)
    {
        throw std::overflow_error("weighted_core_decomposition: neighbor sum exceeds result type");
    }
    total += weight;
}
} // namespace detail

[[nodiscard]] inline CoreDecompositionResult<std::size_t> core_decomposition(
    int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::core_decomposition_graph(n, edges);
    CoreDecompositionResult<std::size_t> result{std::vector<std::size_t>(n), std::vector<int>(n), 0};
    std::size_t maximum = 0;
    for (int v = 0; v < n; ++v)
    {
        result.number[v] = graph[v].size();
        maximum = std::max(maximum, result.number[v]);
    }
    std::vector<std::size_t> first(maximum + 1);
    for (const auto d : result.number)
    {
        ++first[d];
    }
    std::size_t start = 0;
    for (auto &bucket : first)
    {
        const auto count = bucket;
        bucket = start;
        start += count;
    }
    auto next = first;
    std::vector<std::size_t> position(n);
    for (int v = 0; v < n; ++v)
    {
        position[v] = next[result.number[v]]++;
        result.order[position[v]] = v;
    }
    for (const int v : result.order)
    {
        const auto threshold = result.number[v];
        result.degeneracy = threshold;
        for (const int w : graph[v])
        {
            auto &degree = result.number[w];
            if (degree > threshold)
            {
                const auto front = first[degree]++;
                const auto at = position[w];
                const int other = result.order[front];
                std::swap(result.order[front], result.order[at]);
                position[w] = front;
                position[other] = at;
                --degree;
            }
        }
    }
    return result;
}

template <std::integral T>
    requires(!std::same_as<T, bool>)
[[nodiscard]] CoreDecompositionResult<T> weighted_core_decomposition(
    int n, std::span<const std::pair<int, int>> edges, std::span<const T> weights)
{
    const auto graph = detail::core_decomposition_graph(n, edges);
    if (weights.size() != graph.size())
    {
        throw std::invalid_argument("weighted_core_decomposition: wrong number of weights");
    }
    if constexpr (std::signed_integral<T>)
    {
        for (const T w : weights)
        {
            if (w < 0)
            {
                throw std::invalid_argument("weighted_core_decomposition: negative vertex weight");
            }
        }
    }

    using Candidate = std::pair<T, int>;
    std::vector<Candidate> initial;
    initial.reserve(n);
    std::vector<T> score(n);
    for (int v = 0; v < n; ++v)
    {
        for (const int w : graph[v])
        {
            detail::add_core_weight(score[v], weights[w]);
        }
        initial.emplace_back(score[v], v);
    }
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> queue(
        std::greater<Candidate>{}, std::move(initial));
    CoreDecompositionResult<T> result{std::vector<T>(n), {}, T{0}};
    result.order.reserve(n);
    std::vector<bool> removed(n);
    while (!queue.empty())
    {
        const auto [cost, v] = queue.top();
        queue.pop();
        if (removed[v])
        {
            continue;
        }
        removed[v] = true;
        result.degeneracy = std::max(result.degeneracy, cost);
        result.number[v] = result.degeneracy;
        result.order.push_back(v);
        for (const int w : graph[v])
        {
            if (!removed[w])
            {
                score[w] -= weights[v];
                queue.emplace(score[w], w);
            }
        }
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
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::int64_t> weights(n);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    verify_out << sirkit::weighted_core_decomposition<std::int64_t>(n, edges, weights).degeneracy << '\n';
}
