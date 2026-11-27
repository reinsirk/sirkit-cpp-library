#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/maximum_independent_set

// Library: Graph/Maximum_Independent_Set.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Maximum_Independent_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/maximum_independent_set.md
//
// API一覧
// maximum_independent_set(n,edges): 最大個数の独立集合を、頂点番号の昇順で返す。
// maximum_weight_independent_set(weights,edges): 最大重みの独立集合を{weight,vertices}で返す。
// 重み版は非正の頂点を含めない。最適値がint64_tの上限を超えるとoverflow_error。
// どちらも頂点数は0以上46以下。重複辺は可、自己辺のある頂点は選ばない。
// 半分全列挙によりO(n+m+2^ceil(n/2))時間・O(n+2^floor(n/2))領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MAXIMUM_INDEPENDENT_SET_DEFINED
#define SIRKIT_MAXIMUM_INDEPENDENT_SET_DEFINED

inline constexpr int kMaxIndependentSetVertices = 46;

struct MaximumWeightIndependentSetResult
{
    std::int64_t weight;
    std::vector<int> vertices;
};

namespace detail
{
using IndependentSetMask = std::uint64_t;
inline constexpr auto kIndependentSetOverflow = std::uint64_t{1} << 63;

struct IndependentSetGraph
{
    std::array<IndependentSetMask, kMaxIndependentSetVertices> neighbors{};
    IndependentSetMask eligible = 0;
};

inline IndependentSetGraph make_independent_set_graph(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0 || n > kMaxIndependentSetVertices)
    {
        throw std::length_error("maximum_independent_set: vertex count must be in [0,46]");
    }
    IndependentSetGraph graph;
    graph.eligible = (IndependentSetMask{1} << n) - 1;
    for (const auto &[u, v] : edges)
    {
        const auto from = IndependentSetMask{1} << u;
        const auto to = IndependentSetMask{1} << v;
        graph.neighbors[u] |= to;
        graph.neighbors[v] |= from;
        if (u == v)
        {
            graph.eligible &= ~from;
        }
    }
    return graph;
}

template <class Score>
Score add_independent_set_scores(Score a, Score b)
{
    if constexpr (std::is_same_v<Score, std::uint64_t>)
    {
        return a <= kIndependentSetOverflow - b ? a + b : kIndependentSetOverflow;
    }
    else
    {
        return static_cast<Score>(a + b);
    }
}

