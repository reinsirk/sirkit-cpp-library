#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/408

// Library: Graph/Short_Simple_Path.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/short_simple_path.md
// 参考（外積を用いた代表族）: https://arxiv.org/abs/1304.4626
//
// API一覧
// find_short_simple_path(graph,k,starts,ends): k頂点の単純パスを頂点列で1つ返す。存在しなければnullopt。
// 有向辺の隣接リスト。始点候補starts・終点候補endsは0始まり。1<=k<=8、頂点数は998244353未満。
// 乱数は使わない。候補列の重複を許す。多重辺・自己ループがあっても同じ頂点は再訪しない。
// n頂点m辺、D=binom(k,floor(k/2))としてO(k(n+m)D²(D+k))時間、O(m+nD(D+k))領域。始点と終点の候補の読み取り時間を別途加える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORT_SIMPLE_PATH_DEFINED
#define SIRKIT_SHORT_SIMPLE_PATH_DEFINED

namespace detail
{
inline constexpr unsigned kShortPathLimit = 8;
inline constexpr std::uint32_t kShortPathMod = 998244353;

inline void ShortPathCheckParameters(std::size_t vertices, unsigned length)
{
    if (vertices >= kShortPathMod)
    {
        throw std::length_error("short simple path: too many vertices for distinct field labels");
    }
    if (length == 0 || length > kShortPathLimit)
    {
        throw std::invalid_argument("short simple path: number of vertices must be in [1,8]");
    }
}

inline std::size_t ShortPathVertex(int vertex, std::size_t vertices)
{
    if (vertex < 0 || static_cast<std::size_t>(vertex) >= vertices)
    {
        throw std::out_of_range("short simple path: vertex index out of range");
    }
    return static_cast<std::size_t>(vertex);
}

inline std::uint32_t ShortPathScalarProduct(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kShortPathMod);
}

inline std::uint32_t ShortPathInverse(std::uint32_t value)
{
    std::uint32_t result = 1;
    for (auto exponent = kShortPathMod - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = ShortPathScalarProduct(result, value);
        }
        value = ShortPathScalarProduct(value, value);
    }
    return result;
}

inline std::uint32_t ShortPathDifference(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : a + kShortPathMod - b;
}

using ShortPathVector = std::vector<std::uint32_t>;
using ShortPathLabels = std::array<std::uint32_t, kShortPathLimit>;

struct ShortPathRow
{
    std::array<int, kShortPathLimit> path{};
    ShortPathVector wedge;
};

struct ShortPathExteriorTerm
{
    std::size_t input;
    std::size_t output;
    unsigned power;
    bool negative;
};

inline std::vector<unsigned> ShortPathMasks(unsigned dimension, unsigned rank)
{
    std::vector<unsigned> masks;
    for (unsigned mask = 0; mask < (1u << dimension); ++mask)
    {
        if (static_cast<unsigned>(std::popcount(mask)) == rank)
        {
            masks.push_back(mask);
        }
    }
    return masks;
}

inline std::vector<ShortPathExteriorTerm> ShortPathExteriorTerms(unsigned dimension, std::span<const unsigned> previous,
                                                                 std::span<const unsigned> next)
{
    std::vector<ShortPathExteriorTerm> terms;
    std::array<std::size_t, 1u << kShortPathLimit> positions{};
    for (std::size_t i = 0; i < next.size(); ++i)
    {
        positions[next[i]] = i;
    }
    for (std::size_t index = 0; index < previous.size(); ++index)
    {
        const auto mask = previous[index];
        for (unsigned bit = 0; bit < dimension; ++bit)
        {
            if ((mask & (1u << bit)) == 0)
            {
                const bool negative = std::popcount(mask >> (bit + 1)) % 2 != 0;
                terms.push_back({index, positions[mask | (1u << bit)], bit, negative});
            }
        }
    }
    return terms;
}

inline ShortPathVector ShortPathExtend(std::span<const std::uint32_t> coefficients, const ShortPathLabels &label,
                                       std::span<const ShortPathExteriorTerm> terms, std::size_t size)
{
    ShortPathVector result(size);
    for (const auto &term : terms)
    {
        auto &value = result[term.output];
        const auto contribution = ShortPathScalarProduct(coefficients[term.input], label[term.power]);
        value = (term.negative ? value + kShortPathMod - contribution : value + contribution) % kShortPathMod;
    }
    return result;
}

inline bool ShortPathInsert(std::vector<ShortPathVector> &basis, ShortPathVector candidate)
{
    for (std::size_t lead = 0; lead < candidate.size(); ++lead)
    {
        const auto coefficient = candidate[lead];
        if (coefficient == 0)
        {
            continue;
        }
        const auto &row = basis[lead];
        if (row.empty())
        {
            const auto inverse = ShortPathInverse(coefficient);
            for (std::size_t j = lead; j < candidate.size(); ++j)
            {
                candidate[j] = ShortPathScalarProduct(candidate[j], inverse);
            }
            basis[lead] = std::move(candidate);
            return true;
        }
        for (std::size_t j = lead; j < candidate.size(); ++j)
        {
            candidate[j] = ShortPathDifference(candidate[j], ShortPathScalarProduct(coefficient, row[j]));
        }
    }
    return false;
}

