#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc351/tasks/abc351_g

// Library: Graph/Static_Top_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Static_Top_Tree.cpp
// 参考（静的トップ木と頂点数での重み付け）: https://atcoder.jp/contests/abc351/editorial/9899
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_top_tree.md
//
// API一覧
// StaticTopTree<Policy>(values, edges, root=0, policy={}): 0始まりの空でない無向木を構築する。
// Policy::Vertex/Path/Point: 頂点値、根に向かうパスの要約、枝の要約の型。
// policy.vertex(value, branches): 頂点と横の枝をパスにする。枝がなければbranchesはnullptr。
// policy.project(path): 完成したパスを親へつながる枝に変換する。
// policy.rake(left, right): 横の枝を結合する。結合則と交換則が必要。
// policy.compress(upper, lower): 上側と下側のパスを結合する。結合則が必要。
// 各コールバックはconstかつnoexcept。頂点値・要約のムーブ代入もnoexcept。
// size(): 頂点数。O(1)。set(v, value): 頂点値を置き換える。O(log(n+1))。
// all_prod(): 根を含むパス全体の要約へのconst参照。O(1)。必要ならproject相当の変換を呼ぶ。
// 各コールバックがO(1)なら、構築O(n log(n+1))時間、領域O(n)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_TOP_TREE_DEFINED
#define SIRKIT_STATIC_TOP_TREE_DEFINED

namespace detail
{
inline void check_static_top_tree_capacity(std::size_t n)
{
    // 頂点・重い辺・軽い辺を合わせて2n-1個。横の枝同士の結合を足しても3n未満。
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max() / 3))
    {
        throw std::length_error("StaticTopTree: too many vertices");
    }
}
}

template<class Policy>
class StaticTopTree
{
public:
    using Vertex = typename Policy::Vertex;
    using Path = typename Policy::Path;
    using Point = typename Policy::Point;
    using Edge = std::pair<int, int>;

    StaticTopTree(std::span<const Vertex> values, std::span<const Edge> edges, int root = 0, Policy policy = {})
        : policy_(std::move(policy))
    {
        const std::size_t n = values.size();
        detail::check_static_top_tree_capacity(n);
        std::vector<std::size_t> offset(n + 1);
        for (const auto &[u, v] : edges)
        {
            ++offset[static_cast<std::size_t>(u) + 1];
            ++offset[static_cast<std::size_t>(v) + 1];
        }
        for (std::size_t i = 0; i < n; ++i)
        {
            offset[i + 1] += offset[i];
        }
        std::vector<int> adjacency(offset.back());
        auto next = offset;
        for (const auto &[u, v] : edges)
        {
            adjacency[next[u]++] = v;
            adjacency[next[v]++] = u;
        }

        std::vector<int> parent(n, -1), order{root};
        order.reserve(n);
        parent[root] = root;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int v = order[i];
            for (std::size_t j = offset[v]; j < offset[v + 1]; ++j)
            {
                const int u = adjacency[j];
                if (u == parent[v])
                {
                    continue;
                }
                parent[u] = v;
                order.push_back(u);
            }
        }
        std::vector<int> weight(n, 1), heavy(n, -1);
        int heavy_edges = 0;
        for (std::size_t i = n; i > 1; --i)
        {
            const int v = order[i - 1], p = parent[v];
            weight[p] += weight[v];
            if (heavy[p] == -1)
            {
                ++heavy_edges;
                heavy[p] = v;
            }
            else if (weight[v] > weight[heavy[p]])
            {
                heavy[p] = v;
            }
        }

