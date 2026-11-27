#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/chordal_graph_recognition

// Library: Graph/Chordal_Graph.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Chordal_Graph.cpp
// 参考（最大基数探索）: https://doi.org/10.1137/0213035
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chordal_graph.md
//
// API一覧
// recognize_chordal_graph(n,edges): 弦グラフ判定と証明を返す。頂点番号は[0,n)。
// ChordalGraphResult: is_chordalがtrueならperfect_elimination_orderに全頂点を格納する。
// falseならinduced_cycleに4頂点以上の誘導閉路を格納する。始点の再掲はしない。
// 自己辺を無視し、重複辺はまとめて扱う。孤立頂点と空グラフも可。O(n+m)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CHORDAL_GRAPH_DEFINED
#define SIRKIT_CHORDAL_GRAPH_DEFINED

struct ChordalGraphResult
{
    bool is_chordal;
    std::vector<int> perfect_elimination_order;
    std::vector<int> induced_cycle;
};

namespace detail
{
struct ChordalAdjacency
{
    std::vector<std::size_t> offsets;
    std::vector<int> targets;

    std::span<const int> neighbors(int vertex) const
    {
        return std::span<const int>(targets).subspan(offsets[vertex], offsets[vertex + 1] - offsets[vertex]);
    }
};

inline ChordalAdjacency make_chordal_adjacency(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("recognize_chordal_graph: negative vertex count");
    }
    ChordalAdjacency graph{std::vector<std::size_t>(static_cast<std::size_t>(n) + 1), {}};
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("recognize_chordal_graph: endpoint out of range");
        }
#endif
        if (u != v)
        {
            ++graph.offsets[u + 1];
            ++graph.offsets[v + 1];
        }
    }
    for (int v = 0; v < n; ++v)
    {
        graph.offsets[v + 1] += graph.offsets[v];
    }
    graph.targets.resize(graph.offsets[n]);
    auto cursor = graph.offsets;
    for (const auto &[u, v] : edges)
    {
        if (u != v)
        {
            graph.targets[cursor[u]++] = v;
            graph.targets[cursor[v]++] = u;
        }
    }
    return graph;
}

struct ChordalSearchOrder
{
    std::vector<int> order;
    std::vector<int> rank;
    // MCS順で直近の選択済み隣接頂点。
    std::vector<int> parent;
};

inline ChordalSearchOrder chordal_maximum_cardinality_search(int n, const ChordalAdjacency &graph)
{
    constexpr auto kEmpty = std::numeric_limits<std::size_t>::max();
    struct BucketEntry
    {
        std::size_t next;
        int vertex;
    };

    std::vector<std::size_t> heads(n, kEmpty);
    std::vector<BucketEntry> entries;
    entries.reserve(static_cast<std::size_t>(n) + graph.targets.size() / 2);
    const auto enqueue = [&](int label, int v)
    {
        entries.push_back({heads[label], v});
        heads[label] = entries.size() - 1;
    };
    for (int v = 0; v < n; ++v)
    {
        enqueue(0, v);
    }

    ChordalSearchOrder search{{}, std::vector<int>(n), std::vector<int>(n, -1)};
    search.order.reserve(n);
    std::vector<int> label(n);
    int largest = 0;
    for (int i = 0; i < n; ++i)
    {
        int vertex;
        // 更新前のバケット要素は、取り出したときに捨てる。
        for (;;)
        {
            auto &head = heads[largest];
            if (head == kEmpty)
            {
                --largest;
                continue;
            }
            const auto entry = entries[head];
            head = entry.next;
            if (label[entry.vertex] == largest)
            {
                vertex = entry.vertex;
                break;
            }
        }
        label[vertex] = -1;
        search.rank[vertex] = i;
        search.order.push_back(vertex);
        for (int to : graph.neighbors(vertex))
        {
            if (label[to] >= 0 && search.parent[to] != vertex)
            {
                // 同じ頂点から来た多重辺では、ラベルを一度だけ増やす。
                search.parent[to] = vertex;
                ++label[to];
                enqueue(label[to], to);
                largest = std::max(largest, label[to]);
            }
        }
    }
    return search;
}

inline int first_chordal_elimination_violation(const ChordalAdjacency &graph, const ChordalSearchOrder &search)
{
    const int n = static_cast<int>(search.order.size());
    std::vector<int> first_child(n, -1), sibling(n, -1);
    for (int v = 0; v < n; ++v)
    {
        if (const int parent = search.parent[v]; parent >= 0)
        {
            sibling[v] = first_child[parent];
            first_child[parent] = v;
        }
    }
    std::vector<int> adjacent(n, -1);
    int earliest = n;
    for (int parent : search.order)
    {
        for (int to : graph.neighbors(parent))
        {
            adjacent[to] = parent;
        }
        for (int child = first_child[parent]; child >= 0; child = sibling[child])
        {
            for (int to : graph.neighbors(child))
            {
                if (search.rank[to] < search.rank[parent] && adjacent[to] != parent)
                {
                    earliest = std::min(earliest, search.rank[child]);
                    break;
                }
            }
        }
    }
    return earliest;
}

