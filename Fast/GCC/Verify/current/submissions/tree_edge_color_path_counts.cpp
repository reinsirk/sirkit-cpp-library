#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1002

// Library: Graph/Tree_Edge_Color_Path_Counts.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Tree_Edge_Color_Path_Counts.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_edge_color_path_counts.md
//
// API一覧
// tree_edge_color_path_counts(n,edges): 木のパスに現れる辺色が1種類、2種類の頂点対数をそれぞれ返す。
//   TreeColorEdge{from,to,color}。頂点は0始まり、色は任意のint64_t。
// TreeEdgeColorPathCounts{one_color,two_colors}: 異なる2頂点を順序なしで数える。
//   重心を通るパスだけを左右の枝の色集合から数える。O(n log²(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_CENTROID_DECOMPOSITION_DEFINED
#define SIRKIT_CENTROID_DECOMPOSITION_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckCentroidVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("CentroidDecomposition vertex count");
}
}
class CentroidDecomposition {
public:
    struct Ancestor {
        int vertex;
        int distance;
        friend bool operator==(const Ancestor&, const Ancestor&) = default;
    };
private:
    int root_ = -1;
    bool stores_distances_ = false;
    std::vector<int> parent_, depth_, component_size_, order_;
    std::vector<std::vector<Ancestor>> ancestors_;

