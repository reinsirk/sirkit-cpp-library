#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/counting_c4

// Library: Graph/Count_Four_Cycles.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Count_Four_Cycles.cpp
// 参考: Chiba & Nishizeki, Arboricity and Subgraph Listing Algorithms, https://doi.org/10.1137/0214017
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/count_four_cycles.md
//
// API一覧
// count_four_cycles_per_edge(n,edges): 各入力辺を含む長さ4の単純閉路の個数を入力順に返す。
// 頂点は0..n-1。無向多重辺は別々に数え、自己辺の答えは0。閉路の始点や向きを数え分けない。
// 入力辺数m≤4,000,000、自己辺と並行辺を除いた辺数dとしてO(n+m log(m+1)+d√d)時間・O(n+m)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_COUNT_FOUR_CYCLES_DEFINED
#define SIRKIT_COUNT_FOUR_CYCLES_DEFINED
namespace detail
{
inline void CheckFourCycleEdgeCount(std::size_t count)
{
    constexpr std::size_t kMaxEdges = 4'000'000;
    if (count > kMaxEdges)
    {
        // 固定した1辺に対し、残り3辺の組合せの上界をuint64_tに収める。
        throw std::length_error("count_four_cycles_per_edge: too many edges");
    }
}
}

inline std::vector<std::uint64_t> count_four_cycles_per_edge(int n, std::span<const std::pair<int, int>> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("count_four_cycles_per_edge: negative vertex count");
    }
#endif
    detail::CheckFourCycleEdgeCount(edges.size());
    using Endpoints = std::pair<int, int>;
    std::vector<std::pair<Endpoints, std::size_t>> sorted;
    sorted.reserve(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        auto [u, v] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("count_four_cycles_per_edge: invalid endpoint");
        }
#endif
        if (u != v)
        {
            sorted.emplace_back(std::minmax(u, v), i);
        }
    }
    std::sort(sorted.begin(), sorted.end());
    struct Group
    {
        Endpoints endpoints;
        std::uint64_t multiplicity = 0;
        std::uint64_t answer = 0;
    };
    std::vector<Group> groups;
    std::vector<std::size_t> assignment(edges.size(), edges.size());
    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    for (const auto &[endpoints, id] : sorted)
    {
        if (groups.empty() || groups.back().endpoints != endpoints)
        {
            groups.push_back({endpoints});
            ++offset[endpoints.first + 1];
            ++offset[endpoints.second + 1];
        }
        ++groups.back().multiplicity;
        assignment[id] = groups.size() - 1;
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    struct Arc
    {
        int to;
        std::size_t group;
    };
    std::vector<Arc> arcs(offset.back());
    auto next = offset;
    for (std::size_t id = 0; id < groups.size(); ++id)
    {
        const auto [u, v] = groups[id].endpoints;
        arcs[next[u]++] = {v, id};
        arcs[next[v]++] = {u, id};
    }
    const auto vertex_key = [&](int v)
    {
        return std::pair{offset[v + 1] - offset[v], v};
    };
    std::vector<std::uint64_t> paths(n);
    std::vector<int> touched;
    for (int u = 0; u < n; ++u)
    {
        const auto key = vertex_key(u);
        // 各四角形は(次数, 頂点番号)が最大の頂点uで処理する。
        const auto visit_wedges = [&](auto action)
        {
            for (auto i = offset[u]; i < offset[u + 1]; ++i)
            {
                const auto [middle, first] = arcs[i];
                if (vertex_key(middle) >= key)
                {
                    continue;
                }
                for (auto j = offset[middle]; j < offset[middle + 1]; ++j)
                {
                    const auto [opposite, second] = arcs[j];
                    if (vertex_key(opposite) < key)
                    {
                        action(opposite, groups[first], groups[second]);
                    }
                }
            }
        };
        visit_wedges([&](int v, const Group &first, const Group &second)
                     {
                         if (paths[v] == 0)
                         {
                             touched.push_back(v);
                         }
                         paths[v] += first.multiplicity * second.multiplicity;
                     });
        visit_wedges([&](int v, Group &first, Group &second)
                     {
                         const auto alternatives = paths[v] - first.multiplicity * second.multiplicity;
                         first.answer += second.multiplicity * alternatives;
                         second.answer += first.multiplicity * alternatives;
                     });
        for (const int v : touched)
        {
            paths[v] = 0;
        }
        touched.clear();
    }
    std::vector<std::uint64_t> answer(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (assignment[i] != edges.size())
        {
            answer[i] = groups[assignment[i]].answer;
        }
    }
    return answer;
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
    int n;
    std::size_t m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }
    const auto count = count_four_cycles_per_edge(n, edges);
    for (std::size_t i = 0; i < m; ++i)
    {
        verify_out << count[i] << (i + 1 == m ? '\n' : ' ');
    }
}
