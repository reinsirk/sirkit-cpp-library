#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://onlinejudge.u-aizu.ac.jp/problems/DPL_2_A

// Library: Graph/Hamiltonian_Route.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Hamiltonian_Route.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hamiltonian_route.md
// 参考（Held–Karp の部分集合DP）: https://doi.org/10.1137/0110015
//
// API一覧
// shortest_hamiltonian_path<Weight=int64_t>(n,start,cost_at,end=nullopt): startから全頂点をちょうど1度通る最小コストのパス。
// shortest_hamiltonian_cycle<Weight=int64_t>(n,start,cost_at): startから全頂点をちょうど1度通り、startへ戻る最小コストの巡回。
// 共通: {cost,vertices}。解がなければnullopt。cost_at(u,v)は辺コストかoptional。空なら辺なし。
// 負辺・有向辺を許す。頂点番号は0始まり。途中の和もWeightに収まること。O(n^2 2^n)時間・O(n 2^n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HAMILTONIAN_ROUTE_DEFINED
#define SIRKIT_HAMILTONIAN_ROUTE_DEFINED

template <class Weight>
struct HamiltonianRoute
{
    Weight cost{};
    std::vector<std::size_t> vertices;
};

namespace detail
{
inline std::size_t HamiltonianCell(std::size_t mask, std::size_t last, std::size_t stride)
{
    // 終点lastは必ずmask内にあるので、配列ではそのビットを省く。
    const auto low = (std::size_t{1} << last) - 1;
    return last * stride + ((mask & low) | ((mask >> 1) & ~low));
}

template <bool ReturnHome, class Weight, class CostAt>
std::optional<HamiltonianRoute<Weight>> FindHamiltonianRoute(std::size_t n, std::size_t start, const CostAt &cost_at,
                                                             std::optional<std::size_t> end)
{
    if (start >= n || (end && *end >= n))
    {
        throw std::out_of_range("shortest_hamiltonian_route: invalid vertex");
    }
    if (n == 1)
    {
        if constexpr (ReturnHome)
        {
            return HamiltonianRoute<Weight>{Weight{}, {start, start}};
        }
        else
        {
            return HamiltonianRoute<Weight>{Weight{}, {start}};
        }
    }
    if (end && *end == start)
    {
        return std::nullopt;
    }

    const auto tail = n - 1;
    if (tail >= std::numeric_limits<std::size_t>::digits)
    {
        throw std::length_error("shortest_hamiltonian_route: subset mask overflows");
    }
    const auto subsets = std::size_t{1} << tail;
    const auto stride = subsets / 2;
    std::vector<std::optional<Weight>> dp;
    if (stride > dp.max_size() / tail)
    {
        throw std::length_error("shortest_hamiltonian_route: DP table is too large");
    }
    dp.resize(tail * stride);
    std::vector<std::uint8_t> predecessor(dp.size());

    std::vector<std::size_t> vertices;
    vertices.reserve(tail);
    for (std::size_t vertex = 0; vertex < n; ++vertex)
    {
        if (vertex != start)
        {
            vertices.push_back(vertex);
        }
    }
    std::vector<std::optional<Weight>> edges(n * n);
    for (std::size_t u = 0; u < n; ++u)
    {
        for (std::size_t v = 0; v < n; ++v)
        {
            if (u != v)
            {
                edges[u * n + v] = std::optional<Weight>{cost_at(u, v)};
            }
        }
    }
    for (std::size_t mask = 1; mask < subsets; ++mask)
    {
        for (auto ends = mask; ends != 0; ends &= ends - 1)
        {
            const auto last = static_cast<std::size_t>(std::countr_zero(ends));
            const auto index = HamiltonianCell(mask, last, stride);
            const auto v = vertices[last];
            auto &best = dp[index];
            const auto prefix = mask ^ (std::size_t{1} << last);
            if (prefix == 0)
            {
                best = edges[start * n + v];
                continue;
            }
            for (auto prior = prefix; prior != 0; prior &= prior - 1)
            {
                const auto previous = static_cast<std::size_t>(std::countr_zero(prior));
                const auto &before = dp[HamiltonianCell(prefix, previous, stride)];
                const auto &edge = edges[vertices[previous] * n + v];
                if (before && edge)
                {
                    auto candidate = *before;
                    candidate += *edge;
                    if (!best || candidate < *best)
                    {
                        best = std::move(candidate);
                        predecessor[index] = static_cast<std::uint8_t>(previous);
                    }
                }
            }
        }
    }
    auto mask = subsets - 1;
    std::optional<Weight> best;
    std::size_t best_last = 0;
    for (std::size_t last = 0; last < tail; ++last)
    {
        if (end && *end != vertices[last])
        {
            continue;
        }
        auto score = dp[HamiltonianCell(mask, last, stride)];
        if (!score)
        {
            continue;
        }
        if constexpr (ReturnHome)
        {
            const auto &closing_edge = edges[vertices[last] * n + start];
            if (!closing_edge)
            {
                continue;
            }
            *score += *closing_edge;
        }
        if (!best || *score < *best)
        {
            best = std::move(score);
            best_last = last;
        }
    }
    if (!best)
    {
        return std::nullopt;
    }
    std::vector<std::size_t> route;
    route.reserve(n + ReturnHome);
    while (mask != 0)
    {
        route.push_back(vertices[best_last]);
        const auto index = HamiltonianCell(mask, best_last, stride);
        mask ^= std::size_t{1} << best_last;
        best_last = predecessor[index];
    }
    route.push_back(start);
    std::ranges::reverse(route);
    if constexpr (ReturnHome)
    {
        route.push_back(start);
    }
    return HamiltonianRoute<Weight>{*best, std::move(route)};
}
} // namespace detail

template <class Weight = std::int64_t, class CostAt>
std::optional<HamiltonianRoute<Weight>> shortest_hamiltonian_path(std::size_t n, std::size_t start, const CostAt &cost_at,
                                                                  std::optional<std::size_t> end = std::nullopt)
{
    return detail::FindHamiltonianRoute<false, Weight>(n, start, cost_at, end);
}

template <class Weight = std::int64_t, class CostAt>
std::optional<HamiltonianRoute<Weight>> shortest_hamiltonian_cycle(std::size_t n, std::size_t start, const CostAt &cost_at)
{
    return detail::FindHamiltonianRoute<true, Weight>(n, start, cost_at, std::nullopt);
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


#include <cstddef>
#include <optional>
#include <vector>

int main()
{
    std::size_t vertex_count{}, edge_count{};
    verify_in >> vertex_count >> edge_count;
    std::vector<std::optional<int>> edges(vertex_count * vertex_count);
    for (std::size_t edge = 0; edge < edge_count; ++edge)
    {
        std::size_t u{}, v{};
        int weight{};
        verify_in >> u >> v >> weight;
        edges[u * vertex_count + v] = weight;
    }
    const auto cycle = sirkit::shortest_hamiltonian_cycle(vertex_count, 0, [&](std::size_t u, std::size_t v)
                                                          {
                                                              return edges[u * vertex_count + v];
                                                          });
    verify_out << (cycle ? cycle->cost : -1) << '\n';
}
