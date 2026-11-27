#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/minimum_diameter_spanning_tree

// Library: Graph/Minimum_Diameter_Spanning_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Minimum_Diameter_Spanning_Tree.cpp
// 参考（絶対中心と最小直径全域木）: https://doi.org/10.1016/0020-0190(94)00183-Y
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_diameter_spanning_tree.md
//
// API一覧
// MinimumDiameterTreeEdge: 無向辺{from,to,weight}。頂点は[0,n)、重みは非負のint64_t。
// minimum_diameter_spanning_tree(n,edges): 最小直径全域木を{diameter,edge_ids}で返す。
// edge_idsは選んだ辺の入力添字。多重辺・重み0を扱い、自己辺は無視する。
// 空または非連結ならnullopt、1頂点では直径0、最適直径がint64_tの範囲外ならoverflow_error。
// O(n²+nm log(n+1))時間・O(n²+m)領域。頂点数は距離行列を確保できる範囲。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_DIAMETER_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_DIAMETER_SPANNING_TREE_DEFINED

struct MinimumDiameterTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

struct MinimumDiameterTreeResult
{
    std::int64_t diameter;
    std::vector<std::size_t> edge_ids;
};

namespace detail
{
using DiameterCost = std::uint64_t;
inline constexpr DiameterCost kDiameterOverflow = DiameterCost{1} << 63;
inline constexpr DiameterCost kDiameterUnreachable = std::numeric_limits<DiameterCost>::max();
inline constexpr std::size_t kDiameterNoEdge = std::numeric_limits<std::size_t>::max();

inline DiameterCost capped_diameter_add(DiameterCost a, DiameterCost b)
{
    return a >= kDiameterOverflow || b >= kDiameterOverflow - a ? kDiameterOverflow : a + b;
}

struct DiameterArc
{
    int to;
    std::size_t edge_id;
};

class DiameterHeap
{
    std::span<const DiameterCost> distance_;
    std::vector<int> vertices_;
    std::vector<std::size_t> position_;

    void put(std::size_t i, int v)
    {
        vertices_[i] = v;
        position_[v] = i;
    }

public:
    explicit DiameterHeap(std::size_t n) : position_(n, kDiameterNoEdge)
    {
        vertices_.reserve(n);
    }

    void reset(std::span<const DiameterCost> distance)
    {
        distance_ = distance;
        vertices_.clear();
        std::fill(position_.begin(), position_.end(), kDiameterNoEdge);
    }

    bool empty() const
    {
        return vertices_.empty();
    }

    void decrease(int v)
    {
        auto i = position_[v];
        if (i == kDiameterNoEdge)
        {
            i = vertices_.size();
            vertices_.push_back(v);
        }
        while (i != 0)
        {
            const auto parent = (i - 1) / 2;
            const int previous = vertices_[parent];
            if (distance_[previous] <= distance_[v])
            {
                break;
            }
            put(i, previous);
            i = parent;
        }
        put(i, v);
    }

    int pop()
    {
        const int top = vertices_.front(), last = vertices_.back();
        vertices_.pop_back();
        position_[top] = kDiameterNoEdge;
        if (!vertices_.empty())
        {
            std::size_t i = 0;
            for (std::size_t child = 1; child < vertices_.size(); child = i * 2 + 1)
            {
                const auto right = child + 1;
                if (right < vertices_.size() && distance_[vertices_[right]] < distance_[vertices_[child]])
                {
                    child = right;
                }
                const int next = vertices_[child];
                if (distance_[next] >= distance_[last])
                {
                    break;
                }
                put(i, next);
                i = child;
            }
            put(i, last);
        }
        return top;
    }
};

struct DiameterSeed
{
    int vertex;
    DiameterCost distance;
};

inline std::size_t diameter_dijkstra(
    const std::vector<std::vector<DiameterArc>> &graph, std::span<const MinimumDiameterTreeEdge> edges,
    DiameterHeap &heap, std::span<DiameterCost> distance, DiameterSeed first, DiameterSeed second,
    unsigned scale, std::span<int> order, std::span<std::size_t> parent)
{
    std::fill(distance.begin(), distance.end(), kDiameterUnreachable);
    std::fill(parent.begin(), parent.end(), kDiameterNoEdge);
    heap.reset(distance);
    distance[first.vertex] = first.distance;
    heap.decrease(first.vertex);
    if (second.vertex >= 0)
    {
        distance[second.vertex] = second.distance;
        heap.decrease(second.vertex);
    }
    std::size_t count = 0;
    while (!heap.empty())
    {
        const int from = heap.pop();
        if (!order.empty())
        {
            order[count] = from;
        }
        ++count;
        for (const auto &[to, edge_id] : graph[from])
        {
            const auto weight = static_cast<DiameterCost>(edges[edge_id].weight) * scale;
            const auto next = capped_diameter_add(distance[from], weight);
            if (next < distance[to])
            {
                distance[to] = next;
                if (!parent.empty())
                {
                    parent[to] = edge_id;
                }
                heap.decrease(to);
            }
        }
    }
    return count;
}

struct DiameterCenter
{
    DiameterCost diameter = kDiameterOverflow;
    int vertex = -1;
    std::size_t edge = kDiameterNoEdge;
    DiameterCost position_twice = 0;
};

inline void consider_diameter_edge(
    DiameterCenter &best, std::size_t id, DiameterCost weight, std::span<const DiameterCost> from,
    std::span<const DiameterCost> to, std::span<const int> order)
{
    auto right_max = to[order.back()];
    for (std::size_t i = order.size() - 1; i != 0; --i)
    {
        const int v = order[i - 1];
        if (to[v] <= right_max)
        {
            continue;
        }
        // 片端距離の降順で、もう片端距離が伸びる点だけが必要。
        // 隣接する候補の切替点間では、右からの直線と左からの直線が交わる点が最良。
        const auto right = weight + right_max;
        const auto diameter = capped_diameter_add(from[v], right);
        if (diameter < best.diameter)
        {
            best = {diameter, -1, id, right - from[v]};
        }
        right_max = to[v];
    }
}
} // namespace detail

