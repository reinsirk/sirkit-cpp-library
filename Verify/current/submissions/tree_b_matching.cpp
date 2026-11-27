#define SKIP_RUNTIME_CHECKS
// Verify: https://atcoder.jp/contests/abc259/tasks/abc259_f
// Submission: https://atcoder.jp/contests/abc259/submissions/81108588

// Library: Graph/Tree_B_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 参考（親辺を残す木DP）: https://atcoder.jp/contests/abc259/editorial/4296
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_b_matching.md
//
// API一覧
// maximum_weight_tree_b_matching(max_degree,edges): 各頂点の選択次数をmax_degree以下にして、辺重みの和を最大化。
// TreeBMatchingEdge{from,to,weight}: 頂点は0始まり、辺の重みは符号付き64ビット。
// TreeBMatchingResult{weight,edge_ids}: 最大値と、選んだ辺の入力添字。辺を選ばない解も許す。
// 非負の次数上限と無向木を渡す。空木・1頂点では重み0と空の辺列。最適値のint64_t超過はoverflow_error。
// 深い木でも非再帰。O(1+n log(n+1))時間・O(1+n)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_B_MATCHING_DEFINED
#define SIRKIT_TREE_B_MATCHING_DEFINED

struct TreeBMatchingEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct TreeBMatchingResult
{
    std::int64_t weight;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
struct TreeBMatchingArc
{
    int child;
    std::size_t edge;
};

struct TreeBMatchingGain
{
    std::int64_t value;
    TreeBMatchingArc arc;
};

using TreeBMatchingGraph = std::vector<std::vector<TreeBMatchingArc>>;

inline TreeBMatchingGraph tree_b_matching_graph(std::span<const int> max_degree,
                                                std::span<const TreeBMatchingEdge> edges)
{
    TreeBMatchingGraph graph(max_degree.size());
#ifndef SKIP_RUNTIME_CHECKS
    for (const int cap : max_degree)
    {
        if (cap < 0)
        {
            throw std::invalid_argument("maximum_weight_tree_b_matching: negative degree limit");
        }
    }
#endif
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || static_cast<std::size_t>(from) >= graph.size() ||
            to < 0 || static_cast<std::size_t>(to) >= graph.size())
        {
            throw std::out_of_range("maximum_weight_tree_b_matching: vertex out of range");
        }
#endif
        graph[from].push_back({to, id});
        graph[to].push_back({from, id});
    }
    return graph;
}

inline std::vector<int> tree_b_matching_order(const TreeBMatchingGraph &graph, std::span<int> parent)
{
    std::vector<int> order{0};
    parent[0] = -1;
    order.reserve(graph.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        const int v = order[i];
        for (const auto &[to, id] : graph[v])
        {
            if (parent[to] == -2)
            {
                parent[to] = v;
                order.push_back(to);
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (order.size() != graph.size())
    {
        throw std::invalid_argument("maximum_weight_tree_b_matching: graph is not a tree");
    }
#endif
    return order;
}
} // namespace detail

inline TreeBMatchingResult maximum_weight_tree_b_matching(
    std::span<const int> max_degree, std::span<const TreeBMatchingEdge> edges)
{
    const auto n = max_degree.size();
    if (edges.size() != n - static_cast<std::size_t>(n != 0))
    {
        throw std::invalid_argument("maximum_weight_tree_b_matching: incorrect number of edges");
    }
    if (n == 0)
    {
        return {0, {}};
    }

    const auto graph = detail::tree_b_matching_graph(max_degree, edges);
    std::vector<int> parent(n, -2);
    const auto order = detail::tree_b_matching_order(graph, parent);
    std::vector<std::int64_t> penalty(n);
    std::vector<detail::TreeBMatchingGain> gains;
    std::vector<detail::TreeBMatchingArc> retained;
    retained.reserve(edges.size());
    std::vector<std::pair<std::size_t, std::size_t>> choice(n);
    TreeBMatchingResult result{0, {}};
    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int v = *it;
        if (max_degree[v] == 0)
        {
            continue;
        }
        gains.clear();
        for (const auto &arc : graph[v])
        {
            const int u = arc.child;
            if (parent[u] != v || max_degree[u] == 0)
            {
                continue;
            }
            // 親辺に枠を使うとき、子側で諦める利得は子の最後の採用分だけ。
            const auto weight = edges[arc.edge].weight;
            if (weight > penalty[u])
            {
                gains.push_back({weight - penalty[u], arc});
            }
        }
        std::sort(gains.begin(), gains.end(), [](const auto &a, const auto &b)
                  {
                      if (a.value != b.value)
                      {
                          return a.value > b.value;
                      }
                      return a.arc.edge < b.arc.edge;
                  });

        const auto limit = static_cast<std::size_t>(max_degree[v]);
        const auto take = std::min(limit, gains.size());
        choice[v] = {retained.size(), take};
        if (take == limit)
        {
            penalty[v] = gains[take - 1].value;
        }
        for (std::size_t i = 0; i < take; ++i)
        {
            // 全頂点の採用利得の和は、根の木DPと等しい。後続の利得も非負。
            if (gains[i].value > std::numeric_limits<std::int64_t>::max() - result.weight)
            {
                throw std::overflow_error("maximum_weight_tree_b_matching: optimal weight does not fit int64_t");
            }
            result.weight += gains[i].value;
            retained.push_back(gains[i].arc);
        }
    }

    std::vector<bool> parent_selected(n);
    result.edge_ids.reserve(retained.size());
    for (const int v : order)
    {
        const auto [first, take] = choice[v];
        const auto loss = parent_selected[v] && take == static_cast<std::size_t>(max_degree[v]);
        for (std::size_t i = 0; i < take - static_cast<std::size_t>(loss); ++i)
        {
            const auto [u, id] = retained[first + i];
            parent_selected[u] = true;
            result.edge_ids.push_back(id);
        }
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


#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<int> cap(n);
    for (int &value : cap)
    {
        verify_in >> value;
    }
    std::vector<TreeBMatchingEdge> edges(n - 1);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    verify_out << maximum_weight_tree_b_matching(cap, edges).weight << '\n';
}
