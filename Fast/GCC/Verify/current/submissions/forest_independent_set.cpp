#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/dp/tasks/dp_p

// Library: Graph/Forest_Independent_Set.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Forest_Independent_Set.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/forest_independent_set.md
//
// API一覧
// ForestIndependentSet(n, edges): 0 始まりの無向森を作る。閉路、自己辺、多重辺は例外。
// count<T>(weights={}): 両端を同時に選ぶ辺がない頂点集合の個数。重みがあれば状態ごとの積を総和。
//   weights[v] = ForestIndependentSetWeight<T>{excluded,included}。空の森は1通り。
// count_by_vertex<T>(weights={}): 各頂点を選ばない・選ぶ場合の、森全体の重み付き個数を返す。
//   T は 0,1 から構築でき、可換な加算・乗算ができる型。除算を使わない。
// maximum_weight<T>(weights): 頂点重みの和を最大化する集合を {weight, vertices} で返す。空集合も可。
//   T は 0、加算、大小比較が必要。演算結果が T に収まること。すべて O(n+1) 時間・領域、非再帰。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FOREST_INDEPENDENT_SET_DEFINED
#define SIRKIT_FOREST_INDEPENDENT_SET_DEFINED

template <class T>
struct ForestIndependentSetWeight
{
    T excluded;
    T included;
};

template <class T>
struct ForestIndependentSetOptimum
{
    T weight;
    std::vector<int> vertices;
};

class ForestIndependentSet
{
public:
    ForestIndependentSet(int n, std::span<const std::pair<int, int>> edges)
    {
        if (n < 0)
        {
            throw std::length_error("ForestIndependentSet: negative vertex count");
        }
        if (edges.size() > static_cast<std::size_t>(n))
        {
            throw std::invalid_argument("ForestIndependentSet: too many edges");
        }
        std::vector<std::vector<int>> graph(n);
        for (const auto &[u, v] : edges)
        {
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("ForestIndependentSet: edge endpoint");
            }
            graph[u].push_back(v);
            graph[v].push_back(u);
        }

        constexpr int kUnvisited = -2;
        parent_.assign(n, kUnvisited);
        order_.reserve(n);
        children_.resize(n);
        for (int root = 0; root < n; ++root)
        {
            if (parent_[root] != kUnvisited)
            {
                continue;
            }
            parent_[root] = -1;
            roots_.push_back(root);
            const auto start = order_.size();
            order_.push_back(root);
            for (std::size_t i = start; i < order_.size(); ++i)
            {
                const int v = order_[i];
                for (const int w : graph[v])
                {
                    if (w == parent_[v])
                    {
                        continue;
                    }
                    if (parent_[w] != kUnvisited)
                    {
                        throw std::invalid_argument("ForestIndependentSet: cycle");
                    }
                    parent_[w] = v;
                    children_[v].push_back(w);
                    order_.push_back(w);
                }
            }
        }
    }

    template <class T>
    [[nodiscard]] T count(std::span<const ForestIndependentSetWeight<T>> weights = {}) const
    {
        const auto subtree = count_subtrees(weights);
        T total{1};
        for (const int root : roots_)
        {
            total = total * (subtree[root].excluded + subtree[root].included);
        }
        return total;
    }

    template <class T>
    [[nodiscard]] std::vector<ForestIndependentSetWeight<T>>
    count_by_vertex(std::span<const ForestIndependentSetWeight<T>> weights = {}) const
    {
        using Pair = ForestIndependentSetWeight<T>;
        const auto subtree = count_subtrees(weights);
        const Pair one{T{1}, T{1}};
        std::vector<Pair> result(order_.size(), one), prefixes(order_.size(), one);
        T previous{1};
        for (const int root : roots_)
        {
            result[root] = {previous, previous};
            previous = previous * (subtree[root].excluded + subtree[root].included);
        }
        T after{1};
        for (auto it = roots_.rbegin(); it != roots_.rend(); ++it)
        {
            const int root = *it;
            result[root] = {result[root].excluded * after, result[root].included * after};
            after = after * (subtree[root].excluded + subtree[root].included);
        }

        for (const int v : order_)
        {
            const Pair vertex = weights.empty() ? one : weights[v];
            Pair before{result[v].excluded * vertex.excluded, result[v].included * vertex.included};
            for (const int w : children_[v])
            {
                prefixes[w] = before;
                include_child(before, subtree[w]);
            }
            result[v] = before;

            Pair suffix = one;
            for (auto it = children_[v].rbegin(); it != children_[v].rend(); ++it)
            {
                const int w = *it;
                const T without_v = prefixes[w].excluded * suffix.excluded;
                const T with_v = prefixes[w].included * suffix.included;
                result[w] = {without_v + with_v, without_v};
                include_child(suffix, subtree[w]);
            }
        }
        return result;
    }

    template <class T>
    [[nodiscard]] ForestIndependentSetOptimum<T> maximum_weight(std::span<const T> weights) const
    {
        if (weights.size() != order_.size())
        {
            throw std::invalid_argument("ForestIndependentSet: wrong number of vertex weights");
        }
        using Pair = ForestIndependentSetWeight<T>;
        std::vector<Pair> subtree(order_.size(), {T{0}, T{0}});
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            subtree[v].included = weights[v];
            for (const int w : children_[v])
            {
                subtree[v].excluded = subtree[v].excluded + std::max(subtree[w].excluded, subtree[w].included);
                subtree[v].included = subtree[v].included + subtree[w].excluded;
            }
        }

        ForestIndependentSetOptimum<T> result{T{0}, {}};
        std::vector<bool> picked(order_.size());
        for (const int v : order_)
        {
            const int parent = parent_[v];
            if ((parent == -1 || !picked[parent]) && subtree[v].excluded < subtree[v].included)
            {
                picked[v] = true;
                result.weight = result.weight + weights[v];
                result.vertices.push_back(v);
            }
        }
        return result;
    }

private:
    template <class T>
    static void include_child(ForestIndependentSetWeight<T> &parent, const ForestIndependentSetWeight<T> &child)
    {
        parent.excluded = parent.excluded * (child.excluded + child.included);
        parent.included = parent.included * child.excluded;
    }

    template <class T>
    std::vector<ForestIndependentSetWeight<T>> count_subtrees(std::span<const ForestIndependentSetWeight<T>> weights) const
    {
        using Pair = ForestIndependentSetWeight<T>;
        if (!weights.empty() && weights.size() != order_.size())
        {
            throw std::invalid_argument("ForestIndependentSet: wrong number of counting weights");
        }
        const Pair one{T{1}, T{1}};
        std::vector<Pair> subtree(order_.size(), one);
        for (auto it = order_.rbegin(); it != order_.rend(); ++it)
        {
            const int v = *it;
            auto &current = subtree[v];
            if (!weights.empty())
            {
                current = weights[v];
            }
            for (const int w : children_[v])
            {
                include_child(current, subtree[w]);
            }
        }
        return subtree;
    }

    std::vector<int> parent_, order_, roots_;
    std::vector<std::vector<int>> children_;
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
#include <utility>
#include <vector>

namespace
{
struct Count
{
    static constexpr std::uint64_t kMod = 1000000007;
    std::uint64_t value;
    explicit Count(std::uint64_t count) : value(count % kMod)
    {
    }
    friend Count operator+(Count a, Count b)
    {
        return Count(a.value + b.value);
    }
    friend Count operator*(Count a, Count b)
    {
        return Count(a.value * b.value);
    }
};
} // namespace

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
    verify_out << sirkit::ForestIndependentSet(n, edges).count<Count>().value << '\n';
}
