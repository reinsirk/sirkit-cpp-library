#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/2296

// Library: Graph/Incremental_Weighted_Forest.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Incremental_Weighted_Forest.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/incremental_weighted_forest.md
//
// API一覧
// IncrementalWeightedForest(n): [0,n)の孤立頂点から開始。O(n log(n+1))時間・領域。
// link(u,v,weight): 別々の木なら重みが非負の無向辺を加えtrue。同じ木なら変更せずfalse。
// 小さい木の頂点数をkとしてO(k log(n+1))時間、全追加の合計O(n log^2(n+1))。
// distance(u,v): 辺重みの和。非連結ならnullopt。O(log(n+1))。
// diameter(v): 木の直径をDiameter{first,second,distance}で返す。O(log(n+1))。
// connected(u,v)/component_size(v): 連結か/木の頂点数。各O(log(n+1))。size(): 全頂点数、O(1)。
// 成分内の辺の総重みはstd::uint64_tに収める。コピーは独立、ムーブ構築はO(1)。

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
#ifndef SIRKIT_INCREMENTAL_WEIGHTED_FOREST_DEFINED
#define SIRKIT_INCREMENTAL_WEIGHTED_FOREST_DEFINED

class IncrementalWeightedForest
{
public:
    struct Diameter
    {
        int first;
        int second;
        std::uint64_t distance;
    };

    explicit IncrementalWeightedForest(int n)
        : components_(vertex_count(n), -1), levels_(std::bit_width(static_cast<unsigned int>(n))),
          depth_(size()), ancestor_(size() * levels_), root_distance_(size()), total_weight_(size()),
          diameter_(size()), graph_(size())
    {
        for (int v = 0; v < n; ++v)
        {
            std::fill_n(ancestor_.begin() + static_cast<std::size_t>(v) * levels_, levels_, v);
            diameter_[v] = {v, v, 0};
        }
    }

    IncrementalWeightedForest(const IncrementalWeightedForest &) = default;
    IncrementalWeightedForest(IncrementalWeightedForest &&) noexcept = default;

    IncrementalWeightedForest &operator=(IncrementalWeightedForest other) noexcept
    {
        components_.swap(other.components_);
        std::swap(levels_, other.levels_);
        depth_.swap(other.depth_);
        ancestor_.swap(other.ancestor_);
        root_distance_.swap(other.root_distance_);
        total_weight_.swap(other.total_weight_);
        diameter_.swap(other.diameter_);
        graph_.swap(other.graph_);
        pending_.swap(other.pending_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return components_.size();
    }

    [[nodiscard]] bool connected(int u, int v) const
    {
        check_vertex(u);
        check_vertex(v);
        return leader(u) == leader(v);
    }

    [[nodiscard]] int component_size(int v) const
    {
        check_vertex(v);
        return -components_[leader(v)];
    }

    [[nodiscard]] std::optional<std::uint64_t> distance(int u, int v) const
    {
        if (!connected(u, v))
        {
            return std::nullopt;
        }
        return distance_connected(u, v);
    }

    [[nodiscard]] Diameter diameter(int v) const
    {
        check_vertex(v);
        return diameter_[leader(v)];
    }

    bool link(int u, int v, std::uint64_t weight)
    {
        check_vertex(u);
        check_vertex(v);
        int big = leader(u), small = leader(v);
        if (big == small)
        {
            return false;
        }
        if (components_[big] > components_[small])
        {
            std::swap(big, small);
            std::swap(u, v);
        }
        const auto sum = checked_sum(checked_sum(total_weight_[big], total_weight_[small]), weight);
        const auto [end_u, length_u] = farthest(u, diameter_[big]);
        const auto [end_v, length_v] = farthest(v, diameter_[small]);
        auto longest = diameter_[big];
        if (diameter_[small].distance > longest.distance)
        {
            longest = diameter_[small];
        }
        const auto cross = length_u + weight + length_v;
        if (cross > longest.distance)
        {
            longest = {end_u, end_v, cross};
        }

        // メモリの確保を先に済ませてから、小さい側の登録位置を変更する。
        reserve_edge(u);
        reserve_edge(v);
        pending_.reserve(static_cast<std::size_t>(-components_[small]));
        pending_.push_back({v, u, weight});
        for (std::size_t i = 0; i < pending_.size(); ++i)
        {
            const auto [here, parent, cost] = pending_[i];
            write_parent(here, parent, cost);
            for (const auto &[next, next_cost] : graph_[here])
            {
                if (next != parent)
                {
                    pending_.push_back({next, here, next_cost});
                }
            }
        }
        pending_.clear();

        graph_[u].push_back({v, weight});
        graph_[v].push_back({u, weight});
        components_[big] += components_[small];
        components_[small] = big;
        total_weight_[big] = sum;
        diameter_[big] = longest;
        return true;
    }

private:
    struct Edge
    {
        int to;
        std::uint64_t weight;
    };
    struct Visit
    {
        int vertex;
        int parent;
        std::uint64_t cost;
    };

    static std::size_t vertex_count(int n)
    {
        if (n < 0)
        {
            throw std::length_error("IncrementalWeightedForest: negative vertex count");
        }
        return static_cast<std::size_t>(n);
    }

    static std::uint64_t checked_sum(std::uint64_t a, std::uint64_t b)
    {
        if (b > std::numeric_limits<std::uint64_t>::max() - a)
        {
            throw std::overflow_error("IncrementalWeightedForest: component weight out of range");
        }
        return a + b;
    }

    void check_vertex(int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= size())
        {
            throw std::out_of_range("IncrementalWeightedForest: vertex");
        }
#else
        static_cast<void>(v);
#endif
    }

