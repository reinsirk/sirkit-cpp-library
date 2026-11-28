#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/1602

// Library: Graph/Shortest_Nonzero_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考: Iwata–Yamaguchi, Finding a Shortest Non-zero Path in Group-Labeled Graphs, https://arxiv.org/abs/1906.04062
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_nonzero_paths.md
//
// API一覧
// GroupPathEdge<Label>{from,to,length,label}: 0始まりの無向辺。長さはuint32_t。from→toの群ラベルを渡す。
// shortest_nonzero_paths(n,edges,source,identity,combine,inverse):
//   群ラベルの積が単位元でない単純パスについて、始点から各頂点への最短長を返す。
//   逆向きの辺には逆元を使う。多重辺可。自己辺は使わない。
//   vector<optional<uint64_t>>を返す。該当するパスのない頂点と始点自身はnullopt。
//   群演算と比較がO(1)なら、O(n+m log(m+2)) 時間、O(n+m) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_NONZERO_PATHS_DEFINED
#define SIRKIT_SHORTEST_NONZERO_PATHS_DEFINED

template <typename Label>
struct GroupPathEdge
{
    int from;
    int to;
    std::uint32_t length;
    Label label;
};

template <typename Label, typename Combine, typename Inverse>
std::vector<std::optional<std::uint64_t>> shortest_nonzero_paths(
    int n, std::span<const GroupPathEdge<std::type_identity_t<Label>>> edges, int source, const Label &identity,
    Combine &&combine, Inverse &&inverse)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (static_cast<unsigned>(source) >= static_cast<unsigned>(n) || n < 0)
    {
        throw std::out_of_range("shortest_nonzero_paths: invalid source");
    }
#endif
    struct Arc
    {
        int end;
        std::size_t edge;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &edge = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<unsigned>(edge.from) >= static_cast<unsigned>(n) || static_cast<unsigned>(edge.to) >= static_cast<unsigned>(n))
        {
            throw std::out_of_range("shortest_nonzero_paths: invalid endpoint");
        }
#endif
        if (edge.from != edge.to)
        {
            graph[edge.from].push_back({edge.to, id});
            graph[edge.to].push_back({edge.from, id});
        }
    }

    constexpr auto kUnreachable = std::numeric_limits<std::uint64_t>::max();
    std::vector<std::uint64_t> distance(n, kUnreachable);
    std::vector<int> previous(n, -1), depth(n);
    std::vector<Label> labels(n, identity);
    {
        using Step = std::pair<std::uint64_t, int>;
        std::priority_queue<Step, std::vector<Step>, std::greater<Step>> pending;
        distance[source] = 0;
        pending.emplace(0, source);
        while (!pending.empty())
        {
            const auto [current, v] = pending.top();
            pending.pop();
            if (current != distance[v])
            {
                continue;
            }
            for (const auto &[w, id] : graph[v])
            {
                const auto &edge = edges[id];
                const auto candidate = current + edge.length;
                if (candidate < distance[w])
                {
                    distance[w] = candidate;
                    previous[w] = v;
                    depth[w] = depth[v] + 1;
                    if (v == edge.from)
                    {
                        labels[w] = std::invoke(combine, labels[v], edge.label);
                    }
                    else
                    {
                        labels[w] = std::invoke(combine, labels[v], std::invoke(inverse, edge.label));
                    }
                    pending.emplace(candidate, w);
                }
            }
        }
    }

    std::vector<std::optional<std::uint64_t>> result(n);
    for (int v = 0; v < n; ++v)
    {
        if (!(labels[v] == identity))
        {
            result[v] = distance[v];
        }
    }
    using Blossom = std::pair<std::uint64_t, std::size_t>;
    std::priority_queue<Blossom, std::vector<Blossom>, std::greater<Blossom>> pending;
    std::vector<bool> consistent(edges.size());
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &edge = edges[id];
        if (edge.from == edge.to || distance[edge.from] == kUnreachable)
        {
            continue;
        }
        consistent[id] = std::invoke(combine, labels[edge.from], edge.label) == labels[edge.to];
        if (!consistent[id])
        {
            pending.emplace(distance[edge.from] + distance[edge.to] + edge.length, id);
        }
    }

    // 縮約された成分の代表は、最短路木で最も浅い頂点。
    std::vector<int> representative(n), joined;
    std::iota(representative.begin(), representative.end(), 0);
    auto root = [&](int v)
    {
        while (representative[v] != v)
        {
            representative[v] = representative[representative[v]];
            v = representative[v];
        }
        return v;
    };
    while (!pending.empty())
    {
        const auto [height, id] = pending.top();
        pending.pop();
        auto from = root(edges[id].from);
        auto to = root(edges[id].to);
        while (from != to)
        {
            if (depth[from] < depth[to])
            {
                std::swap(from, to);
            }
            joined.push_back(from);
            from = root(previous[from]);
        }
        for (int v : joined)
        {
            representative[v] = from;
            const auto alternative = height - distance[v];
            if (!result[v])
            {
                result[v] = alternative;
            }
            for (const auto &[w, next] : graph[v])
            {
                if (consistent[next])
                {
                    pending.emplace(alternative + distance[w] + edges[next].length, next);
                }
            }
        }
        joined.clear();
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
#include <string>
#include <vector>

int main()
{
    int n{}, m{}, bits{};
    verify_in >> n >> m >> bits;
    std::vector<sirkit::GroupPathEdge<std::uint32_t>> edges(m);
    std::string binary;
    binary.reserve(bits);
    for (auto &[from, to, length, label] : edges)
    {
        verify_in >> from >> to >> length >> binary;
        --from;
        --to;
        for (const char bit : binary)
        {
            label = (label << 1) | static_cast<std::uint32_t>(bit - '0');
        }
    }
    const auto answer = sirkit::shortest_nonzero_paths(n, edges, n - 1, std::uint32_t{0}, std::bit_xor<>{}, std::identity{});
    for (int v = 0; v + 1 < n; ++v)
    {
        if (answer[v])
        {
            verify_out << *answer[v] << '\n';
        }
        else
        {
            verify_out << -1 << '\n';
        }
    }
}
