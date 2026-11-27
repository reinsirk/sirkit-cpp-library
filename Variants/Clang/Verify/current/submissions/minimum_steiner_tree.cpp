#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/minimum_steiner_tree

// Library: Graph/Steiner_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Steiner_Tree.cpp
// 参考（部分集合DP）: https://doi.org/10.1002/net.3230010302
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/steiner_tree.md
//
// API一覧
// SteinerTreeEdge{from,to,weight}: 端点が[0,n)の無向辺。重みは非負のint64_t。
// minimum_steiner_tree(n,edges,terminals): 全指定頂点を結ぶ最小費用の木を返す。
// SteinerTree{cost,edge_ids}: 費用と、選んだ辺の入力位置。順序は任意。
// 指定頂点の重複を除き、0個または1個なら費用0と空の辺列。連結不能ならnullopt。
// 費用がint64_tの上限を超えるとoverflow_error。自己辺・多重辺可。
// 異なる指定頂点がk≥2個ならb=k−1、Tを重複込みの指定数として、時間O(T+3^b*n+2^b*(n+m)*log(n+m+1))、領域O(2^b*n+m)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STEINER_TREE_DEFINED
#define SIRKIT_STEINER_TREE_DEFINED

struct SteinerTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct SteinerTree
{
    std::int64_t cost;
    std::vector<std::size_t> edge_ids;
};

inline std::optional<SteinerTree> minimum_steiner_tree(
    int n, std::span<const SteinerTreeEdge> edges, std::span<const int> terminals)
{
    if (n < 0)
    {
        throw std::length_error("minimum_steiner_tree: negative vertex count");
    }
    if (terminals.empty())
    {
        return SteinerTree{0, {}};
    }
    const auto vertex_count = static_cast<std::size_t>(n);
    std::vector<unsigned char> reached(vertex_count);
    std::vector<int> distinct;
    for (int vertex : terminals)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n)
        {
            throw std::out_of_range("minimum_steiner_tree: terminal out of range");
        }
#endif
        if (!reached[vertex])
        {
            reached[vertex] = 1;
            distinct.push_back(vertex);
        }
    }
    if (distinct.size() == 1)
    {
        return SteinerTree{0, {}};
    }

    using Mask = std::size_t;
    using Cost = std::uint64_t;
    constexpr Cost kUnreachable = Cost{1} << std::numeric_limits<std::int64_t>::digits;
    const auto variable_count = distinct.size() - 1;
    if (variable_count >= std::numeric_limits<Mask>::digits)
    {
        throw std::length_error("minimum_steiner_tree: too many terminals for a subset mask");
    }
    const Mask subset_count = Mask{1} << variable_count;
    struct Choice
    {
        std::size_t value = 0;
        bool via_edge = false;
    };
    std::vector<Cost> dp;
    std::vector<Choice> trace;
    if (subset_count > std::min(dp.max_size(), trace.max_size()) / vertex_count)
    {
        throw std::length_error("minimum_steiner_tree: subset tables are too large");
    }

    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(vertex_count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("minimum_steiner_tree: endpoint out of range");
        }
        if (weight < 0)
        {
            throw std::invalid_argument("minimum_steiner_tree: negative edge weight");
        }
