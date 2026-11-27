#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/st_numbering

// Library: Graph/ST_Numbering.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/ST_Numbering.cpp
// 参考（Tarjanの符号挿入法）: https://doi.org/10.3233/FI-1986-9105
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/st_numbering.md
//
// API一覧
// st_numbering(n,source,sink,edges): 無向グラフのST番号付けを返す。存在しなければstd::nullopt。
// rank[v]が頂点vの0始まりの順位。sourceは0、sinkはn-1。各辺を小さい順位から大きい順位へ向けると
// 各頂点がsourceからsinkへ向かうパスに載る。sourceとsinkは非隣接でも可。
// 頂点番号は[0,n)。多重辺可、自己辺は無視する。空グラフ・同一端点には番号付けなし。
// 反復DFSと連結リストでO(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ST_NUMBERING_DEFINED
#define SIRKIT_ST_NUMBERING_DEFINED

namespace detail
{
struct StNumberingAdjacency
{
    std::vector<std::size_t> offsets;
    std::vector<int> neighbors;
};

inline StNumberingAdjacency make_st_numbering_graph(int n, std::span<const std::pair<int, int>> edges)
{
    StNumberingAdjacency graph{std::vector<std::size_t>(static_cast<std::size_t>(n) + 1), {}};
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("st_numbering: edge endpoint out of range");
        }
#endif
        if (u != v)
        {
            ++graph.offsets[u + 1];
            ++graph.offsets[v + 1];
        }
    }
    for (int v = 0; v < n; ++v)
    {
        graph.offsets[v + 1] += graph.offsets[v];
    }
    graph.neighbors.resize(graph.offsets.back());
    auto cursor = graph.offsets;
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            graph.neighbors[cursor[u]++] = v;
            graph.neighbors[cursor[v]++] = u;
        }
    }
    return graph;
}

inline std::vector<int> restore_st_ranks(
    const std::vector<int> &preorder, const std::vector<int> &parent, const std::vector<int> &low)
{
    const auto n = preorder.size();
    const int source = preorder[0], sink = preorder[1];
    std::vector<int> link_before(n, -1), link_after(n, -1);
    std::vector<unsigned char> positive(n);
    link_before[sink] = source;
    link_after[source] = sink;
    for (std::size_t i = 2; i < n; ++i)
    {
        const int v = preorder[i], p = parent[v];
        const bool insert_after = positive[preorder[low[v]]] != 0;
        const int before = insert_after ? p : link_before[p];
        const int after = insert_after ? link_after[p] : p;
        link_after[before] = v;
        link_before[after] = v;
        link_before[v] = before;
        link_after[v] = after;
        positive[p] = !insert_after;
    }

    std::vector<int> ranks(n);
    int rank = 0;
    for (int v = source; v >= 0; v = link_after[v])
    {
        ranks[v] = rank++;
    }
    return ranks;
}
} // namespace detail

inline std::optional<std::vector<int>> st_numbering(
    int n, int source, int sink, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("st_numbering: negative vertex count");
    }
    if (n == 0)
    {
        return std::nullopt;
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || sink < 0 || sink >= n)
    {
        throw std::out_of_range("st_numbering: source or sink out of range");
    }
#endif
    if (source == sink || edges.empty())
    {
        return std::nullopt;
    }
    const auto graph = detail::make_st_numbering_graph(n, edges);
    if (graph.offsets[source] == graph.offsets[source + 1])
    {
        return std::nullopt;
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
    };
    std::vector<Frame> pending;
    std::vector<int> parent(n), number(n, -1), low(n), preorder;
    preorder.reserve(n);
    const auto enter = [&](int v, int p)
    {
        parent[v] = p;
        number[v] = low[v] = static_cast<int>(preorder.size());
        preorder.push_back(v);
        pending.push_back({v, graph.offsets[v]});
    };

    // 仮のsource-sink辺を最初に通ったとみなし、source以外をsinkから探索する。
    number[source] = 0;
    preorder.push_back(source);
    enter(sink, source);
    while (!pending.empty())
    {
        auto &frame = pending.back();
        const int v = frame.vertex;
        if (frame.next != graph.offsets[v + 1])
        {
            const int to = graph.neighbors[frame.next++];
            if (number[to] < 0)
            {
                enter(to, v);
            }
            else
            {
                // 親への辺が混ざっても、下の「親より厳密に小さい」条件には影響しない。
                low[v] = std::min(low[v], number[to]);
            }
            continue;
        }

        pending.pop_back();
        if (v != sink)
        {
            const int p = parent[v];
            if (low[v] >= number[p])
            {
                return std::nullopt;
            }
            low[p] = std::min(low[p], low[v]);
        }
    }
    if (preorder.size() != static_cast<std::size_t>(n))
    {
        return std::nullopt;
    }
    return detail::restore_st_ranks(preorder, parent, low);
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


#include <utility>
#include <vector>

int main()
{
    int tests;
    verify_in >> tests;
    std::vector<std::pair<int, int>> edges;
    while (tests-- != 0)
    {
        int n, m, source, sink;
        verify_in >> n >> m >> source >> sink;
        edges.resize(m);
        for (auto &[u, v] : edges)
        {
            verify_in >> u >> v;
        }
        const auto ranks = st_numbering(n, source, sink, edges);
        if (!ranks)
        {
            verify_out << "No\n";
            continue;
        }
        verify_out << "Yes\n";
        for (int v = 0; v < n; ++v)
        {
            if (v != 0)
            {
                verify_out << ' ';
            }
            verify_out << (*ranks)[v];
        }
        verify_out << '\n';
    }
}
