#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://yukicoder.me/problems/no/1069

// Library: Graph/K_Shortest_Simple_Paths.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/K_Shortest_Simple_Paths.cpp
// 参考（Yen 法）: https://doi.org/10.1287/mnsc.17.11.712
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/k_shortest_simple_paths.md
//
// API一覧
// k_shortest_simple_paths(n,edges,source,target,k,undirected=false): 単純路を長さの小さい順に最大 k 本返す。
//   ShortestPathEdge{from,to,weight} は0始まりの端点と非負の uint64_t 重み。自己辺・多重辺可。
//   SimpleWeightedPath{distance,vertices,edges} は距離、頂点列、入力順の辺 ID 列。
//   同じ長さでも辺 ID 列の異なる単純路は別に数える。同距離での順は未規定。
//   始点と終点が同じなら長さ0の路だけ。距離が uint64_t を超える路は除外する。
//   最悪 O(kn(n+m log(m+1)+n log(k+1)+k)) 時間、O(n+m+kn) 領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_PATH_DISTANCE_DEFINED
#define SIRKIT_SHORTEST_PATH_DISTANCE_DEFINED

struct ShortestPathEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

namespace detail
{
using ShortestPathAdjacency = std::vector<std::vector<std::pair<std::uint64_t, int>>>;

inline std::vector<std::optional<std::uint64_t>> shortest_path_edge_distances(const ShortestPathAdjacency &graph, int source)
{
    using State = std::pair<std::uint64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pending;
    std::vector<std::optional<std::uint64_t>> distance(graph.size());
    distance[source] = 0;
    pending.emplace(0, source);
    while (!pending.empty())
    {
        const auto [prefix, vertex] = pending.top();
        pending.pop();
        if (prefix != distance[vertex].value())
        {
            continue;
        }
        for (const auto &[weight, next] : graph[vertex])
        {
            if (weight > std::numeric_limits<std::uint64_t>::max() - prefix)
            {
                continue;
            }
            const std::uint64_t candidate = prefix + weight;
            if (!distance[next] || candidate < distance[next].value())
            {
                distance[next] = candidate;
                pending.emplace(candidate, next);
            }
        }
    }
    return distance;
}
} // namespace detail

#endif

#ifndef SIRKIT_K_SHORTEST_SIMPLE_PATHS_DEFINED
#define SIRKIT_K_SHORTEST_SIMPLE_PATHS_DEFINED

struct SimpleWeightedPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<std::size_t> edges;
};

