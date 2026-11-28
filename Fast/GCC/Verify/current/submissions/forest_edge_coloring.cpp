#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc146/tasks/abc146_d

// Library: Graph/Forest_Coloring.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Forest_Coloring.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forest_coloring.md
//
// API一覧
// ForestColoring(n,edges): 0始まりの無向森。閉路、多重辺、自己辺があれば例外。構築 O(n+m+1) 時間・領域。
// edge_coloring(): 接する辺はすべて異なる色。最小色数は最大次数Δ。
// distance_two_coloring(): 距離が1または2の頂点を異なる色にする。空でない森の最小色数はΔ+1。
//   ForestColoringResult{color_count,colors}。色は0始まり。辺は入力順、頂点は番号順。空なら0色。
// count_distance_two<T>(palette): 使用可能な色をpalette個とした頂点彩色の個数。空の森は1通り。
//   T{0},T{1}とpalette以下の整数から構築でき、乗算可能。各操作 O(n+1) 時間。数え上げの追加領域は O(1)。非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FOREST_COLORING_DEFINED
#define SIRKIT_FOREST_COLORING_DEFINED

struct ForestColoringResult
{
    int color_count;
    std::vector<int> colors;
};

class ForestColoring
{
public:
    ForestColoring(int n, std::span<const std::pair<int, int>> edges) : edge_count_(edges.size())
    {
        if (n < 0)
        {
            throw std::length_error("ForestColoring: negative vertex count");
        }
        adjacent_.resize(n);
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [u, v] = edges[i];
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("ForestColoring: edge endpoint");
            }
            adjacent_[u].push_back({v, i});
            adjacent_[v].push_back({u, i});
        }

        constexpr int kUnvisited = -2;
        parent_.assign(n, kUnvisited);
        parent_edge_.assign(n, edge_count_);
        order_.reserve(n);
        for (int root = 0; root < n; ++root)
        {
            if (parent_[root] != kUnvisited)
            {
                continue;
            }
            parent_[root] = -1;
            const auto first = order_.size();
            order_.push_back(root);
            for (std::size_t i = first; i < order_.size(); ++i)
            {
                const int v = order_[i];
                for (const auto &[next, edge] : adjacent_[v])
                {
                    if (edge == parent_edge_[v])
                    {
                        continue;
                    }
                    if (parent_[next] != kUnvisited)
                    {
                        throw std::invalid_argument("ForestColoring: graph contains a cycle");
                    }
                    parent_[next] = v;
                    parent_edge_[next] = edge;
                    order_.push_back(next);
                }
                max_degree_ = std::max(max_degree_, static_cast<int>(adjacent_[v].size()));
            }
        }
    }

    [[nodiscard]] ForestColoringResult edge_coloring() const
    {
        if (order_.empty())
        {
            return {0, {}};
        }
        ForestColoringResult result{max_degree_, std::vector<int>(edge_count_)};
        for (const int v : order_)
        {
            const auto upward = parent_edge_[v];
            const int forbidden = upward == edge_count_ ? -1 : result.colors[upward];
            int next_color = 0;
            for (const auto &[child, edge] : adjacent_[v])
            {
                if (edge != upward)
                {
                    if (next_color == forbidden)
                    {
                        ++next_color;
                    }
                    result.colors[edge] = next_color++;
                }
            }
        }
        return result;
    }

    [[nodiscard]] ForestColoringResult distance_two_coloring() const
    {
        if (order_.empty())
        {
            return {0, {}};
        }
        ForestColoringResult result{max_degree_ + 1, std::vector<int>(adjacent_.size())};
        for (const int v : order_)
        {
            const int parent = parent_[v];
            const int parent_color = parent == -1 ? -1 : result.colors[parent];
            int next_color = 0;
            for (const auto &[child, edge] : adjacent_[v])
            {
                if (child != parent)
                {
                    while (next_color == result.colors[v] || next_color == parent_color)
                    {
                        ++next_color;
                    }
                    result.colors[child] = next_color++;
                }
            }
        }
        return result;
    }

    template <class T>
    [[nodiscard]] T count_distance_two(std::uint64_t palette) const
    {
        if (order_.empty())
        {
            return T{1};
        }
        if (palette < static_cast<std::uint64_t>(max_degree_) + 1)
        {
            return T{0};
        }

        T total{1};
        for (const int v : order_)
        {
            const bool root = parent_[v] == -1;
            if (root)
            {
                total = total * T(palette);
            }
            const auto children = adjacent_[v].size() - static_cast<std::size_t>(!root);
            const auto available = palette - (root ? 1U : 2U);
            for (std::size_t position = 0; position < children; ++position)
            {
                total = total * T(available - position);
            }
        }
        return total;
    }

private:
    std::size_t edge_count_;
    int max_degree_ = 0;
    std::vector<std::vector<std::pair<int, std::size_t>>> adjacent_;
    std::vector<int> parent_, order_;
    std::vector<std::size_t> parent_edge_;
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


#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto answer = sirkit::ForestColoring(n, edges).edge_coloring();
    verify_out << answer.color_count << '\n';
    for (const int color : answer.colors)
    {
        verify_out << color + 1 << '\n';
    }
}
