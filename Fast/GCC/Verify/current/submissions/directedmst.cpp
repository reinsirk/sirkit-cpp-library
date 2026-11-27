#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/directedmst

// Library: Graph/Minimum_Directed_Spanning_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Minimum_Directed_Spanning_Tree.cpp
// 参考（有向閉路の縮約と優先度付きキュー）: https://doi.org/10.1002/net.3230070103
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_directed_spanning_tree.md
//
// API一覧
// DirectedMstEdge: 有向辺{from,to,weight}。頂点番号は0始まり。負の重み・自己辺・多重辺に対応。
// minimum_directed_spanning_tree(n,root,edges): rootから全頂点へ到達する最小全域有向木を返す。存在しなければstd::nullopt。
// DirectedMstResult: costは重みの合計。incoming_edge[v]は頂点vに入る辺の入力添字。根の欄はkNoEdge。
// 辺・合計は符号付き64ビット。最小費用が範囲外ならoverflow_error。
// O(N+M log(M+1))時間（単純グラフならO(N+M log(N+1))）・O(N+M)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_DIRECTED_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_DIRECTED_SPANNING_TREE_DEFINED

struct DirectedMstEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct DirectedMstResult
{
    static constexpr std::size_t kNoEdge = std::numeric_limits<std::size_t>::max();
    std::int64_t cost;
    std::vector<std::size_t> incoming_edge;
};

namespace detail
{
inline constexpr std::size_t kDirectedMstNone = DirectedMstResult::kNoEdge;

class DirectedMstHeap
{
    struct Node
    {
        std::size_t left = kDirectedMstNone;
        std::size_t right = kDirectedMstNone;
        std::uint64_t key = 0;
        std::uint64_t decrease = 0;
        int rank = 1;
    };

    std::vector<Node> nodes_;

    int rank(std::size_t node) const
    {
        return node == kDirectedMstNone ? 0 : nodes_[node].rank;
    }

    void push(std::size_t node)
    {
        auto &entry = nodes_[node];
        reduce(entry.left, entry.decrease);
        reduce(entry.right, entry.decrease);
        entry.decrease = 0;
    }

public:
    explicit DirectedMstHeap(std::size_t size) : nodes_(size)
    {
    }

    void initialize(std::size_t edge, std::int64_t weight)
    {
        // 全ての解の辺数が等しいため、重みに同じ定数を足してよい。
        constexpr auto kSign = std::uint64_t{1} << 63;
        nodes_[edge].key = static_cast<std::uint64_t>(weight) ^ kSign;
    }

    std::uint64_t key(std::size_t node) const
    {
        return nodes_[node].key;
    }

    void reduce(std::size_t node, std::uint64_t amount)
    {
        if (node != kDirectedMstNone)
        {
            // 縮約による各辺の減少量は、元の非負のキー以下に収まる。
            nodes_[node].key -= amount;
            nodes_[node].decrease += amount;
        }
    }

    std::size_t meld(std::size_t a, std::size_t b)
    {
        if (a == kDirectedMstNone)
        {
            return b;
        }
        if (b == kDirectedMstNone)
        {
            return a;
        }
        if (nodes_[a].key > nodes_[b].key)
        {
            std::swap(a, b);
        }
        push(a);
        auto &entry = nodes_[a];
        entry.right = meld(entry.right, b);
        if (rank(entry.left) < rank(entry.right))
        {
            std::swap(entry.left, entry.right);
        }
        entry.rank = rank(entry.right) + 1;
        return a;
    }

    std::size_t pop(std::size_t node)
    {
        push(node);
        return meld(nodes_[node].left, nodes_[node].right);
    }
};

class DirectedMstDisjointSet
{
    std::vector<int> parents_;

public:
    explicit DirectedMstDisjointSet(int n) : parents_(n, -1)
    {
    }

    int find(int vertex)
    {
        while (parents_[vertex] >= 0)
        {
            const int parent = parents_[vertex];
            if (parents_[parent] < 0)
            {
                return parent;
            }
            vertex = parents_[vertex] = parents_[parent];
        }
        return vertex;
    }