inline std::vector<SimpleWeightedPath> k_shortest_simple_paths(
    int n, std::span<const ShortestPathEdge> edges, int source, int target,
    std::size_t k, bool undirected = false)
{
    if (k == 0)
    {
        return {};
    }
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const auto [from, to, weight] = edges[id];
        graph[from].push_back({to, id});
        if (undirected && from != to)
        {
            graph[to].push_back({from, id});
        }
    }

    std::vector<unsigned char> blocked_vertices(n), blocked_edges(edges.size());
    std::vector<std::optional<std::uint64_t>> distances(n);
    std::vector<int> previous_vertex(n);
    std::vector<std::size_t> previous_edge(n);
    using Entry = std::pair<std::uint64_t, int>;
    std::vector<Entry> pending;
    const std::greater<Entry> farther;
    auto shortest = [&](int start, std::uint64_t prefix) -> std::optional<SimpleWeightedPath>
    {
        std::ranges::fill(distances, std::nullopt);
        pending.clear();
        distances[start] = prefix;
        pending.emplace_back(prefix, start);
        while (!pending.empty())
        {
            std::pop_heap(pending.begin(), pending.end(), farther);
            const auto [distance, u] = pending.back();
            pending.pop_back();
            if (distance != *distances[u])
            {
                continue;
            }
            if (u == target)
            {
                SimpleWeightedPath result{distance, {target}, {}};
                for (int v = target; v != start;)
                {
                    result.edges.push_back(previous_edge[v]);
                    v = previous_vertex[v];
                    result.vertices.push_back(v);
                }
                std::ranges::reverse(result.vertices);
                std::ranges::reverse(result.edges);
                return result;
            }
            for (const auto &[v, id] : graph[u])
            {
                const auto weight = edges[id].weight;
                if (blocked_vertices[v] || blocked_edges[id] || weight > std::numeric_limits<std::uint64_t>::max() - distance)
                {
                    continue;
                }
                const auto next = distance + weight;
                if (!distances[v] || next < *distances[v])
                {
                    distances[v] = next;
                    previous_vertex[v] = u;
                    previous_edge[v] = id;
                    pending.emplace_back(next, v);
                    std::push_heap(pending.begin(), pending.end(), farther);
                }
            }
        }
        return std::nullopt;
    };

    struct Shorter
    {
        bool operator()(const SimpleWeightedPath &a, const SimpleWeightedPath &b) const
        {
            return std::tie(a.distance, a.edges) < std::tie(b.distance, b.edges);
        }
    };
    std::vector<SimpleWeightedPath> result;
    std::set<SimpleWeightedPath, Shorter> candidates;
    auto first = shortest(source, 0);
    if (!first)
    {
        return result;
    }
    result.push_back(std::move(*first));
    std::vector<std::size_t> common;
    while (result.size() < k)
    {
        const auto &base = result.back();
        common.clear();
        for (const auto &path : result)
        {
            const auto prefix = std::mismatch(base.edges.begin(), base.edges.end(), path.edges.begin(), path.edges.end());
            common.push_back(static_cast<std::size_t>(prefix.first - base.edges.begin()));
        }

        std::ranges::fill(blocked_vertices, 0);
        std::uint64_t prefix = 0;
        for (std::size_t i = 0; i < base.edges.size(); ++i)
        {
            // 同じ接頭辞から、採用済みのどの路とも異なる辺に進む。
            for (std::size_t j = 0; j < result.size(); ++j)
            {
                if (i <= common[j])
                {
                    blocked_edges[result[j].edges[i]] = 1;
                }
            }
            if (auto suffix = shortest(base.vertices[i], prefix))
            {
                suffix->vertices.insert(suffix->vertices.begin(), base.vertices.begin(), base.vertices.begin() + i);
                suffix->edges.insert(suffix->edges.begin(), base.edges.begin(), base.edges.begin() + i);
                candidates.insert(std::move(*suffix));
                if (candidates.size() > k - result.size())
                {
                    // 残り本数より下位の候補は最終結果に必要ない。
                    candidates.erase(std::prev(candidates.end()));
                }
            }
            for (std::size_t j = 0; j < result.size(); ++j)
            {
                if (i <= common[j])
                {
                    blocked_edges[result[j].edges[i]] = 0;
                }
            }
            blocked_vertices[base.vertices[i]] = 1;
            prefix += edges[base.edges[i]].weight;
        }
        if (candidates.empty())
        {
            break;
        }
        result.push_back(std::move(candidates.extract(candidates.begin()).value()));
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


#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    std::size_t k{};
    verify_in >> n >> m >> k >> source >> target;
    --source;
    --target;
    std::vector<std::pair<int, int>> points(n);
    for (auto &[x, y] : points)
    {
        verify_in >> x >> y;
    }
    constexpr double kScale = 1e9;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to;
        --from;
        --to;
        const auto [x, y] = points[from];
        const auto [u, v] = points[to];
        weight = static_cast<std::uint64_t>(std::llround(std::hypot(x - u, y - v) * kScale));
    }
    const auto paths = sirkit::k_shortest_simple_paths(n, edges, source, target, k, true);
    verify_out.fixed(9);
    for (std::size_t i = 0; i < k; ++i)
    {
        if (i < paths.size())
        {
            verify_out << static_cast<double>(paths[i].distance) / kScale << '\n';
        }
        else
        {
            verify_out << -1 << '\n';
        }
    }
}
