#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/1320

// Library: Graph/Shortest_Weighted_Cycle.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_weighted_cycle.md
//
// API一覧
// shortest_weighted_cycle(n, edges, directed=true, through=std::nullopt):
//   重みの和が最小の単純閉路を返す。throughを指定するとその頂点を含む閉路だけを調べる。
//   WeightedCycleEdge{from,to,weight=1}。重みは非負のuint64_t。自己辺と多重辺も扱う。
//   閉路がないか、重みの和がuint64_tの上限を超えるとnullopt。
//   1始点: O((n+m)log(n+m+1))時間。全頂点: O(n(n+m)log(n+m+1))時間。領域O(n+m+1)。
// WeightedCycle{weight,cycle}: 重みの和と、巡回順の頂点列・入力の辺番号列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CYCLE_DETECTION_DEFINED
#define SIRKIT_CYCLE_DETECTION_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t cycle_arc_count(std::size_t edges, std::size_t directions, std::size_t limit)
{
    if (edges > limit / directions)
    {
        throw std::length_error("find_cycle: too many edges");
    }
    return edges * directions;
}
} // namespace detail

struct Cycle
{
    std::vector<int> vertices;
    std::vector<std::size_t> edge_ids;
};

inline std::optional<Cycle> find_cycle(int n, std::span<const std::pair<int, int>> edges, bool directed = true)
{
    if (n < 0)
    {
        throw std::length_error("find_cycle: negative vertex count");
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    const auto directions = directed ? 1U : 2U;
    const auto arc_count = detail::cycle_arc_count(edges.size(), directions, std::vector<Arc>{}.max_size());

    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    for (const auto &[u, v] : edges)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("find_cycle: vertex out of range");
        }
#endif
        ++offset[u];
        if (!directed)
        {
            ++offset[v];
        }
    }
    std::partial_sum(offset.begin(), offset.end(), offset.begin());
    std::vector<Arc> adjacency(arc_count);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [u, v] = edges[id];
        adjacency[--offset[u]] = {v, id};
        if (!directed)
        {
            adjacency[--offset[v]] = {u, id};
        }
    }

    struct Frame
    {
        int vertex;
        std::size_t next;
        std::size_t incoming;
    };
    constexpr int kUnseen = -1;
    constexpr int kFinished = -2;
    std::vector<int> position(n, kUnseen);
    std::vector<Frame> stack;
    for (int start = 0; start < n; ++start)
    {
        if (position[start] != kUnseen)
        {
            continue;
        }
        position[start] = 0;
        stack.push_back({start, offset[start], edges.size()});
        while (!stack.empty())
        {
            auto &frame = stack.back();
            if (frame.next == offset[frame.vertex + 1])
            {
                position[frame.vertex] = kFinished;
                stack.pop_back();
                continue;
            }
            const auto [to, id] = adjacency[frame.next++];
            if (!directed && id == frame.incoming)
            {
                continue;
            }
            if (position[to] == kUnseen)
            {
                position[to] = static_cast<int>(stack.size());
                stack.push_back({to, offset[to], id});
            }
            else if (position[to] >= 0)
            {
                const auto first = static_cast<std::size_t>(position[to]);
                Cycle result;
                result.vertices.reserve(stack.size() - first);
                result.edge_ids.reserve(stack.size() - first);
                result.vertices.push_back(to);
                for (std::size_t i = first + 1; i < stack.size(); ++i)
                {
                    result.vertices.push_back(stack[i].vertex);
                    result.edge_ids.push_back(stack[i].incoming);
                }
                result.edge_ids.push_back(id);
                return result;
            }
        }
    }
    return std::nullopt;
}

#endif

#ifndef SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED
#define SIRKIT_SHORTEST_WEIGHTED_CYCLE_DEFINED

struct WeightedCycleEdge
{
    int from;
    int to;
    std::uint64_t weight = 1;
};

struct WeightedCycle
{
    std::uint64_t weight;
    Cycle cycle;
};