    void check_vertex(int vertex) const {
        static_cast<void>(vertex);
    }
    void check_distances() const {
    }
    void initialize(const Graph& graph, bool store_distances) {
        const auto n = graph.size();
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckCentroidVertexCount(n);
        stores_distances_ = store_distances;
        parent_.assign(n, -1); depth_.resize(n); component_size_.resize(n); order_.reserve(n);
        if (store_distances) ancestors_.resize(n);
        if (n == 0) return;
        std::vector<unsigned char> removed(n);
        std::vector<int> traversal_parent(n, -2), subtree(n), component;
        std::vector<std::pair<int,int>> pending{{0,-1}};
        struct Visit { int vertex, parent, distance; };
        std::vector<Visit> visits;
        component.reserve(n);
        while (!pending.empty()) {
            const auto [start, above] = pending.back(); pending.pop_back();
            component.clear(); component.push_back(start);
            traversal_parent[static_cast<std::size_t>(start)] = -1;
            for (std::size_t i = 0; i < component.size(); ++i) {
                const int vertex = component[i];
                subtree[static_cast<std::size_t>(vertex)] = 1;
                for (const int to : graph[static_cast<std::size_t>(vertex)]) {
                    if (to == traversal_parent[static_cast<std::size_t>(vertex)]) {
                        continue;
                    }
                    if (removed[static_cast<std::size_t>(to)]) continue;
                    traversal_parent[static_cast<std::size_t>(to)] = vertex;
                    component.push_back(to);
                }
            }
            for (auto it = component.rbegin(); it != component.rend(); ++it) {
                const int above_vertex = traversal_parent[static_cast<std::size_t>(*it)];
                if (above_vertex != -1)
                    subtree[static_cast<std::size_t>(above_vertex)] += subtree[static_cast<std::size_t>(*it)];
            }
            const int count = static_cast<int>(component.size());
            int centroid = start;
            for (;;) {
                int next = -1;
                for (const int to : graph[static_cast<std::size_t>(centroid)]) {
                    if (!removed[static_cast<std::size_t>(to)] &&
                        traversal_parent[static_cast<std::size_t>(to)] == centroid &&
                        subtree[static_cast<std::size_t>(to)] > count / 2) { next = to; break; }
                }
                if (next == -1) break;
                centroid = next;
            }
            parent_[static_cast<std::size_t>(centroid)] = above;
            depth_[static_cast<std::size_t>(centroid)] = above == -1 ? 0 : depth_[static_cast<std::size_t>(above)] + 1;
            component_size_[static_cast<std::size_t>(centroid)] = count;
            order_.push_back(centroid);
            if (above == -1) root_ = centroid;
            if (store_distances) {
                visits.clear(); visits.push_back({centroid, -1, 0});
                for (std::size_t i = 0; i < visits.size(); ++i) {
                    const auto [vertex, from, distance] = visits[i];
                    ancestors_[static_cast<std::size_t>(vertex)].push_back({centroid, distance});
                    for (const int to : graph[static_cast<std::size_t>(vertex)])
                        if (to != from && !removed[static_cast<std::size_t>(to)])
                            visits.push_back({to, vertex, distance + 1});
                }
            }
            removed[static_cast<std::size_t>(centroid)] = 1;
            for (const int to : graph[static_cast<std::size_t>(centroid)])
                if (!removed[static_cast<std::size_t>(to)]) pending.emplace_back(to, centroid);
        }
    }
public:
    CentroidDecomposition() = default;
    explicit CentroidDecomposition(const Graph& graph, bool store_distances = true) { initialize(graph, store_distances); }
    CentroidDecomposition(const CentroidDecomposition&) = default;
    CentroidDecomposition& operator=(const CentroidDecomposition& other) {
        if (this != &other) { CentroidDecomposition next(other); swap(next); }
        return *this;
    }
    CentroidDecomposition(CentroidDecomposition&& other) noexcept { swap(other); }
    CentroidDecomposition& operator=(CentroidDecomposition&& other) noexcept {
        CentroidDecomposition next(std::move(other)); swap(next); return *this;
    }
    void swap(CentroidDecomposition& other) noexcept {
        std::swap(root_, other.root_); std::swap(stores_distances_, other.stores_distances_);
        parent_.swap(other.parent_); depth_.swap(other.depth_); component_size_.swap(other.component_size_);
        order_.swap(other.order_); ancestors_.swap(other.ancestors_);
    }
    friend void swap(CentroidDecomposition& a, CentroidDecomposition& b) noexcept { a.swap(b); }
    void build(const Graph& graph, bool store_distances = true) {
        CentroidDecomposition next(graph, store_distances); swap(next);
    }
    void clear() noexcept {
        root_ = -1; stores_distances_ = false;
        parent_.clear(); depth_.clear(); component_size_.clear(); order_.clear(); ancestors_.clear();
    }
    std::size_t size() const noexcept { return parent_.size(); }
    bool empty() const noexcept { return parent_.empty(); }
    int root() const noexcept { return root_; }
    bool stores_distances() const noexcept { return stores_distances_; }
    int parent(int vertex) const { check_vertex(vertex); return parent_[static_cast<std::size_t>(vertex)]; }
    int depth(int vertex) const { check_vertex(vertex); return depth_[static_cast<std::size_t>(vertex)]; }
    int component_size(int vertex) const { check_vertex(vertex); return component_size_[static_cast<std::size_t>(vertex)]; }
    const std::vector<int>& parents() const noexcept { return parent_; }
    const std::vector<int>& depths() const noexcept { return depth_; }
    const std::vector<int>& order() const noexcept { return order_; }
    const std::vector<Ancestor>& ancestors(int vertex) const {
        check_vertex(vertex); check_distances(); return ancestors_[static_cast<std::size_t>(vertex)];
    }
    std::int64_t distance(int u, int v) const {
        const auto& a = ancestors(u); const auto& b = ancestors(v);
        std::size_t i = 0;
        while (i < a.size() && i < b.size() && a[i].vertex == b[i].vertex) ++i;
        return static_cast<std::int64_t>(a[i - 1].distance) + b[i - 1].distance;
    }
    Graph centroid_tree() const {
        Graph result(size());
        for (std::size_t vertex = 0; vertex < size(); ++vertex) {
            const int above = parent_[vertex];
            if (above != -1) {
                result[vertex].push_back(above);
                result[static_cast<std::size_t>(above)].push_back(static_cast<int>(vertex));
            }
        }
        return result;
    }
};
#endif

