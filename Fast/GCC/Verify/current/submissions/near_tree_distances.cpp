#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/past202104-open/tasks/past202104_o

// Library: Graph/Near_Tree_Distances.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Near_Tree_Distances.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/near_tree_distances.md
//
// API一覧
// NearTreeDistances(n, edges): 無向・非重み付きグラフを前処理。多重辺・自己辺も可。
//   成分iの頂点数をn_i、辺数をm_i、余分な辺数をk_i=m_i-n_i+1とする。
//   時間O(sum_i((k_i+1)(n_i+m_i)))、領域O(m+sum_i((k_i+1)n_i))。
// size(): 頂点数。O(1)。
// distance(u,v): 最短路の辺数。非連結なら-1。同じ成分iならO(k_i+log(n_i+1))、非連結ならO(1)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNDIRECTED_PSEUDOFOREST_DEFINED
#define SIRKIT_UNDIRECTED_PSEUDOFOREST_DEFINED

class UndirectedPseudoforest
{
    struct Arc
    {
        int to;
        int id;
    };

public:
    struct Cycle
    {
        std::span<const int> vertices;
        std::span<const int> edge_ids;
    };

    static std::optional<UndirectedPseudoforest> build(int n, std::span<const std::pair<int, int>> edges)
    {
        if (n < 0)
        {
            throw std::length_error("UndirectedPseudoforest: negative size");
        }
        if (edges.size() > static_cast<std::size_t>(n))
        {
            return std::nullopt;
        }
        UndirectedPseudoforest result(n);
        std::vector<std::vector<Arc>> adjacency(n);
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            const auto &[u, v] = edges[id];
            adjacency[u].push_back({v, static_cast<int>(id)});
            adjacency[v].push_back({u, static_cast<int>(id)});
        }

        std::vector<std::size_t> degree(n);
        std::vector<int> peeled;
        peeled.reserve(n);
        for (int v = 0; v < n; ++v)
        {
            degree[v] = adjacency[v].size();
            if (degree[v] <= 1)
            {
                peeled.push_back(v);
            }
        }
        for (std::size_t i = 0; i < peeled.size(); ++i)
        {
            const int v = peeled[i];
            degree[v] = 0;
            for (const Arc &arc : adjacency[v])
            {
                if (degree[arc.to] != 0)
                {
                    result.parent_[v] = arc.to;
                    if (--degree[arc.to] == 1)
                    {
                        peeled.push_back(arc.to);
                    }
                    break;
                }
            }
        }
        if (std::ranges::any_of(degree, [](std::size_t d)
                                {
                                    return d > 2;
                                }))
        {
            return std::nullopt;
        }
        result.build_cycles(adjacency, degree);
        result.build_attachment_trees(peeled);
        return result;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(parent_.size());
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return tree_count_ + cycle_count();
    }

    [[nodiscard]] int cycle_count() const noexcept
    {
        return static_cast<int>(cycle_offsets_.size() - 1);
    }

    [[nodiscard]] int component(int v) const
    {
        check_vertex(v);
        return component_[v];
    }

    [[nodiscard]] int root(int v) const
    {
        check_vertex(v);
        return root_[v];
    }

    [[nodiscard]] int depth(int v) const
    {
        check_vertex(v);
        return depth_[v];
    }

    [[nodiscard]] int cycle_id(int v) const
    {
        return cycle_id_[root(v)];
    }

    [[nodiscard]] Cycle cycle(int id) const
    {
        const auto first = cycle_offsets_[id];
        const auto length = cycle_offsets_[id + 1] - first;
        return {std::span(cycle_vertices_).subspan(first, length),
                std::span(cycle_edges_).subspan(first, length)};
    }

    [[nodiscard]] bool unique_path(int u, int v) const
    {
        return root(u) == root(v);
    }

    [[nodiscard]] int distance(int u, int v) const
    {
        if (component(u) != component(v))
        {
            return -1;
        }
        const int a = root_[u];
        const int b = root_[v];
        if (a != b)
        {
            const int length = static_cast<int>(cycle(cycle_id_[a]).vertices.size());
            const int along = std::abs(position_[a] - position_[b]);
            return depth_[u] + depth_[v] + std::min(along, length - along);
        }
        const int lca = attachment_lca(u, v);
        return (depth_[u] - depth_[lca]) + (depth_[v] - depth_[lca]);
    }