inline std::optional<MinimumDiameterTreeResult> minimum_diameter_spanning_tree(
    int n, std::span<const MinimumDiameterTreeEdge> edges)
{
    if (n < 0)
    {
        throw std::length_error("minimum_diameter_spanning_tree: negative vertex count");
    }
    if (n == 0)
    {
        return std::nullopt;
    }
    const auto count = static_cast<std::size_t>(n);
    using Cost = detail::DiameterCost;
    constexpr auto kMaxCells = static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(Cost);
    if (count > kMaxCells / count)
    {
        throw std::length_error("minimum_diameter_spanning_tree: distance matrix too large");
    }

    std::vector<std::vector<detail::DiameterArc>> graph(count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        if (from != to)
        {
            graph[from].push_back({to, id});
            graph[to].push_back({from, id});
        }
    }
    detail::DiameterHeap heap(count);
    std::vector<Cost> distances(count * count);
    std::vector<int> orders(count * count);
    const auto row = [&](int v)
    {
        return std::span<Cost>(distances).subspan(static_cast<std::size_t>(v) * count, count);
    };
    const auto order = [&](int v)
    {
        return std::span<int>(orders).subspan(static_cast<std::size_t>(v) * count, count);
    };

    detail::DiameterCenter best;
    for (int v = 0; v < n; ++v)
    {
        const auto distance = row(v);
        const auto settled = order(v);
        const auto reached = detail::diameter_dijkstra(graph, edges, heap, distance, {v, 0}, {-1, 0}, 1, settled, {});
        if (reached != count)
        {
            return std::nullopt;
        }
        const auto radius = distance[settled.back()];
        if (radius == detail::kDiameterOverflow)
        {
            throw std::overflow_error("minimum_diameter_spanning_tree: graph distance does not fit int64_t");
        }
        const auto diameter = detail::capped_diameter_add(radius, radius);
        if (diameter < best.diameter)
        {
            best = {diameter, v, detail::kDiameterNoEdge, 0};
        }
    }
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto &[from, to, weight] = edges[id];
        if (from != to && weight != 0)
        {
            detail::consider_diameter_edge(best, id, static_cast<Cost>(weight), row(from), row(to), order(from));
        }
    }
    if (best.diameter == detail::kDiameterOverflow)
    {
        throw std::overflow_error("minimum_diameter_spanning_tree: minimum diameter does not fit int64_t");
    }

    std::vector<std::size_t> parent(count);
    MinimumDiameterTreeResult result{static_cast<std::int64_t>(best.diameter), {}};
    result.edge_ids.reserve(count - 1);
    if (best.edge == detail::kDiameterNoEdge)
    {
        detail::diameter_dijkstra(graph, edges, heap, row(0), {best.vertex, 0}, {-1, 0}, 1, {}, parent);
    }
    else
    {
        // 端点の中心より厳密に良いときだけこの経路に来るため、両端が仮想中心に直接つながる。
        const auto &[from, to, weight] = edges[best.edge];
        detail::diameter_dijkstra(graph, edges, heap, row(0), {from, best.position_twice},
                                  {to, 2 * static_cast<Cost>(weight) - best.position_twice}, 2, {}, parent);
        result.edge_ids.push_back(best.edge);
    }
    for (const auto id : parent)
    {
        if (id != detail::kDiameterNoEdge)
        {
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


#include <cstddef>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<MinimumDiameterTreeEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }
    const auto result = minimum_diameter_spanning_tree(n, edges);
    verify_out << result->diameter << '\n';
    for (std::size_t i = 0; i < result->edge_ids.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << result->edge_ids[i];
    }
    verify_out << '\n';
}
