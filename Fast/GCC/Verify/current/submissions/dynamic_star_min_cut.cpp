#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/global_minimum_cut_of_dynamic_star_augmented_graph

// Library: Graph/Dynamic_Star_Min_Cut.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Dynamic_Star_Min_Cut.cpp
// 参考（極点集合）: https://doi.org/10.1007/978-3-540-77120-3_4
// 参考（スターの辺を更新する最小カット）: https://doi.org/10.1093/ietisy/e90-d.2.428
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dynamic_star_min_cut.md
//
// API一覧
// DynamicStarMinCut(n,edges,star={}): 無向グラフに頂点nを加え、各頂点vと重みstar[v]の辺で結ぶ。
// DynamicStarCutEdge{u,v,weight}: 元の無向辺。平行辺は合算し、自己辺を無視する。starが空なら全部0。
// n>=1、元の辺（自己辺を除く）とスターの重み合計はstd::uint64_tに収まること。
// set_star_weight(v,weight): スターの辺(v,n)を更新し、全域最小カットの重みを返す。O(log^2(n+1))。
// minimum_cut()/star_weight(v)/size(): 最小カットの重み/スターの辺重み/元の頂点数。各O(1)。
// 構築はO(n(n+m)log(n+1))時間・O(n+m)一時領域。保持領域O(n)。
// コピーはO(n)で重みを独立に保持する。ムーブ構築はO(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DYNAMIC_STAR_MIN_CUT_DEFINED
#define SIRKIT_DYNAMIC_STAR_MIN_CUT_DEFINED

struct DynamicStarCutEdge
{
    int u;
    int v;
    std::uint64_t weight;
};

namespace detail
{
inline int star_cut_vertex_count(int n)
{
    if (n <= 0 || n > std::numeric_limits<int>::max() / 2)
    {
        throw std::length_error("DynamicStarMinCut: vertex count");
    }
    return n;
}

inline std::size_t star_cut_arc_slots(std::size_t edges, std::size_t capacity)
{
    if (edges > capacity / 2)
    {
        throw std::length_error("DynamicStarMinCut: too many edges");
    }
    return 2 * edges;
}

inline std::uint64_t star_cut_checked_add(std::uint64_t sum, std::uint64_t weight)
{
    if (weight > std::numeric_limits<std::uint64_t>::max() - sum)
    {
        throw std::overflow_error("DynamicStarMinCut: total edge weight out of range");
    }
    return sum + weight;
}

class StarCutDegreeHeap
{
public:
    explicit StarCutDegreeHeap(int n) : position_(n), key_(n)
    {
        vertices_.reserve(n);
    }