inline bool ShortPathUsedVertex(const ShortPathRow &row, unsigned count, int vertex)
{
    const auto last = row.path.begin() + count;
    return std::find(row.path.begin(), last, vertex) != last;
}
} // namespace detail

[[nodiscard]] inline std::optional<std::vector<int>> find_short_simple_path(std::span<const std::vector<int>> graph,
                                                                            unsigned length,
                                                                            std::span<const int> starts,
                                                                            std::span<const int> ends)
{
    const auto n = graph.size();
    detail::ShortPathCheckParameters(n, length);
    std::vector<std::vector<std::size_t>> incoming(n);
    for (std::size_t u = 0; u < n; ++u)
    {
        for (const auto v : graph[u])
        {
            incoming[detail::ShortPathVertex(v, n)].push_back(u);
        }
    }
    std::vector<bool> can_start(n), can_end(n);
    for (const auto start : starts)
    {
        can_start[detail::ShortPathVertex(start, n)] = true;
    }
    for (const auto end : ends)
    {
        can_end[detail::ShortPathVertex(end, n)] = true;
    }
    if (length > n)
    {
        return std::nullopt;
    }
    if (length == 1)
    {
        for (std::size_t v = 0; v < n; ++v)
        {
            if (can_start[v] && can_end[v])
            {
                return std::vector<int>{static_cast<int>(v)};
            }
        }
        return std::nullopt;
    }

    std::vector<detail::ShortPathLabels> labels(n);
    std::vector<std::vector<detail::ShortPathRow>> layer(n);
    for (std::size_t v = 0; v < n; ++v)
    {
        auto &label = labels[v];
        label[0] = 1;
        for (unsigned power = 1; power < length; ++power)
        {
            label[power] = detail::ShortPathScalarProduct(label[power - 1], static_cast<std::uint32_t>(v + 1));
        }
        if (can_start[v])
        {
            auto &row = layer[v].emplace_back();
            row.path[0] = static_cast<int>(v);
            row.wedge.assign(label.begin(), label.begin() + length);
        }
    }
    auto masks = detail::ShortPathMasks(length, 1);
    for (unsigned count = 1; count + 1 < length; ++count)
    {
        const auto next_masks = detail::ShortPathMasks(length, count + 1);
        const auto terms = detail::ShortPathExteriorTerms(length, masks, next_masks);
        const auto dimension = next_masks.size();
        std::vector<std::vector<detail::ShortPathRow>> next(n);
        for (std::size_t v = 0; v < n; ++v)
        {
            const auto vertex = static_cast<int>(v);
            std::vector<detail::ShortPathVector> basis(dimension);
            for (const auto u : incoming[v])
            {
                for (const auto &path : layer[u])
                {
                    if (detail::ShortPathUsedVertex(path, count, vertex))
                    {
                        continue;
                    }
                    auto wedge = detail::ShortPathExtend(path.wedge, labels[v], terms, dimension);
                    if (detail::ShortPathInsert(basis, wedge))
                    {
                        auto &state = next[v].emplace_back(detail::ShortPathRow{path.path, std::move(wedge)});
                        state.path[count] = vertex;
                    }
                }
            }
        }
        // 同じ終点で外積を張る基底を残せば、どの後続の単純パスも見落とさない。
        masks = next_masks;
        layer.swap(next);
    }

    for (std::size_t v = 0; v < n; ++v)
    {
        if (!can_end[v])
        {
            continue;
        }
        const auto vertex = static_cast<int>(v);
        for (const auto u : incoming[v])
        {
            for (const auto &path : layer[u])
            {
                if (!detail::ShortPathUsedVertex(path, length - 1, vertex))
                {
                    std::vector<int> found(path.path.begin(), path.path.begin() + (length - 1));
                    found.push_back(vertex);
                    return found;
                }
            }
        }
    }
    return std::nullopt;
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
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    std::vector<std::vector<int>> graph(n);
    std::vector<int> neighbors;
    for (std::size_t i = 0; i < m; ++i)
    {
        int u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        if (u == 0 || v == 0)
        {
            neighbors.push_back(u + v);
        }
        else
        {
            graph[static_cast<std::size_t>(u)].push_back(v);
            graph[static_cast<std::size_t>(v)].push_back(u);
        }
    }
    verify_out << (sirkit::find_short_simple_path(graph, 4, neighbors, neighbors) ? "YES\n" : "NO\n");
}