namespace detail
{
struct WeightedCycleArc
{
    int to;
    std::size_t id;
    std::uint64_t weight;
};

using WeightedCycleGraph = std::vector<std::vector<WeightedCycleArc>>;

inline std::optional<WeightedCycle> shortest_weighted_cycle_from(
    const WeightedCycleGraph &graph, int source, std::size_t edge_count, bool directed,
    std::optional<std::uint64_t> limit)
{
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    struct Connection
    {
        int from;
        int to;
        std::size_t id;
        std::uint64_t cost;
    };
    std::optional<Connection> best;
    auto consider = [&](int from, int to, std::size_t id, std::uint64_t cost)
    {
        if (!limit || cost < *limit)
        {
            best = {from, to, id, cost};
            limit = cost;
        }
    };

    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    std::vector<int> parent(graph.size(), -1);
    std::vector<std::size_t> incoming(graph.size());
    std::vector<std::size_t> first(graph.size(), edge_count);
    std::vector<bool> settled(graph.size());
    using State = std::pair<std::uint64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> queue;
    distance[source] = 0;
    queue.emplace(0, source);

    while (!queue.empty())
    {
        const auto [prefix, from] = queue.top();
        queue.pop();
        if (prefix != distance[from].value())
        {
            continue;
        }
        if (limit && prefix >= *limit)
        {
            break;
        }
        settled[from] = true;
        for (const auto &[to, id, weight] : graph[from])
        {
            if (weight > kMax - prefix)
            {
                continue;
            }
            const auto candidate = prefix + weight;
            if (to == source)
            {
                if (directed || first[from] != id)
                {
                    consider(from, to, id, candidate);
                }
                continue;
            }
            // In an undirected graph, two different first edges identify disjoint tree paths from source.
            if (!directed && from != source && settled[to] && first[from] != first[to])
            {
                const auto suffix = distance[to].value();
                if (suffix <= kMax - candidate)
                {
                    consider(from, to, id, candidate + suffix);
                }
            }
            if (!distance[to] || candidate < distance[to].value())
            {
                distance[to] = candidate;
                parent[to] = from;
                incoming[to] = id;
                first[to] = from == source ? id : first[from];
                queue.emplace(candidate, to);
            }
        }
    }
    if (!best)
    {
        return std::nullopt;
    }

    WeightedCycle result{best->cost, {}};
    auto &vertices = result.cycle.vertices;
    auto &edges = result.cycle.edge_ids;
    for (int v = best->from; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    vertices.push_back(source);
    std::ranges::reverse(vertices);
    std::ranges::reverse(edges);
    edges.push_back(best->id);
    for (int v = best->to; v != source; v = parent[v])
    {
        vertices.push_back(v);
        edges.push_back(incoming[v]);
    }
    return result;
}
} // namespace detail

inline std::optional<WeightedCycle> shortest_weighted_cycle(
    int n, std::span<const WeightedCycleEdge> edges, bool directed = true,
    std::optional<int> through = std::nullopt)
{
    if (n < 0)
    {
        throw std::length_error("shortest_weighted_cycle: negative vertex count");
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (through && (*through < 0 || *through >= n))
    {
        throw std::out_of_range("shortest_weighted_cycle: invalid required vertex");
    }
#endif
    detail::WeightedCycleGraph graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [from, to, weight] = edges[id];
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("shortest_weighted_cycle: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id, weight});
        if (!directed)
        {
            graph[to].push_back({from, id, weight});
        }
    }

    std::optional<WeightedCycle> result;
    const int first = through.value_or(0);
    const int last = through ? *through + 1 : n;
    for (int v = first; v < last; ++v)
    {
        if (graph[v].empty())
        {
            continue;
        }
        const auto bound = result ? std::optional<std::uint64_t>(result->weight) : std::nullopt;
        if (auto better = detail::shortest_weighted_cycle_from(graph, v, edges.size(), directed, bound))
        {
            result = std::move(better);
            if (result->weight == 0)
            {
                break;
            }
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


#include <vector>

int main()
{
    int type{}, n{}, m{};
    verify_in >> type >> n >> m;
    std::vector<sirkit::WeightedCycleEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    const auto optimum = sirkit::shortest_weighted_cycle(n, edges, type == 1);
    if (optimum)
    {
        verify_out << optimum->weight << '\n';
    }
    else
    {
        verify_out << "-1\n";
    }
}