    void reset(const std::vector<int> &active, const std::vector<std::uint64_t> &degree)
    {
        vertices_ = active;
        key_ = degree;
        for (std::size_t i = 0; i < vertices_.size(); ++i)
        {
            position_[vertices_[i]] = static_cast<int>(i);
        }
        for (std::size_t i = vertices_.size() / 2; i != 0; )
        {
            sift_down(--i);
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return vertices_.size();
    }

    int remove_minimum()
    {
        const int removed = vertices_.front();
        position_[removed] = -1;
        vertices_.front() = vertices_.back();
        vertices_.pop_back();
        sift_down(0);
        return removed;
    }

    void subtract_weight(int vertex, std::uint64_t weight)
    {
        const int position = position_[vertex];
        if (position < 0)
        {
            return;
        }
        key_[vertex] -= weight;
        auto i = static_cast<std::size_t>(position);
        while (i != 0)
        {
            const auto parent = (i - 1) / 2;
            if (key_[vertices_[parent]] <= key_[vertex])
            {
                break;
            }
            place(i, vertices_[parent]);
            i = parent;
        }
        place(i, vertex);
    }

    [[nodiscard]] std::pair<int, int> final_pair() const noexcept
    {
        return {vertices_[0], vertices_[1]};
    }

    [[nodiscard]] std::uint64_t minimum_degree() const noexcept
    {
        return key_[vertices_.front()];
    }

private:
    void place(std::size_t i, int vertex) noexcept
    {
        vertices_[i] = vertex;
        position_[vertex] = static_cast<int>(i);
    }

    void sift_down(std::size_t i)
    {
        const int vertex = vertices_[i];
        for (auto child = 2 * i + 1; child < vertices_.size(); child = 2 * i + 1)
        {
            if (child + 1 < vertices_.size() && key_[vertices_[child + 1]] < key_[vertices_[child]])
            {
                ++child;
            }
            if (key_[vertex] <= key_[vertices_[child]])
            {
                break;
            }
            place(i, vertices_[child]);
            i = child;
        }
        place(i, vertex);
    }

    std::vector<int> vertices_, position_;
    std::vector<std::uint64_t> key_;
};

struct StarCutTreeNode
{
    int left = -1;
    int right = -1;
    std::uint64_t boundary = 0;
};

inline std::vector<StarCutTreeNode> make_star_cut_tree(int n, std::span<const DynamicStarCutEdge> edges)
{
    const auto count = static_cast<std::size_t>(2 * n - 1);
    std::vector<StarCutTreeNode> tree(count);
    std::vector<int> component(n), tree_node(n), active(n);
    std::iota(component.begin(), component.end(), 0);
    std::iota(tree_node.begin(), tree_node.end(), 0);
    std::iota(active.begin(), active.end(), 0);
    StarCutDegreeHeap heap(n);
    struct Arc
    {
        int to;
        std::size_t next;
        std::uint64_t weight;
    };
    constexpr auto kNoArc = std::numeric_limits<std::size_t>::max();
    std::vector<std::size_t> first(n);
    std::vector<Arc> arcs;
    arcs.reserve(star_cut_arc_slots(edges.size(), arcs.max_size()));
    std::vector<std::uint64_t> degree(n);
    const auto add_arc = [&](int u, int v, std::uint64_t weight)
    {
        const auto index = arcs.size();
        arcs.push_back({v, first[u], weight});
        first[u] = index;
        degree[u] += weight;
    };
    for (const auto &[u, v, weight] : edges)
    {
        if (u != v)
        {
            tree[u].boundary += weight;
            tree[v].boundary += weight;
        }
    }

    for (std::size_t node = n; node < count; ++node)
    {
        std::fill(first.begin(), first.end(), kNoArc);
        std::fill(degree.begin(), degree.end(), 0);
        arcs.clear();
        for (const auto &[u, v, weight] : edges)
        {
            const auto a = component[u], b = component[v];
            if (a != b && weight != 0)
            {
                add_arc(a, b, weight);
                add_arc(b, a, weight);
            }
        }
        heap.reset(active, degree);
        // 現在の最小次数頂点を消す操作を繰り返す。最後の2頂点の縮約は極点集合を失わない。
        while (heap.size() > 2)
        {
            const int removed = heap.remove_minimum();
            for (auto i = first[removed]; i != kNoArc; i = arcs[i].next)
            {
                heap.subtract_weight(arcs[i].to, arcs[i].weight);
            }
        }
        const auto [a, b] = heap.final_pair();
        const auto common = heap.minimum_degree();
        tree[node] = {tree_node[a], tree_node[b], (degree[a] - common) + (degree[b] - common)};
        tree_node[a] = static_cast<int>(node);
        for (auto &owner : component)
        {
            if (owner == b)
            {
                owner = a;
            }
        }
        active.erase(std::find(active.begin(), active.end(), b));
    }
    return tree;
}
}

class DynamicStarMinCut
{
public:
    DynamicStarMinCut(int n, std::span<const DynamicStarCutEdge> edges,
                      std::span<const std::uint64_t> star = {})
        : star_(detail::star_cut_vertex_count(n))
    {
        for (const auto &[u, v, weight] : edges)
        {
            if (u != v)
            {
                base_total_ = detail::star_cut_checked_add(base_total_, weight);
            }
        }
        if (!star.empty())
        {
            std::copy(star.begin(), star.end(), star_.begin());
        }
        for (const auto weight : star_)
        {
            star_total_ = detail::star_cut_checked_add(star_total_, weight);
        }
        static_cast<void>(detail::star_cut_checked_add(base_total_, star_total_));
        build_paths(detail::make_star_cut_tree(n, edges));
    }

    DynamicStarMinCut(const DynamicStarMinCut &) = default;
    DynamicStarMinCut(DynamicStarMinCut &&) noexcept = default;