    int join_roots(int a, int b)
    {
        if (parents_[a] > parents_[b])
        {
            std::swap(a, b);
        }
        parents_[a] += parents_[b];
        parents_[b] = a;
        return a;
    }
};

struct DirectedMstComponent
{
    std::size_t heap = kDirectedMstNone;
    std::size_t incoming = kDirectedMstNone;
    std::size_t first_child = kDirectedMstNone;
    std::size_t sibling = kDirectedMstNone;
    int representative = 0;
    int seen_from = -1;
};

struct DirectedMstInterval
{
    int begin = -1;
    int end = -1;
};

inline std::vector<DirectedMstInterval> directed_mst_intervals(
    int n, const std::vector<DirectedMstComponent> &components)
{
    std::vector<DirectedMstInterval> intervals(components.size());
    std::vector<std::pair<std::size_t, bool>> pending;
    int leaves = 0;
    // 親の縮約番号は子より大きいので、各縮約木の根から順に始められる。
    for (std::size_t i = components.size(); i != 0; --i)
    {
        const auto top = i - 1;
        if (intervals[top].begin >= 0)
        {
            continue;
        }
        pending.emplace_back(top, false);
        while (!pending.empty())
        {
            const auto [node, finished] = pending.back();
            pending.pop_back();
            auto &interval = intervals[node];
            if (finished)
            {
                interval.end = leaves;
            }
            else if (node < static_cast<std::size_t>(n))
            {
                interval.begin = leaves;
                interval.end = ++leaves;
            }
            else
            {
                interval.begin = leaves;
                pending.emplace_back(node, true);
                for (auto child = components[node].first_child; child != kDirectedMstNone; child = components[child].sibling)
                {
                    pending.emplace_back(child, false);
                }
            }
        }
    }
    return intervals;
}

inline DirectedMstResult restore_directed_mst(
    int n, int root, std::span<const DirectedMstEdge> edges, std::vector<DirectedMstComponent> &components)
{
    if (components.size() > static_cast<std::size_t>(n))
    {
        const auto intervals = directed_mst_intervals(n, components);
        for (std::size_t i = components.size(); i > static_cast<std::size_t>(n); --i)
        {
            const auto &component = components[i - 1];
            const int target = intervals[edges[component.incoming].to].begin;
            // 外から入る辺の着点を含む子だけ、閉路内で選んだ入辺を置換する。
            for (auto child = component.first_child; child != kDirectedMstNone; child = components[child].sibling)
            {
                if (intervals[child].begin <= target && target < intervals[child].end)
                {
                    components[child].incoming = component.incoming;
                }
            }
        }
    }

    DirectedMstResult result{0, std::vector<std::size_t>(n, kDirectedMstNone)};
    std::uint64_t low = 0;
    std::int64_t high = 0;
    for (int vertex = 0; vertex < n; ++vertex)
    {
        if (vertex == root)
        {
            continue;
        }
        const auto index = components[vertex].incoming;
        result.incoming_edge[vertex] = index;
        const auto weight = edges[index].weight;
        const auto next = low + static_cast<std::uint64_t>(weight);
        // 128ビット相当で加算し、途中で64ビットを超えてから相殺する場合も扱う。
        high += static_cast<int>(next < low) - static_cast<int>(weight < 0);
        low = next;
    }
    result.cost = static_cast<std::int64_t>(low);
    if (high != -static_cast<int>(result.cost < 0))
    {
        throw std::overflow_error("minimum_directed_spanning_tree: total cost does not fit int64_t");
    }
    return result;
}
} // namespace detail

inline std::optional<DirectedMstResult> minimum_directed_spanning_tree(
    int n, int root, std::span<const DirectedMstEdge> edges)
{
    if (n <= 0)
    {
        throw std::invalid_argument("minimum_directed_spanning_tree: nonpositive vertex count");
    }
    constexpr auto kNone = detail::kDirectedMstNone;
    std::vector<detail::DirectedMstComponent> components(n);
    components.reserve(static_cast<std::size_t>(n) * 2 - 1);
    detail::DirectedMstHeap heap(edges.size());
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[from, to, weight] = edges[i];
        if (to == root || from == to)
        {
            continue;
        }
        heap.initialize(i, weight);
        components[to].heap = heap.meld(components[to].heap, i);
    }

    detail::DirectedMstDisjointSet groups(n);
    std::vector<std::size_t> component_of(n);
    for (int vertex = 0; vertex < n; ++vertex)
    {
        components[vertex].representative = vertex;
        component_of[vertex] = static_cast<std::size_t>(vertex);
    }
    components[root].seen_from = root;
    std::vector<std::size_t> path;
    for (int start = 0; start < n; ++start)
    {
        auto current = component_of[groups.find(start)];
        while (components[current].seen_from < 0)
        {
            auto &component = components[current];
            component.seen_from = start;
            path.push_back(current);

            std::size_t source = kNone;
            while (component.heap != kNone)
            {
                const auto edge = component.heap;
                source = component_of[groups.find(edges[edge].from)];
                component.heap = heap.pop(edge);
                if (source != current)
                {
                    component.incoming = edge;
                    heap.reduce(component.heap, heap.key(edge));
                    break;
                }
            }
            if (component.incoming == kNone)
            {
                return std::nullopt;
            }
            if (components[source].seen_from != start)
            {
                current = source;
                continue;
            }

            // pathの末尾にある閉路を縮約し、調整済みの入辺ヒープを結合する。
            detail::DirectedMstComponent cycle;
            cycle.representative = component.representative;
            do
            {
                const auto child = path.back();
                path.pop_back();
                auto &member = components[child];
                member.sibling = cycle.first_child;
                cycle.first_child = child;
                cycle.heap = heap.meld(cycle.heap, member.heap);
                if (child != current)
                {
                    cycle.representative = groups.join_roots(cycle.representative, member.representative);
                }
            } while (cycle.first_child != source);
            current = components.size();
            component_of[cycle.representative] = current;
            components.push_back(cycle);
        }
        path.clear();
    }
    return detail::restore_directed_mst(n, root, edges, components);
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
    int n, m, root;
    verify_in >> n >> m >> root;
    std::vector<DirectedMstEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto result = minimum_directed_spanning_tree(n, root, edges);
    verify_out << result->cost << '\n';
    for (int v = 0; v < n; ++v)
    {
        if (v != 0)
        {
            verify_out << ' ';
        }
        verify_out << (v == root ? root : edges[result->incoming_edge[v]].from);
    }
    verify_out << '\n';
}