#ifndef SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED
#define SIRKIT_TREE_EDGE_COLOR_PATH_COUNTS_DEFINED

struct TreeColorEdge
{
    int from;
    int to;
    std::int64_t color;
};

struct TreeEdgeColorPathCounts
{
    std::uint64_t one_color{};
    std::uint64_t two_colors{};
};

[[nodiscard]] inline TreeEdgeColorPathCounts tree_edge_color_path_counts(
    int n, std::span<const TreeColorEdge> edges)
{
    if (n < 0 || edges.size() != static_cast<std::size_t>(n == 0 ? 0 : n - 1))
    {
        throw std::length_error("tree_edge_color_path_counts: inconsistent tree size");
    }

    std::vector<std::int64_t> colors;
    colors.reserve(edges.size());
    for (const auto &edge : edges)
    {
        colors.push_back(edge.color);
    }
    std::ranges::sort(colors);
    colors.erase(std::unique(colors.begin(), colors.end()), colors.end());

    Graph graph(n);
    std::vector<std::vector<std::pair<int, int>>> colored(n);
    for (const auto &[u, v, label] : edges)
    {
        const int color = static_cast<int>(std::lower_bound(colors.begin(), colors.end(), label) - colors.begin());
        graph[u].push_back(v);
        graph[v].push_back(u);
        colored[u].emplace_back(v, color);
        colored[v].emplace_back(u, color);
    }

    const CentroidDecomposition layout(graph, false);
    std::vector<bool> removed(n);
    std::vector<int> one(colors.size()), two_containing(colors.size()), touched;
    std::map<std::pair<int, int>, int> two;
    struct Visit
    {
        int vertex;
        int parent;
        int first;
        int second;
    };
    std::vector<Visit> pending;
    std::vector<std::pair<int, int>> branch;
    TreeEdgeColorPathCounts result;
    for (const int centroid : layout.order())
    {
        removed[centroid] = true;
        int one_total = 0;
        for (const auto &[start, color] : colored[centroid])
        {
            if (removed[start])
            {
                continue;
            }
            branch.clear();
            pending.push_back({start, centroid, color, -1});
            while (!pending.empty())
            {
                const auto [v, previous, first, second] = pending.back();
                pending.pop_back();
                branch.emplace_back(first, second);
                for (const auto &[u, next_color] : colored[v])
                {
                    if (u == previous || removed[u])
                    {
                        continue;
                    }
                    if (next_color == first || next_color == second)
                    {
                        pending.push_back({u, v, first, second});
                    }
                    else if (second == -1)
                    {
                        const auto [low, high] = std::minmax(first, next_color);
                        pending.push_back({u, v, low, high});
                    }
                }
            }

            for (const auto &[first, second] : branch)
            {
                if (second == -1)
                {
                    result.one_color += 1 + one[first];
                    result.two_colors += one_total - one[first] + two_containing[first];
                }
                else
                {
                    result.two_colors += 1 + one[first] + one[second] + two[{first, second}];
                }
            }
            for (const auto &[first, second] : branch)
            {
                touched.push_back(first);
                if (second == -1)
                {
                    ++one_total;
                    ++one[first];
                }
                else
                {
                    touched.push_back(second);
                    ++two_containing[first];
                    ++two_containing[second];
                    ++two[{first, second}];
                }
            }
        }
        for (const int color : touched)
        {
            one[color] = two_containing[color] = 0;
        }
        touched.clear();
        two.clear();
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
    int n{}, colors{};
    verify_in >> n >> colors;
    std::vector<sirkit::TreeColorEdge> edges(n - 1);
    for (auto &[u, v, color] : edges)
    {
        verify_in >> u >> v >> color;
        --u;
        --v;
    }
    verify_out << sirkit::tree_edge_color_path_counts(n, edges).two_colors << '\n';
}
