#define SKIP_RUNTIME_CHECKS
// Verify: https://atcoder.jp/contests/abc291/tasks/abc291_h

// Library: Graph/Centroid_Decomposition.cpp
// SPDX-License-Identifier: CC0-1.0
// 数学的参考: https://nyaannyaan.github.io/library/tree/centroid-decomposition.hpp.html
// 参考: https://ei1333.github.io/library/graph/tree/centroid-decomposition.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/centroid_decomposition.md
//
// API一覧
// Graph: vector<vector<int>>。頂点0..n-1の単純無向木。n=0も扱う。入力は変更・保持しない。
// CentroidDecomposition()/CentroidDecomposition(graph,store_distances=true): 空/重心分解を構築。
// build(graph,store_distances=true): 再構築、失敗時は元の結果を維持。時間O(n log(n+1))。
// 距離を保存する場合は領域O(n log(n+1))、省く場合はO(n)。全探索を反復で行う。
// size()/empty()/root()/stores_distances(): 頂点数/空か/重心木の根（空なら-1）/距離保存の有無、O(1)。
// parent(v)/depth(v)/component_size(v): 重心木の親（根は-1）/深さ/除去時の成分サイズ、O(1)。
// parents()/depths()/order(): 親配列/深さ配列/親が先に現れる重心列へのconst参照、O(1)。
// Ancestor{int vertex; int distance;}: 祖先重心と元の木における辺数距離。
// ancestors(v): 重心木の根からvまでのAncestor列へのconst参照、O(1)。距離保存時だけ使う。
// distance(u,v): 元の木の距離をint64_tで返す、O(log(n+1))。距離保存が前提。
// centroid_tree(): 重心木の無向隣接リストを所有値で返す、O(n)。
// clear()/swap(other): 空にする/交換する。clearは格納要素数に比例、swapはO(1)。
// コピーは独立。move元は空。build/clear/代入により、以前の配列参照は無効となる。
// SKIP_RUNTIME_CHECKSで頂点・距離保存・初回の探索に組み込んだ木の入力検査を除く。
// 入力検査のためだけの全辺走査はしない。頂点数の表現上限は省略しない。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_CENTROID_DECOMPOSITION_DEFINED
#define SIRKIT_CENTROID_DECOMPOSITION_DEFINED
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
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
            throw std::out_of_range("CentroidDecomposition vertex");
#else
        static_cast<void>(vertex);
#endif
    }
    void check_distances() const {
#ifndef SKIP_RUNTIME_CHECKS
        if (!stores_distances_) throw std::logic_error("CentroidDecomposition distances not stored");
#endif
    }
    void initialize(const Graph& graph, bool store_distances) {
        const auto n = graph.size();
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("CentroidDecomposition vertex count");
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
#ifndef SKIP_RUNTIME_CHECKS
            const bool validate = above == -1;
#endif
            component.clear(); component.push_back(start);
            traversal_parent[static_cast<std::size_t>(start)] = -1;
            for (std::size_t i = 0; i < component.size(); ++i) {
                const int vertex = component[i];
                subtree[static_cast<std::size_t>(vertex)] = 1;
#ifndef SKIP_RUNTIME_CHECKS
                std::size_t parent_edges = 0;
#endif
                for (const int to : graph[static_cast<std::size_t>(vertex)]) {
#ifndef SKIP_RUNTIME_CHECKS
                    if (validate && (to < 0 || static_cast<std::size_t>(to) >= n || to == vertex))
                        throw std::invalid_argument("CentroidDecomposition tree edge");
#endif
                    if (to == traversal_parent[static_cast<std::size_t>(vertex)]) {
#ifndef SKIP_RUNTIME_CHECKS
                        if (validate) ++parent_edges;
#endif
                        continue;
                    }
                    if (removed[static_cast<std::size_t>(to)]) continue;
#ifndef SKIP_RUNTIME_CHECKS
                    if (validate && traversal_parent[static_cast<std::size_t>(to)] != -2)
                        throw std::invalid_argument("CentroidDecomposition input is not a tree");
#endif
                    traversal_parent[static_cast<std::size_t>(to)] = vertex;
                    component.push_back(to);
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (validate && vertex != start && parent_edges != 1)
                    throw std::invalid_argument("CentroidDecomposition asymmetric tree");
#endif
            }
#ifndef SKIP_RUNTIME_CHECKS
            if (validate && component.size() != n)
                throw std::invalid_argument("CentroidDecomposition disconnected tree");
#endif
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
int main() {
    VerifyInput input;
    VerifyOutput output;
    int n = 0; input >> n;
    Graph graph(static_cast<std::size_t>(n));
    for (int i = 0; i < n - 1; ++i) {
        int u = 0, v = 0; input >> u >> v; --u; --v;
        graph[static_cast<std::size_t>(u)].push_back(v);
        graph[static_cast<std::size_t>(v)].push_back(u);
    }
    CentroidDecomposition decomposition(graph, false);
    for (int vertex = 0; vertex < n; ++vertex) {
        if (vertex != 0) output << ' ';
        const int p = decomposition.parent(vertex);
        output << (p == -1 ? -1 : p + 1);
    }
    output << '\n';
}