private:
    explicit UndirectedPseudoforest(int n)
        : parent_(n, -1), root_(n, -1), depth_(n), head_(n), component_(n),
          cycle_id_(n, -1), position_(n), cycle_offsets_{0}
    {
    }

    void build_cycles(const std::vector<std::vector<Arc>> &adjacency, const std::vector<std::size_t> &degree)
    {
        for (int start = 0; start < size(); ++start)
        {
            if (degree[start] == 0 || root_[start] != -1)
            {
                continue;
            }
            const int id = cycle_count();
            const auto begin = cycle_offsets_.back();
            int vertex = start;
            int last_edge = -1;
            do
            {
                root_[vertex] = vertex;
                head_[vertex] = vertex;
                component_[vertex] = start;
                cycle_id_[vertex] = id;
                position_[vertex] = static_cast<int>(cycle_vertices_.size() - begin);
                cycle_vertices_.push_back(vertex);
                // A core vertex has exactly two incident arcs. Use the other edge to advance.
                const auto arc = std::ranges::find_if(adjacency[vertex], [&](const Arc &e)
                                                      {
                                                          return degree[e.to] != 0 && e.id != last_edge;
                                                      });
                last_edge = arc->id;
                cycle_edges_.push_back(last_edge);
                vertex = arc->to;
            } while (vertex != start);
            cycle_offsets_.push_back(cycle_vertices_.size());
        }
    }

    void build_attachment_trees(const std::vector<int> &peeled)
    {
        std::vector<int> subtree(size(), 1);
        std::vector<int> heavy(size(), -1);
        for (const int v : peeled)
        {
            const int p = parent_[v];
            if (p != -1)
            {
                subtree[p] += subtree[v];
                if (heavy[p] == -1 || subtree[heavy[p]] < subtree[v])
                {
                    heavy[p] = v;
                }
            }
        }
        for (auto it = peeled.rbegin(); it != peeled.rend(); ++it)
        {
            const int v = *it;
            const int p = parent_[v];
            if (p == -1)
            {
                root_[v] = head_[v] = component_[v] = v;
                ++tree_count_;
            }
            else
            {
                root_[v] = root_[p];
                component_[v] = component_[p];
                head_[v] = heavy[p] == v ? head_[p] : v;
                depth_[v] = depth_[p] + 1;
            }
        }
    }

    [[nodiscard]] int attachment_lca(int u, int v) const
    {
        while (head_[u] != head_[v])
        {
            if (depth_[head_[u]] < depth_[head_[v]])
            {
                std::swap(u, v);
            }
            u = parent_[head_[u]];
        }
        return depth_[u] < depth_[v] ? u : v;
    }

    void check_vertex(int v) const
    {
        static_cast<void>(v);
    }

    int tree_count_ = 0;
    std::vector<int> parent_;
    std::vector<int> root_;
    std::vector<int> depth_;
    std::vector<int> head_;
    std::vector<int> component_;
    std::vector<int> cycle_id_;
    std::vector<int> position_;
    std::vector<std::size_t> cycle_offsets_;
    std::vector<int> cycle_vertices_;
    std::vector<int> cycle_edges_;
};

#endif

#ifndef SIRKIT_NEAR_TREE_DISTANCES_DEFINED
#define SIRKIT_NEAR_TREE_DISTANCES_DEFINED

class NearTreeDistances
{
    using Adjacency = std::vector<std::vector<int>>;

    struct Location
    {
        int component = -1;
        int offset = 0;
    };

public:
    NearTreeDistances(int n, std::span<const std::pair<int, int>> edges)
        : vertices_(checked_size(n)), tree_(prepare(edges))
    {
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(vertices_.size());
    }

    [[nodiscard]] int distance(int from, int to) const
    {
        check_vertex(from);
        check_vertex(to);
        const auto a = vertices_[from];
        const auto b = vertices_[to];
        if (a.component != b.component)
        {
            return -1;
        }
        int answer = tree_.distance(from, to);
        for (const auto &distance : from_hub_[a.component])
        {
            const auto through_hub = static_cast<std::int64_t>(distance[a.offset]) + distance[b.offset];
            if (through_hub < answer)
            {
                answer = static_cast<int>(through_hub);
            }
        }
        return answer;
    }

private:
    static int checked_size(int n)
    {
        if (n < 0)
        {
            throw std::length_error("NearTreeDistances: negative size");
        }
        return n;
    }

    void check_vertex(int v) const
    {
        static_cast<void>(v);
    }

    std::vector<int> bfs(const Adjacency &graph, int source, std::size_t n, std::vector<int> &pending) const
    {
        std::vector<int> distance(n, -1);
        pending.clear();
        pending.push_back(source);
        distance[vertices_[source].offset] = 0;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int u = pending[i];
            const int next_distance = distance[vertices_[u].offset] + 1;
            for (const int v : graph[u])
            {
                int &value = distance[vertices_[v].offset];
                if (value == -1)
                {
                    value = next_distance;
                    pending.push_back(v);
                }
            }
        }
        return distance;
    }

    UndirectedPseudoforest prepare(std::span<const std::pair<int, int>> edges)
    {
        const int n = size();
        Adjacency graph(n);
        for (const auto &[u, v] : edges)
        {
            graph[u].push_back(v);
            graph[v].push_back(u);
        }
        std::vector<std::pair<int, int>> tree;
        tree.reserve(n);
        std::vector<int> parent(n, -1);
        std::vector<bool> hub(n);
        std::vector<int> vertices;
        vertices.reserve(n);
        std::vector<int> pending;
        for (int root = 0; root < n; ++root)
        {
            if (vertices_[root].component != -1)
            {
                continue;
            }
            const int component = static_cast<int>(from_hub_.size());
            auto &rows = from_hub_.emplace_back();
            vertices.clear();
            vertices.push_back(root);
            vertices_[root] = {component, 0};
            for (std::size_t i = 0; i < vertices.size(); ++i)
            {
                const int u = vertices[i];
                for (const int v : graph[u])
                {
                    if (vertices_[v].component == -1)
                    {
                        vertices_[v] = {component, static_cast<int>(vertices.size())};
                        tree.emplace_back(u, v);
                        parent[v] = u;
                        vertices.push_back(v);
                    }
                }
            }

            // A hub at either endpoint covers a non-tree edge. Loops and extra copies of a tree edge are redundant.
            for (const int u : vertices)
            {
                for (const int v : graph[u])
                {
                    if (u < v && parent[u] != v && parent[v] != u && !hub[u] && !hub[v])
                    {
                        hub[u] = true;
                        rows.push_back(bfs(graph, u, vertices.size(), pending));
                    }
                }
            }
        }
        return UndirectedPseudoforest::build(n, tree).value();
    }

    std::vector<Location> vertices_;
    std::vector<std::vector<std::vector<int>>> from_hub_;
    UndirectedPseudoforest tree_;
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
    int n{}, m{}, q{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::NearTreeDistances distances(n, edges);
    verify_in >> q;
    while (q-- > 0)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << distances.distance(u - 1, v - 1) << '\n';
    }
}
