#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc197/tasks/abc197_f

// Library: Graph/Palindromic_Shortest_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Palindromic_Shortest_Paths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromic_shortest_paths.md
//
// API一覧
// PalindromicPathEdge{from,to,label}: 0始まりの端点と uint8_t のラベル。
// PalindromicShortestPaths(n,edges,undirected=false): ラベル列が回文になる最短歩道を全頂点対で前計算。
//   自己辺・多重辺可。頂点や辺は繰り返し通れる。辺 ID は入力順。
//   ラベルの種類を σ、ラベル c の辺数を m_c として時間 O((σ+1)n²+Σm_c²)、領域 O(n²+m)。
// size(): 頂点数。distance(from,to): 最短歩道の長さ、存在しなければ -1。ともに O(1)。
// path(from,to): 頂点列 vertices と辺 ID 列 edges を含む Walk。存在しなければ nullopt。長さ ℓ で O(ℓ+1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMIC_SHORTEST_PATHS_DEFINED
#define SIRKIT_PALINDROMIC_SHORTEST_PATHS_DEFINED

struct PalindromicPathEdge
{
    int from;
    int to;
    std::uint8_t label;
};

class PalindromicShortestPaths
{
public:
    struct Walk
    {
        std::vector<int> vertices;
        std::vector<std::size_t> edges;
    };

    explicit PalindromicShortestPaths(int n, std::span<const PalindromicPathEdge> edges, bool undirected = false)
        : n_(n)
    {
        if (n < 0)
        {
            throw std::length_error("PalindromicShortestPaths: negative vertex count");
        }
        const auto count = static_cast<std::size_t>(n);
        if (count != 0 && count > states_.max_size() / count)
        {
            throw std::length_error("PalindromicShortestPaths: too many vertex pairs");
        }
        states_.resize(count * count);
        std::vector<std::size_t> pending;
        pending.reserve(count);
        for (int u = 0; u < n; ++u)
        {
            const auto where = index(u, u);
            states_[where].distance = 0;
            pending.push_back(where);
        }

        std::array<int, 256> label_ids;
        label_ids.fill(-1);
        int alphabet = 0;
        for (const auto &[from, to, label] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            check_vertex(from);
            check_vertex(to);
#endif
            if (label_ids[label] == -1)
            {
                label_ids[label] = alphabet++;
            }
        }

        struct Arc
        {
            int vertex;
            std::size_t id;
        };
        std::vector<std::vector<Arc>> incoming(count * alphabet), outgoing(count * alphabet);
        auto bucket = [alphabet](int vertex, int label)
        {
            return static_cast<std::size_t>(vertex) * alphabet + label;
        };
        auto add_arc = [&](int from, int to, int label, std::size_t id)
        {
            outgoing[bucket(from, label)].push_back({to, id});
            incoming[bucket(to, label)].push_back({from, id});
            const auto where = index(from, to);
            if (states_[where].distance == -1)
            {
                states_[where] = State{1, 0, id, 0};
                pending.push_back(where);
            }
        };
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto [u, v, label] = edges[id];
            add_arc(u, v, label_ids[label], id);
            if (undirected && u != v)
            {
                add_arc(v, u, label_ids[label], id);
            }
        }

        // 長さ0と1の中心を入れてから、同じラベルの辺を両端に足す。
        for (std::size_t cursor = 0; cursor < pending.size() && pending.size() != states_.size(); ++cursor)
        {
            const auto inside = pending[cursor];
            const int u = static_cast<int>(inside / count);
            const int v = static_cast<int>(inside % count);
            const std::int64_t length = states_[inside].distance + 2;
            for (int label = 0; label < alphabet; ++label)
            {
                for (const auto &[from, left_id] : incoming[bucket(u, label)])
                {
                    for (const auto &[to, right_id] : outgoing[bucket(v, label)])
                    {
                        const auto outside = index(from, to);
                        if (states_[outside].distance == -1)
                        {
                            states_[outside] = State{length, inside, left_id, right_id};
                            pending.push_back(outside);
                        }
                    }
                }
            }
        }
    }

    int size() const noexcept
    {
        return n_;
    }

    std::int64_t distance(int from, int to) const
    {
        return states_[checked_index(from, to)].distance;
    }

    std::optional<Walk> path(int from, int to) const
    {
        auto where = checked_index(from, to);
        const std::int64_t distance = states_[where].distance;
        if (distance == -1)
        {
            return std::nullopt;
        }
        const auto length = static_cast<std::size_t>(distance);
        Walk result{std::vector<int>(length + 1), std::vector<std::size_t>(length)};
        std::size_t left = 0;
        std::size_t right = length;
        const auto count = static_cast<std::size_t>(n_);
        for (;;)
        {
            result.vertices[left] = static_cast<int>(where / count);
            result.vertices[right] = static_cast<int>(where % count);
            if (left == right)
            {
                break;
            }
            const auto &state = states_[where];
            result.edges[left] = state.left_edge;
            if (left + 1 == right)
            {
                break;
            }
            result.edges[right - 1] = state.right_edge;
            where = state.inside;
            ++left;
            --right;
        }
        return result;
    }

private:
    struct State
    {
        std::int64_t distance = -1;
        std::size_t inside = 0;
        std::size_t left_edge = 0;
        std::size_t right_edge = 0;
    };

    int n_;
    std::vector<State> states_;

#ifndef SKIP_RUNTIME_CHECKS
    void check_vertex(int v) const
    {
        if (v < 0 || v >= n_)
        {
            throw std::out_of_range("PalindromicShortestPaths: invalid vertex");
        }
    }
#endif

    std::size_t index(int from, int to) const noexcept
    {
        return static_cast<std::size_t>(from) * n_ + to;
    }

    std::size_t checked_index(int from, int to) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        check_vertex(from);
        check_vertex(to);
#endif
        return index(from, to);
    }
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
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::PalindromicPathEdge> edges(m);
    for (auto &[from, to, label] : edges)
    {
        char ch{};
        verify_in >> from >> to >> ch;
        --from;
        --to;
        label = static_cast<std::uint8_t>(ch);
    }
    const sirkit::PalindromicShortestPaths paths(n, edges, true);
    verify_out << paths.distance(0, n - 1) << '\n';
}