template <class Score>
MaximumWeightIndependentSetResult solve_independent_set(
    const IndependentSetGraph &graph, const std::array<Score, kMaxIndependentSetVertices> &weights)
{
    std::array<int, kMaxIndependentSetVertices> vertices{};
    std::array<int, kMaxIndependentSetVertices> indices{};
    IndependentSetMask selected = 0;
    Score base = 0;
    int count = 0;
    for (auto remaining = graph.eligible; remaining != 0; remaining &= remaining - 1)
    {
        const int v = std::countr_zero(remaining);
        if ((graph.neighbors[v] & graph.eligible) == 0)
        {
            // 候補同士で隣接しない正の重みの頂点は必ず採用できる。
            selected |= IndependentSetMask{1} << v;
            base = add_independent_set_scores(base, weights[v]);
        }
        else
        {
            indices[v] = count;
            vertices[count++] = v;
        }
    }

    std::array<IndependentSetMask, kMaxIndependentSetVertices> neighbors{};
    for (int i = 0; i < count; ++i)
    {
        for (auto rest = graph.neighbors[vertices[i]] & graph.eligible; rest != 0; rest &= rest - 1)
        {
            neighbors[i] |= IndependentSetMask{1} << indices[std::countr_zero(rest)];
        }
    }

    using HalfMask = std::uint32_t;
    const int right_count = count / 2;
    const int left_count = count - right_count;
    const auto left_full = (HalfMask{1} << left_count) - 1;
    const auto right_full = (HalfMask{1} << right_count) - 1;
    std::array<HalfMask, kMaxIndependentSetVertices> left_neighbors{};
    std::array<HalfMask, kMaxIndependentSetVertices> right_neighbors{};
    for (int i = 0; i < count; ++i)
    {
        left_neighbors[i] = static_cast<HalfMask>(neighbors[i]) & left_full;
        right_neighbors[i] = static_cast<HalfMask>(neighbors[i] >> left_count);
    }

    // best_right[mask]は右側でmaskに含まれる頂点だけ使った場合の最適値。
    std::vector<Score> best_right(static_cast<std::size_t>(right_full) + 1);
    for (HalfMask mask = 1; mask <= right_full; ++mask)
    {
        const int v = left_count + std::countr_zero(mask);
        const auto without = mask & (mask - 1);
        const auto take = add_independent_set_scores(weights[vertices[v]], best_right[without & ~right_neighbors[v]]);
        best_right[mask] = std::max(best_right[without], take);
    }

    Score best = 0;
    HalfMask best_left = 0;
    HalfMask allowed_right = 0;
    const auto enumerate = [&](auto &&self, HalfMask candidates, HalfMask right, Score score, HalfMask subset) -> void
    {
        const auto total = add_independent_set_scores(score, best_right[right]);
        if (total > best)
        {
            best = total;
            best_left = subset;
            allowed_right = right;
        }
        while (candidates != 0)
        {
            const int u = std::countr_zero(candidates);
            const auto bit = HalfMask{1} << u;
            candidates ^= bit;
            const auto next_score = add_independent_set_scores(score, weights[vertices[u]]);
            self(self, candidates & ~left_neighbors[u], right & ~right_neighbors[u], next_score, subset | bit);
        }
    };
    enumerate(enumerate, left_full, right_full, base, 0);

    if constexpr (std::is_same_v<Score, std::uint64_t>)
    {
        if (best == kIndependentSetOverflow)
        {
            throw std::overflow_error("maximum_weight_independent_set: total weight does not fit int64_t");
        }
    }
    while (best_left != 0)
    {
        selected |= IndependentSetMask{1} << vertices[std::countr_zero(best_left)];
        best_left &= best_left - 1;
    }
    while (allowed_right != 0)
    {
        const int v = left_count + std::countr_zero(allowed_right);
        const auto without = allowed_right & (allowed_right - 1);
        if (best_right[allowed_right] != best_right[without])
        {
            selected |= IndependentSetMask{1} << vertices[v];
            allowed_right = without & ~right_neighbors[v];
        }
        else
        {
            allowed_right = without;
        }
    }
    MaximumWeightIndependentSetResult result{static_cast<std::int64_t>(best), {}};
    for (; selected != 0; selected &= selected - 1)
    {
        result.vertices.push_back(std::countr_zero(selected));
    }
    return result;
}
} // namespace detail

inline std::vector<int> maximum_independent_set(int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::make_independent_set_graph(n, edges);
    std::array<std::uint8_t, kMaxIndependentSetVertices> weights;
    weights.fill(1);
    return detail::solve_independent_set(graph, weights).vertices;
}

inline MaximumWeightIndependentSetResult maximum_weight_independent_set(
    std::span<const std::int64_t> weights, std::span<const std::pair<int, int>> edges)
{
    if (weights.size() > kMaxIndependentSetVertices)
    {
        throw std::length_error("maximum_weight_independent_set: vertex count must be in [0,46]");
    }
    auto graph = detail::make_independent_set_graph(static_cast<int>(weights.size()), edges);
    std::array<std::uint64_t, kMaxIndependentSetVertices> positive_weights{};
    for (std::size_t v = 0; v < weights.size(); ++v)
    {
        if (weights[v] <= 0)
        {
            graph.eligible &= ~(detail::IndependentSetMask{1} << v);
        }
        else
        {
            positive_weights[v] = static_cast<std::uint64_t>(weights[v]);
        }
    }
    return detail::solve_independent_set(graph, positive_weights);
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
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }

    const auto selected = maximum_independent_set(n, edges);
    verify_out << selected.size() << '\n';
    for (std::size_t i = 0; i < selected.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << selected[i];
    }
    verify_out << '\n';
}