    DynamicStarMinCut &operator=(DynamicStarMinCut other) noexcept
    {
        star_.swap(other.star_);
        parent_.swap(other.parent_);
        head_.swap(other.head_);
        positions_.swap(other.positions_);
        minimum_.swap(other.minimum_);
        lazy_.swap(other.lazy_);
        std::swap(base_total_, other.base_total_);
        std::swap(star_total_, other.star_total_);
        std::swap(segment_base_, other.segment_base_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return star_.size();
    }

    [[nodiscard]] std::uint64_t star_weight(int v) const
    {
        check_vertex(v);
        return star_[v];
    }

    [[nodiscard]] std::uint64_t minimum_cut() const noexcept
    {
        return minimum_.empty() ? 0 : minimum_[1];
    }

    std::uint64_t set_star_weight(int v, std::uint64_t weight)
    {
        check_vertex(v);
        const auto old = star_[v];
        if (old == weight)
        {
            return minimum_cut();
        }
        const auto remaining = star_total_ - old;
        static_cast<void>(detail::star_cut_checked_add(base_total_ + remaining, weight));
        star_[v] = weight;
        star_total_ = remaining + weight;
        // 各値は範囲内。符号付き差分の代わりに2^64を法とする加算を使う。
        const auto delta = weight - old;
        while (v != -1)
        {
            const int chain = head_[v];
            range_add(1, 0, segment_base_, positions_[chain], positions_[v] + 1, delta);
            v = parent_[chain];
        }
        return minimum_[1];
    }

private:
    void check_vertex(int v) const
    {
        static_cast<void>(v);
    }

    void build_paths(std::vector<detail::StarCutTreeNode> tree)
    {
        const auto n = size(), count = tree.size();
        parent_.assign(count, -1);
        head_.resize(count);
        positions_.resize(count);
        std::vector<int> leaves(count, 1);
        std::vector<std::uint64_t> incident(count);
        std::copy(star_.begin(), star_.end(), incident.begin());
        for (std::size_t v = n; v < count; ++v)
        {
            auto &node = tree[v];
            if (leaves[node.left] < leaves[node.right])
            {
                std::swap(node.left, node.right);
            }
            parent_[node.left] = parent_[node.right] = static_cast<int>(v);
            leaves[v] = leaves[node.left] + leaves[node.right];
            incident[v] = incident[node.left] + incident[node.right];
        }
        segment_base_ = std::bit_ceil(count);
        minimum_.assign(2 * segment_base_, std::numeric_limits<std::uint64_t>::max());
        lazy_.resize(2 * segment_base_);
        std::vector<int> pending{static_cast<int>(count - 1)};
        std::size_t pos = 0;
        while (!pending.empty())
        {
            const int head = pending.back();
            pending.pop_back();
            for (int v = head; v != -1; v = tree[v].left)
            {
                head_[v] = head;
                positions_[v] = pos;
                minimum_[segment_base_ + pos++] = tree[v].boundary + incident[v];
                if (tree[v].right != -1)
                {
                    pending.push_back(tree[v].right);
                }
            }
        }
        for (std::size_t v = segment_base_; v != 1; )
        {
            --v;
            minimum_[v] = std::min(minimum_[2 * v], minimum_[2 * v + 1]);
        }
    }

    void add(std::size_t node, std::uint64_t delta) noexcept
    {
        minimum_[node] += delta;
        lazy_[node] += delta;
    }

    void range_add(std::size_t node, std::size_t left, std::size_t right,
                   std::size_t q_left, std::size_t q_right, std::uint64_t delta) noexcept
    {
        if (q_left <= left && right <= q_right)
        {
            add(node, delta);
            return;
        }
        if (const auto tag = lazy_[node]; tag != 0)
        {
            add(2 * node, tag);
            add(2 * node + 1, tag);
            lazy_[node] = 0;
        }
        const auto middle = left + (right - left) / 2;
        if (q_left < middle)
        {
            range_add(2 * node, left, middle, q_left, q_right, delta);
        }
        if (middle < q_right)
        {
            range_add(2 * node + 1, middle, right, q_left, q_right, delta);
        }
        minimum_[node] = std::min(minimum_[2 * node], minimum_[2 * node + 1]);
    }

    std::vector<std::uint64_t> star_;
    std::uint64_t base_total_ = 0, star_total_ = 0;
    std::vector<int> parent_, head_;
    std::vector<std::size_t> positions_;
    std::size_t segment_base_ = 1;
    std::vector<std::uint64_t> minimum_, lazy_;
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
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::uint64_t> star(n);
    for (auto &weight : star)
    {
        verify_in >> weight;
    }
    std::vector<sirkit::DynamicStarCutEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
    }
    sirkit::DynamicStarMinCut cut(n, edges, star);
    for (int i = 0; i < q; ++i)
    {
        int v{};
        std::uint64_t weight{};
        verify_in >> v >> weight;
        verify_out << cut.set_star_weight(v, weight) << '\n';
    }
}