inline std::vector<int> chordal_cycle_from_boundary(
    const ChordalAdjacency &graph, const std::vector<int> &component, int seed, int apex, int first, int last)
{
    std::vector<int> predecessor(component.size(), -1);
    predecessor[first] = first;
    std::vector<int> queue{first};
    // firstとlastはどちらも成分seedに接するので、成分内を通って到達できる。
    for (std::size_t head = 0; predecessor[last] < 0; ++head)
    {
        const int from = queue[head];
        for (int to : graph.neighbors(from))
        {
            if (predecessor[to] < 0 && (component[to] == seed || to == last))
            {
                predecessor[to] = from;
                queue.push_back(to);
            }
        }
    }
    std::vector<int> cycle{apex};
    for (int v = last; v != first; v = predecessor[v])
    {
        cycle.push_back(v);
    }
    cycle.push_back(first);
    return cycle;
}

inline std::vector<int> chordal_prefix_hole(
    const ChordalAdjacency &graph, const ChordalSearchOrder &search, int cutoff)
{
    constexpr int kUnseen = -1;
    constexpr int kNeighbor = -2;
    struct Boundary
    {
        std::size_t begin;
        std::size_t end;
        int component;
        int next;
    };

    const auto n = search.order.size();
    const int apex = search.order[cutoff];
    std::vector<int> component(n, kUnseen), stamp(n, kUnseen), group(n, kUnseen);
    for (int v : graph.neighbors(apex))
    {
        component[v] = kNeighbor;
    }

    std::vector<int> boundary_vertices;
    std::vector<Boundary> boundaries;
    std::vector<int> queue;
    for (int i = 0; i < cutoff; ++i)
    {
        const int seed = search.order[i];
        if (component[seed] != kUnseen)
        {
            continue;
        }
        component[seed] = seed;
        queue.clear();
        queue.push_back(seed);
        const auto begin = boundary_vertices.size();
        int pivot = -1;
        for (std::size_t head = 0; head < queue.size(); ++head)
        {
            for (int to : graph.neighbors(queue[head]))
            {
                if (search.rank[to] >= cutoff)
                {
                    continue;
                }
                if (component[to] == kNeighbor)
                {
                    if (stamp[to] != seed)
                    {
                        stamp[to] = seed;
                        boundary_vertices.push_back(to);
                        if (pivot < 0 || search.rank[pivot] < search.rank[to])
                        {
                            pivot = to;
                        }
                    }
                }
                else if (component[to] == kUnseen)
                {
                    component[to] = seed;
                    queue.push_back(to);
                }
            }
        }
        if (pivot >= 0)
        {
            boundaries.push_back({begin, boundary_vertices.size(), seed, group[pivot]});
            group[pivot] = static_cast<int>(boundaries.size() - 1);
        }
    }

    // 頂点apexより前の部分は弦グラフ。境界の先頭を除去する段階で、
    // 残りが先頭にすべて隣接すれば完全除去順の性質から境界はクリーク。
    std::fill(stamp.begin(), stamp.end(), kUnseen);
    for (int i = 0; i < cutoff; ++i)
    {
        const int pivot = search.order[i];
        if (group[pivot] < 0)
        {
            continue;
        }
        for (int to : graph.neighbors(pivot))
        {
            stamp[to] = pivot;
        }
        for (int b = group[pivot]; b >= 0; b = boundaries[b].next)
        {
            const auto &boundary = boundaries[b];
            for (auto j = boundary.begin; j < boundary.end; ++j)
            {
                const int v = boundary_vertices[j];
                if (v != pivot && stamp[v] != pivot)
                {
                    return chordal_cycle_from_boundary(graph, component, boundary.component, apex, pivot, v);
                }
            }
        }
    }
    return {};
}
} // namespace detail

inline ChordalGraphResult recognize_chordal_graph(int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::make_chordal_adjacency(n, edges);
    auto search = detail::chordal_maximum_cardinality_search(n, graph);
    const int cutoff = detail::first_chordal_elimination_violation(graph, search);
    if (cutoff < n)
    {
        // MCSの先頭cutoff+1頂点で初めて弦グラフでなくなる。
        // その部分グラフの誘導閉路はcutoff番目の頂点を必ず通る。
        return {false, {}, detail::chordal_prefix_hole(graph, search, cutoff)};
    }
    std::reverse(search.order.begin(), search.order.end());
    return {true, std::move(search.order), {}};
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
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }

    const auto result = recognize_chordal_graph(n, edges);
    const auto &vertices = result.is_chordal ? result.perfect_elimination_order : result.induced_cycle;
    if (result.is_chordal)
    {
        verify_out << "YES\n";
    }
    else
    {
        verify_out << "NO\n"
                   << vertices.size() << '\n';
    }
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << vertices[i];
    }
    verify_out << '\n';
}
