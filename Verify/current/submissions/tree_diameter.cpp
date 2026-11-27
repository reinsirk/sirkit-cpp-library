#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/tree_diameter
// Submission: https://judge.yosupo.jp/submission/419659

// Library: Graph/Tree_Diameter.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（2回の木の走査）: https://ei1333.github.io/luzhiled/snippets/tree/tree-diameter.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_diameter.md
//
// API一覧
// TreeDiameterEdge {from,to,weight}: 0始まりの端点と非負のstd::int64_t重みを持つ無向辺。
// TreeDiameterResult {length,path}: 直径の長さと、直径をなすパスの頂点列。
// tree_diameter(n,edges): n頂点の無向木の直径とパスを返す。O(n)時間・領域。
// n=0では{0,{}}、n=1や全辺の重みが0なら{0,{0}}。同じ長さのパスが複数あれば1つを返す。
// 長さがstd::int64_tの範囲を超えるとstd::overflow_error。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_DIAMETER_DEFINED
#define SIRKIT_TREE_DIAMETER_DEFINED

struct TreeDiameterEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct TreeDiameterResult
{
    std::int64_t length;
    std::vector<int> path;
};

inline TreeDiameterResult tree_diameter(int n, std::span<const TreeDiameterEdge> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("tree_diameter: negative vertex count");
    }
#endif
    const auto vertices = static_cast<std::size_t>(n);
#ifndef SKIP_RUNTIME_CHECKS
    if (edges.size() != (vertices == 0 ? 0 : vertices - 1))
    {
        throw std::invalid_argument("tree_diameter: wrong edge count");
    }
#endif
    if (vertices == 0)
    {
        return {0, {}};
    }
    if (vertices == 1)
    {
        return {0, {0}};
    }

    std::vector<std::size_t> offset(vertices + 1);
    for (const auto &[from, to, weight] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(from) >= vertices || static_cast<std::size_t>(to) >= vertices)
        {
            throw std::out_of_range("tree_diameter: invalid endpoint");
        }
        if (from == to || weight < 0)
        {
            throw std::invalid_argument("tree_diameter: loop or negative weight");
        }
#endif
        ++offset[from];
        ++offset[to];
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());

    struct Arc
    {
        int to;
        std::int64_t weight;
    };
    std::vector<Arc> arcs(2 * edges.size());
    // 各頂点の区間を後ろから埋めると、offsetをそのまま区間の先頭に使える。
    for (const auto &[from, to, weight] : edges)
    {
        arcs[--offset[from]] = {to, weight};
        arcs[--offset[to]] = {from, weight};
    }

    std::vector<int> parent(vertices, -1);
    std::vector<std::int64_t> distance(vertices);
    std::vector<int> order;
    order.reserve(vertices);

    const auto farthest_from = [&](int root, [[maybe_unused]] bool check_tree)
    {
        int farthest = root;
        order.clear();
        order.push_back(root);
        parent[root] = root;
        distance[root] = 0;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int from = order[i];
            for (std::size_t j = offset[from]; j < offset[from + 1]; ++j)
            {
                const auto &[to, weight] = arcs[j];
                if (to == parent[from])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (check_tree && parent[to] != -1)
                {
                    throw std::invalid_argument("tree_diameter: cycle");
                }
#endif
                if (weight > std::numeric_limits<std::int64_t>::max() - distance[from])
                {
                    throw std::overflow_error("tree_diameter: path length overflow");
                }

                parent[to] = from;
                distance[to] = distance[from] + weight;
                if (distance[to] > distance[farthest])
                {
                    farthest = to;
                }
                order.push_back(to);
            }
        }
        return farthest;
    };

    const int start = farthest_from(0, true);
#ifndef SKIP_RUNTIME_CHECKS
    if (order.size() != vertices)
    {
        throw std::invalid_argument("tree_diameter: disconnected");
    }
#endif
    const int end = farthest_from(start, false);

    TreeDiameterResult result{distance[end], {}};
    for (int vertex = end; vertex != start; vertex = parent[vertex])
    {
        result.path.push_back(vertex);
    }
    result.path.push_back(start);
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

#include <cstddef>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<TreeDiameterEdge> edges(n - 1);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto result = tree_diameter(n, edges);
    verify_out << result.length << ' ' << result.path.size() << '\n';
    for (std::size_t i = 0; i < result.path.size(); ++i)
    {
        verify_out << result.path[i] << (i + 1 == result.path.size() ? '\n' : ' ');
    }
}
