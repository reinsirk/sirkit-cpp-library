#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc414/tasks/abc414_g

// Library: Graph/Range_Edge_Graph.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Range_Edge_Graph.cpp
// 参考（セグメント木での区間辺の圧縮）: https://atcoder.jp/contests/abc414/editorial/13415
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/range_edge_graph.md
//
// API一覧
// RangeEdgeGraph(n): 頂点0..n-1の有向グラフ。定数の辺費用を使う層0を用意する。O(n)時間・領域。
// add_layer(departure,arrival): 頂点ごとの出発・到着費用を持つ層を追加し、層番号を返す。O(n)。
// add_range_edge(l,r,L,R,w,layer=0): [l,r)のuから[L,R)のvへ w+departure[u]+arrival[v] の辺を追加。
// 空区間があれば何もしない。補助頂点は高々2個、辺の追加O(log(n+1))、辺列拡張を含め償却。
// add_edge(u,v,w): 元の頂点間の有向辺を1本追加。償却O(1)。重みの型はstd::int64_t。
// size()/vertex_count(): 元の頂点数/補助頂点込みの頂点数。edges(): (始点,終点,重み)の辺列へのconst参照。各O(1)。
// graph(): WGraph形式に変換して返す。拡張後の頂点数Vと辺数Eに対しO(V+E)時間・領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRAPH_TYPES_DEFINED
#define SIRKIT_GRAPH_TYPES_DEFINED
struct Edge
{
    int to;
    std::int64_t w;
    Edge(int destination, std::int64_t weight) : to(destination), w(weight) {}
};
using WGraph = std::vector<std::vector<Edge>>;
#endif

#ifndef SIRKIT_RANGE_EDGE_GRAPH_DEFINED
#define SIRKIT_RANGE_EDGE_GRAPH_DEFINED

namespace detail
{
inline void check_range_edge_vertex_count(int current, std::size_t extra)
{
    if (extra > static_cast<std::size_t>(std::numeric_limits<int>::max() - current))
    {
        throw std::length_error("RangeEdgeGraph: too many vertices");
    }
}
}

class RangeEdgeGraph
{
public:
    using WeightedEdge = std::tuple<int, int, std::int64_t>;

    explicit RangeEdgeGraph(int n) : n_(n), vertex_count_(n)
    {
        if (n < 0)
        {
            throw std::length_error("RangeEdgeGraph: negative vertex count");
        }
        append_layer({}, {});
    }

    [[nodiscard]] int size() const noexcept
    {
        return n_;
    }

    [[nodiscard]] int vertex_count() const noexcept
    {
        return vertex_count_;
    }

    [[nodiscard]] const std::vector<WeightedEdge> &edges() const noexcept
    {
        return edges_;
    }

    std::size_t add_layer(std::span<const std::int64_t> departure, std::span<const std::int64_t> arrival)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (departure.size() != static_cast<std::size_t>(n_) || arrival.size() != static_cast<std::size_t>(n_))
        {
            throw std::invalid_argument("RangeEdgeGraph: cost layer size");
        }
#endif
        return append_layer(departure, arrival);
    }

    void add_edge(int u, int v, std::int64_t w)
    {
#ifndef SKIP_RUNTIME_CHECKS
        require_vertex(u);
        require_vertex(v);
#endif
        edges_.emplace_back(u, v, w);
    }

    void add_range_edge(int l, int r, int left, int right, std::int64_t w, std::size_t layer = 0)
    {
#ifndef SKIP_RUNTIME_CHECKS
        require_interval(l, r);
        require_interval(left, right);
        if (layer >= layers_.size())
        {
            throw std::out_of_range("RangeEdgeGraph: layer index");
        }
#endif
        if (l == r || left == right)
        {
            return;
        }
        const auto &costs = layers_[layer];
        Cover from_buffer, to_buffer;
        const auto from = cover(l, r, from_buffer);
        const auto to = cover(left, right, to_buffer);
        const auto first = minimum(costs.departure, from), second = minimum(costs.arrival, to);
        const auto middle = add_three(w, first, second);
        detail::check_range_edge_vertex_count(vertex_count_, 2);
        const int board = vertex_count_, arrive = board + 1;
        std::array<WeightedEdge, 2 * kCoverCapacity + 1> added;
        std::size_t count = 0;
        for (const int node : from)
        {
            added[count++] = {vertex(costs, node, true), board, costs.departure[node] - first};
        }
        added[count++] = {board, arrive, middle};
        for (const int node : to)
        {
            added[count++] = {arrive, vertex(costs, node, false), costs.arrival[node] - second};
        }
        edges_.insert(edges_.end(), added.begin(), added.begin() + count);
        vertex_count_ += 2;
    }

    [[nodiscard]] WGraph graph() const
    {
        WGraph result(vertex_count_);
        std::vector<std::size_t> degrees(vertex_count_);
        for (const auto &[from, to, cost] : edges_)
        {
            ++degrees[from];
        }
        for (int v = 0; v < vertex_count_; ++v)
        {
            result[v].reserve(degrees[v]);
        }
        for (const auto &[from, to, cost] : edges_)
        {
            result[from].emplace_back(to, cost);
        }
        return result;
    }

private:
    // 半開区間の分解では各高さから高々2頂点を使う。
    static constexpr std::size_t kCoverCapacity = std::numeric_limits<unsigned int>::digits * 2;
    using Cover = std::array<int, kCoverCapacity>;
    struct Layer
    {
        int base;
        std::vector<std::int64_t> departure;
        std::vector<std::int64_t> arrival;
    };

#ifndef SKIP_RUNTIME_CHECKS
    void require_vertex(int v) const
    {
        if (v < 0 || v >= n_)
        {
            throw std::out_of_range("RangeEdgeGraph: vertex index");
        }
    }

    void require_interval(int l, int r) const
    {
        if (l < 0 || r < l || r > n_)
        {
            throw std::out_of_range("RangeEdgeGraph: interval");
        }
    }