#endif
        graph[from].push_back({to, id});
        graph[to].push_back({from, id});
    }

    const auto state_count = subset_count * vertex_count;
    dp.assign(state_count, kUnreachable);
    trace.resize(state_count);
    using QueueEntry = std::pair<Cost, int>;
    std::vector<QueueEntry> queue;
    constexpr std::greater<QueueEntry> kGreater{};
    for (Mask mask = 1; mask < subset_count; ++mask)
    {
        const auto base = mask * vertex_count;
        if (std::has_single_bit(mask))
        {
            dp[base + distinct[std::countr_zero(mask) + 1]] = 0;
        }
        else
        {
            // 子集合対の片側に最下位ビットを固定し、各分割を一度だけ試す。
            const Mask pivot = Mask{1} << std::countr_zero(mask);
            const Mask rest = mask ^ pivot;
            for (Mask part = (rest - 1) & rest;; part = (part - 1) & rest)
            {
                const Mask left = part | pivot;
                const auto left_base = left * vertex_count;
                const auto right_base = (mask ^ left) * vertex_count;
                for (std::size_t v = 0; v < vertex_count; ++v)
                {
                    const auto first = dp[left_base + v];
                    const auto second = dp[right_base + v];
                    if (first != kUnreachable && second != kUnreachable)
                    {
                        // 有限値は最大INT64_MAX。2つ足してもuint64_tに収まる。
                        const auto merged = first + second;
                        if (merged < dp[base + v])
                        {
                            dp[base + v] = merged;
                            trace[base + v] = {left, false};
                        }
                    }
                }
                if (part == 0)
                {
                    break;
                }
            }
        }

        queue.clear();
        for (int v = 0; v < n; ++v)
        {
            if (dp[base + v] != kUnreachable)
            {
                queue.emplace_back(dp[base + v], v);
            }
        }
        std::make_heap(queue.begin(), queue.end(), kGreater);
        while (!queue.empty())
        {
            std::pop_heap(queue.begin(), queue.end(), kGreater);
            const auto [cost, vertex] = queue.back();
            queue.pop_back();
            if (cost != dp[base + vertex])
            {
                continue;
            }
            for (const auto &[to, id] : graph[vertex])
            {
                const auto next_cost = cost + static_cast<Cost>(edges[id].weight);
                if (next_cost < dp[base + to])
                {
                    dp[base + to] = next_cost;
                    trace[base + to] = {id, true};
                    queue.emplace_back(next_cost, to);
                    std::push_heap(queue.begin(), queue.end(), kGreater);
                }
            }
        }
    }

    const Mask all = subset_count - 1;
    const int root = distinct.front();
    const auto best = dp[all * vertex_count + root];
    const bool representable = best != kUnreachable;
    std::vector<unsigned char> selected(edges.size(), static_cast<unsigned char>(!representable));
    if (representable)
    {
        std::vector<std::pair<Mask, int>> pending{{all, root}};
        while (!pending.empty())
        {
            const auto [mask, vertex] = pending.back();
            pending.pop_back();
            const auto choice = std::exchange(trace[mask * vertex_count + vertex], Choice{});
            if (choice.via_edge)
            {
                selected[choice.value] = 1;
                const auto &edge = edges[choice.value];
                const int from = edge.to == vertex ? edge.from : edge.to;
                pending.emplace_back(mask, from);
            }
            else if (choice.value != 0)
            {
                pending.emplace_back(choice.value, vertex);
                pending.emplace_back(mask ^ choice.value, vertex);
            }
        }
    }

    // 復元した辺の和集合から木を取り出す。有限解がなければ元グラフで到達性を調べる。
    std::fill(reached.begin(), reached.end(), 0);
    reached[root] = 1;
    std::vector<int> order{root};
    SteinerTree result{representable ? static_cast<std::int64_t>(best) : 0, {}};
    for (std::size_t i = 0; i < order.size(); ++i)
    {
        for (const auto &[to, id] : graph[order[i]])
        {
            if (selected[id] && !reached[to])
            {
                reached[to] = 1;
                order.push_back(to);
                if (representable)
                {
                    result.edge_ids.push_back(id);
                }
            }
        }
    }
    if (!representable)
    {
        if (std::ranges::all_of(distinct, [&](int terminal)
                                {
                                    return reached[terminal] != 0;
                                }))
        {
            throw std::overflow_error("minimum_steiner_tree: minimum cost does not fit int64_t");
        }
        return std::nullopt;
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
    int n, m;
    verify_in >> n >> m;
    std::vector<SteinerTreeEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }
    int k;
    verify_in >> k;
    std::vector<int> terminals(k);
    for (auto &v : terminals)
    {
        verify_in >> v;
    }
    const auto result = minimum_steiner_tree(n, edges, terminals);
    if (!result)
    {
        return 1;
    }
    verify_out << result->cost << ' ' << result->edge_ids.size() << '\n';
    for (std::size_t i = 0; i < result->edge_ids.size(); ++i)
    {
        verify_out << result->edge_ids[i] << (i + 1 == result->edge_ids.size() ? '\n' : ' ');
    }
}