    int leader(int v) const noexcept
    {
        while (components_[v] >= 0)
        {
            v = components_[v];
        }
        return v;
    }

    int ancestor(int v, unsigned int level) const noexcept
    {
        return ancestor_[static_cast<std::size_t>(v) * levels_ + level];
    }

    int lca(int u, int v) const noexcept
    {
        if (depth_[u] < depth_[v])
        {
            std::swap(u, v);
        }
        auto difference = static_cast<unsigned int>(depth_[u] - depth_[v]);
        for (; difference != 0; difference &= difference - 1)
        {
            u = ancestor(u, static_cast<unsigned int>(std::countr_zero(difference)));
        }
        if (u == v)
        {
            return u;
        }
        for (auto level = levels_; level != 0; )
        {
            --level;
            const int a = ancestor(u, level), b = ancestor(v, level);
            if (a != b)
            {
                u = a;
                v = b;
            }
        }
        return ancestor(u, 0);
    }

    std::uint64_t distance_connected(int u, int v) const noexcept
    {
        const auto root = root_distance_[lca(u, v)];
        return (root_distance_[u] - root) + (root_distance_[v] - root);
    }

    std::pair<int, std::uint64_t> farthest(int v, Diameter diameter) const noexcept
    {
        const auto first = distance_connected(v, diameter.first);
        const auto second = distance_connected(v, diameter.second);
        return first >= second ? std::pair{diameter.first, first} : std::pair{diameter.second, second};
    }

    void reserve_edge(int v)
    {
        auto &list = graph_[v];
        if (list.size() == list.capacity())
        {
            list.reserve(2 * list.size() + 1);
        }
    }

    void write_parent(int v, int parent, std::uint64_t weight) noexcept
    {
        depth_[v] = depth_[parent] + 1;
        root_distance_[v] = root_distance_[parent] + weight;
        auto *row = ancestor_.data() + static_cast<std::size_t>(v) * levels_;
        row[0] = parent;
        for (unsigned int level = 1; level < levels_; ++level)
        {
            row[level] = ancestor(row[level - 1], level - 1);
        }
    }

    std::vector<int> components_;
    unsigned int levels_;
    std::vector<int> depth_, ancestor_;
    std::vector<std::uint64_t> root_distance_, total_weight_;
    std::vector<Diameter> diameter_;
    std::vector<std::vector<Edge>> graph_;
    std::vector<Visit> pending_;
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

int main()
{
    int n{}, x{}, q{};
    verify_in >> n >> x >> q;
    sirkit::IncrementalWeightedForest forest(n);
    for (int i = 0; i < q; ++i)
    {
        int type{}, v{};
        verify_in >> type >> v;
        if (type == 1)
        {
            std::uint64_t weight{};
            verify_in >> weight;
            forest.link(x, v, weight);
        }
        else if (type == 2)
        {
            int u{};
            verify_in >> u;
            if (const auto distance = forest.distance(u, v))
            {
                verify_out << *distance << '\n';
                x = static_cast<int>((x + *distance) % n);
            }
            else
            {
                verify_out << -1 << '\n';
            }
        }
        else if (type == 3)
        {
            verify_out << forest.diameter(v).distance << '\n';
        }
        else
        {
            x = (x + v) % n;
        }
    }
}