        vertices_ = std::vector<Vertex>(values.begin(), values.end());
        leaf_.resize(n);
        const std::size_t path_count = n + heavy_edges;
        const std::size_t light_edges = n - 1 - heavy_edges;
        paths_.reserve(path_count);
        points_.reserve(2 * light_edges);
        nodes_.reserve(path_count + 2 * light_edges);
        std::vector<int> path_top(n);
        std::vector<Weighted> branches, chain;
        for (std::size_t i = n; i > 0; --i)
        {
            const int top = order[i - 1];
            if (top != root && heavy[parent[top]] == top)
            {
                continue;
            }
            chain.clear();
            for (int v = top; v != -1; v = heavy[v])
            {
                branches.clear();
                for (std::size_t j = offset[v]; j < offset[v + 1]; ++j)
                {
                    const int u = adjacency[j];
                    if (u != parent[v] && u != heavy[v])
                    {
                        const int branch = Append(Kind::kProject, path_top[u], -1);
                        AppendWeighted(branches, branch, weight[u]);
                    }
                }
                const int side = branches.empty() ? -1 : Join(branches, 0, branches.size(), Kind::kRake);
                leaf_[v] = Append(Kind::kVertex, v, side);
                const int weight_below = heavy[v] == -1 ? 0 : weight[heavy[v]];
                AppendWeighted(chain, leaf_[v], weight[v] - weight_below);
            }
            path_top[top] = Join(chain, 0, chain.size(), Kind::kCompress);
        }
    }

    StaticTopTree(const StaticTopTree &) = default;
    StaticTopTree(StaticTopTree &&) noexcept = default;

    StaticTopTree &operator=(const StaticTopTree &other)
    {
        return *this = StaticTopTree(other);
    }

    StaticTopTree &operator=(StaticTopTree &&) noexcept = default;

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(vertices_.size());
    }

    void set(int v, Vertex value)
    {
        vertices_[v] = std::move(value);
        for (int node = leaf_[v]; node != -1; node = nodes_[node].parent)
        {
            Recompute<false>(nodes_[node]);
        }
    }

    [[nodiscard]] const Path &all_prod() const
    {
        return paths_.back().value;
    }