#endif

    static std::int64_t safe_add(std::int64_t a, std::int64_t b)
    {
        if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
            (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
        {
            throw std::overflow_error("RangeEdgeGraph: range edge cost out of range");
        }
        return a + b;
    }

    static std::int64_t add_three(std::int64_t a, std::int64_t b, std::int64_t c)
    {
        // 異符号の2項を先に足せば、中間結果だけがはみ出すことを避けられる。
        if ((a < 0) == (b < 0))
        {
            return safe_add(safe_add(a, c), b);
        }
        return safe_add(safe_add(a, b), c);
    }

    std::vector<std::int64_t> anchors(std::span<const std::int64_t> costs) const
    {
        std::vector<std::int64_t> result(static_cast<std::size_t>(n_) * 2);
        if (!costs.empty())
        {
            const auto [minimum_cost, maximum_cost] = std::minmax_element(costs.begin(), costs.end());
            const auto spread = static_cast<std::uint64_t>(*maximum_cost) - static_cast<std::uint64_t>(*minimum_cost);
            if (spread > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
            {
                throw std::overflow_error("RangeEdgeGraph: cost layer span out of range");
            }
            std::copy(costs.begin(), costs.end(), result.begin() + n_);
            for (int i = n_ - 1; i > 0; --i)
            {
                result[i] = std::min(result[2 * i], result[2 * i + 1]);
            }
        }
        return result;
    }

    int vertex(const Layer &layer, int node, bool upward) const noexcept
    {
        return node >= n_ ? node - n_ : layer.base + node - 1 + (upward ? 0 : n_ - 1);
    }

    std::size_t append_layer(std::span<const std::int64_t> departure, std::span<const std::int64_t> arrival)
    {
        const auto extra = 2 * static_cast<std::size_t>(std::max(n_ - 1, 0));
        detail::check_range_edge_vertex_count(vertex_count_, extra);
        Layer layer{vertex_count_, anchors(departure), anchors(arrival)};
        std::vector<WeightedEdge> added;
        added.reserve(2 * extra);
        for (int node = 2; node < 2 * n_; ++node)
        {
            const int parent = node / 2;
            added.emplace_back(vertex(layer, node, true), vertex(layer, parent, true),
                               layer.departure[node] - layer.departure[parent]);
            added.emplace_back(vertex(layer, parent, false), vertex(layer, node, false),
                               layer.arrival[node] - layer.arrival[parent]);
        }
        const auto needed_layers = layers_.size() + 1;
        if (needed_layers > layers_.capacity())
        {
            // Layerは2バイトより大きく、要素数の2倍はsize_tに収まる。
            const auto capacity = std::min(layers_.max_size(), 2 * (needed_layers - 1) + 1);
            layers_.reserve(std::max(needed_layers, capacity));
        }
        // 辺列の確保が失敗しても、層を公開する前なのでグラフは変わらない。
        edges_.insert(edges_.end(), added.begin(), added.end());
        layers_.push_back(std::move(layer));
        vertex_count_ += static_cast<int>(extra);
        return layers_.size() - 1;
    }

    std::span<const int> cover(int l, int r, Cover &output) const
    {
        std::size_t count = 0;
        for (l += n_, r += n_; l < r; l /= 2, r /= 2)
        {
            if (l & 1)
            {
                output[count++] = l++;
            }
            if (r & 1)
            {
                output[count++] = --r;
            }
        }
        return {output.data(), count};
    }

    static std::int64_t minimum(const std::vector<std::int64_t> &anchors, std::span<const int> interval_nodes)
    {
        auto result = std::numeric_limits<std::int64_t>::max();
        for (const int node : interval_nodes)
        {
            result = std::min(result, anchors[node]);
        }
        return result;
    }

    int n_, vertex_count_;
    std::vector<Layer> layers_;
    std::vector<WeightedEdge> edges_;
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
#include <functional>
#include <queue>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::int64_t> coordinate(n), negative(n);
    for (int i = 0; i < n; ++i)
    {
        verify_in >> coordinate[i];
        negative[i] = -coordinate[i];
    }
    const auto graph = [&] {
        sirkit::RangeEdgeGraph network(n);
        const auto east = network.add_layer(negative, coordinate);
        const auto west = network.add_layer(coordinate, negative);
        for (int i = 0; i < m; ++i)
        {
            int board_left{}, board_right{}, arrive_left{}, arrive_right{};
            std::int64_t fare{};
            verify_in >> board_left >> board_right >> arrive_left >> arrive_right >> fare;
            const auto direction = board_right < arrive_left ? east : west;
            network.add_range_edge(board_left - 1, board_right, arrive_left - 1, arrive_right, fare, direction);
        }
        return network.graph();
    }();

    constexpr std::int64_t kInfinity = std::int64_t{1} << 60;
    std::vector<std::int64_t> distance(graph.size(), kInfinity);
    using State = std::pair<std::int64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<>> queue;
    distance[0] = 0;
    queue.emplace(0, 0);
    while (!queue.empty())
    {
        const auto [current, v] = queue.top();
        queue.pop();
        if (current != distance[v])
        {
            continue;
        }
        for (const auto &edge : graph[v])
        {
            const auto next = current + edge.w;
            if (next < distance[edge.to])
            {
                distance[edge.to] = next;
                queue.emplace(next, edge.to);
            }
        }
    }
    for (int v = 1; v < n; ++v)
    {
        if (v > 1)
        {
            verify_out << ' ';
        }
        verify_out << (distance[v] == kInfinity ? -1 : distance[v]);
    }
    verify_out << '\n';
}