private:
    static_assert(std::is_nothrow_move_constructible_v<Policy> && std::is_nothrow_move_assignable_v<Policy>);
    static_assert(std::is_nothrow_move_assignable_v<Vertex> &&
                  std::is_nothrow_move_assignable_v<Path> && std::is_nothrow_move_assignable_v<Point>);
    static_assert(noexcept(std::declval<const Policy &>().vertex(std::declval<const Vertex &>(), std::declval<const Point *>())));
    static_assert(noexcept(std::declval<const Policy &>().project(std::declval<const Path &>())));
    static_assert(noexcept(std::declval<const Policy &>().rake(std::declval<const Point &>(), std::declval<const Point &>())));
    static_assert(noexcept(std::declval<const Policy &>().compress(std::declval<const Path &>(), std::declval<const Path &>())));

    enum class Kind : unsigned char
    {
        kVertex,
        kProject,
        kRake,
        kCompress,
    };

    struct Cluster
    {
        int left, right, parent, state;
        Kind kind;
    };

    struct Weighted
    {
        int cluster, cumulative;
    };

    template<class State>
    struct Cell
    {
        State value;
    };

    [[no_unique_address]] Policy policy_;
    std::vector<Vertex> vertices_;
    std::vector<int> leaf_;
    std::vector<Cluster> nodes_;
    std::vector<Cell<Path>> paths_;
    std::vector<Cell<Point>> points_;

    static void AppendWeighted(std::vector<Weighted> &items, int cluster, int weight)
    {
        const int previous = items.empty() ? 0 : items.back().cumulative;
        items.push_back({cluster, previous + weight});
    }

    int Join(std::span<const Weighted> items, std::size_t first, std::size_t last, Kind kind)
    {
        if (last - first == 1)
        {
            return items[first].cluster;
        }
        // 中央を含む要素を除くと、左右とも元の重みの半分以下になる。
        const int base = first == 0 ? 0 : items[first - 1].cumulative;
        const int target = base + (items[last - 1].cumulative - base + 1) / 2;
        const auto median = std::lower_bound(items.begin() + first, items.begin() + last, target,
                                            [](const Weighted &item, int value) { return item.cumulative < value; });
        const auto middle = static_cast<std::size_t>(median - items.begin());
        int cluster = median->cluster;
        if (first != middle)
        {
            const int left = Join(items, first, middle, kind);
            cluster = Append(kind, left, cluster);
        }
        if (middle + 1 != last)
        {
            const int right = Join(items, middle + 1, last, kind);
            cluster = Append(kind, cluster, right);
        }
        return cluster;
    }

    int Append(Kind kind, int left, int right)
    {
        const int index = static_cast<int>(nodes_.size());
        const auto state = static_cast<int>(kind == Kind::kProject || kind == Kind::kRake ? points_.size() : paths_.size());
        nodes_.push_back({left, right, -1, state, kind});
        if (kind != Kind::kVertex)
        {
            nodes_[left].parent = index;
        }
        if (right != -1)
        {
            nodes_[right].parent = index;
        }
        Recompute<true>(nodes_[index]);
        return index;
    }

    template<bool AppendState, class State>
    static void Store(std::vector<Cell<State>> &states, int index, State value) noexcept(!AppendState)
    {
        if constexpr (AppendState)
        {
            states.push_back({std::move(value)});
        }
        else
        {
            states[index].value = std::move(value);
        }
    }

    const Path &PathState(int cluster) const noexcept
    {
        return paths_[nodes_[cluster].state].value;
    }

    const Point &PointState(int cluster) const noexcept
    {
        return points_[nodes_[cluster].state].value;
    }

    template<bool AppendState>
    void Recompute(const Cluster &cluster) noexcept(!AppendState)
    {
        const Policy &policy = policy_;
        if (cluster.kind == Kind::kVertex)
        {
            const Point *side = cluster.right == -1 ? nullptr : &PointState(cluster.right);
            Store<AppendState>(paths_, cluster.state, policy.vertex(std::as_const(vertices_)[cluster.left], side));
        }
        else if (cluster.kind == Kind::kProject)
        {
            Store<AppendState>(points_, cluster.state, policy.project(PathState(cluster.left)));
        }
        else if (cluster.kind == Kind::kRake)
        {
            Store<AppendState>(points_, cluster.state, policy.rake(PointState(cluster.left), PointState(cluster.right)));
        }
        else
        {
            Store<AppendState>(paths_, cluster.state, policy.compress(PathState(cluster.left), PathState(cluster.right)));
        }
    }
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
struct HashOperations
{
    using Vertex = std::uint64_t;
    using Point = std::uint64_t;
    struct Path
    {
        std::uint64_t mul;
        std::uint64_t add;
    };
    static constexpr std::uint64_t kMod = 998244353;

    Path vertex(Vertex value, const Point *branches) const noexcept
    {
        return {branches ? *branches : 1, value};
    }

    Point project(const Path &path) const noexcept
    {
        return path.add;
    }

    Point rake(Point left, Point right) const noexcept
    {
        return left * right % kMod;
    }

    Path compress(const Path &upper, const Path &lower) const noexcept
    {
        return {upper.mul * lower.mul % kMod, (upper.mul * lower.add + upper.add) % kMod};
    }
};
}

int main()
{
    int n{}, q{};
    verify_in >> n >> q;
    std::vector<std::pair<int, int>> edges;
    edges.reserve(n - 1);
    for (int v = 1; v < n; ++v)
    {
        int parent{};
        verify_in >> parent;
        edges.emplace_back(parent - 1, v);
    }
    std::vector<std::uint64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::StaticTopTree<HashOperations> tree(values, edges);
    for (int i = 0; i < q; ++i)
    {
        int v{};
        std::uint64_t value{};
        verify_in >> v >> value;
        tree.set(v - 1, value);
        verify_out << tree.all_prod().add << '\n';
    }
}
