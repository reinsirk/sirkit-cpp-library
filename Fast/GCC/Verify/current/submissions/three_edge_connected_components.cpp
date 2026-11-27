#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/three_edge_connected_components

// Library: Graph/Three_Edge_Connected_Components.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Three_Edge_Connected_Components.cpp
// 参考（DFS木辺・後退辺と2辺カット）: https://doi.org/10.4230/LIPIcs.SEA.2021.20
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/three_edge_connected_components.md
//
// API一覧
// three_edge_connected_component_ids(n,edges): countと、頂点ごとの成分番号idsを返す。
// three_edge_connected_components(n,edges): 各成分の頂点列を返す。
// 元の無向グラフで任意の2辺を除いても互いに到達できる頂点をまとめる（除く辺は0～2本）。
// 頂点・成分番号は0始まり。多重辺を1本ずつ扱い、自己辺は無視する。空グラフの成分数は0。
// どちらも決定的なO((n+m) log(n+1))時間・O(n+m)領域。再帰を使わない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_THREE_EDGE_CONNECTED_COMPONENTS_DEFINED
#define SIRKIT_THREE_EDGE_CONNECTED_COMPONENTS_DEFINED

struct ThreeEdgeComponentIds
{
    int count;
    std::vector<int> ids;
};

namespace detail
{
struct ThreeEdgeDfs
{
    std::vector<int> parent;
    std::vector<int> depth;
    std::vector<int> order;
    std::vector<std::ptrdiff_t> crossing;
    std::vector<int> highest;
};

// 処理済みの木辺を飛び越え、最初の未割り当ての祖先を求める。
class ThreeEdgeAncestors
{
    std::vector<int> links_;
    std::vector<int> top_;

    int root(int v)
    {
        while (links_[v] >= 0)
        {
            const int p = links_[v];
            if (links_[p] >= 0)
            {
                links_[v] = links_[p];
            }
            v = links_[v];
        }
        return v;
    }

public:
    explicit ThreeEdgeAncestors(int n) : links_(n, -1), top_(n)
    {
        std::iota(top_.begin(), top_.end(), 0);
    }

    int first(int v)
    {
        return top_[root(v)];
    }

    void erase(int v, int above)
    {
        int from = root(v), to = root(above);
        const int top = top_[to];
        if (links_[from] > links_[to])
        {
            std::swap(from, to);
        }
        links_[from] += links_[to];
        links_[to] = from;
        top_[from] = top;
    }
};

inline std::vector<bool> three_edge_spanning_forest(
    int n, std::span<const std::pair<int, int>> edges, ThreeEdgeDfs &dfs)
{
    constexpr int kUnseen = -2;
    std::vector<bool> tree(edges.size());
    std::vector<std::vector<std::pair<int, std::size_t>>> graph(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[u, v] = edges[i];
        if (u != v)
        {
            graph[u].emplace_back(v, i);
            graph[v].emplace_back(u, i);
        }
    }
    std::vector<std::size_t> cursor(n);
    std::vector<int> path;
    path.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (dfs.parent[root] != kUnseen)
        {
            continue;
        }
        dfs.parent[root] = -1;
        dfs.order.push_back(root);
        path.push_back(root);
        while (!path.empty())
        {
            const int v = path.back();
            if (cursor[v] == graph[v].size())
            {
                path.pop_back();
                continue;
            }
            const auto &[to, edge_id] = graph[v][cursor[v]++];
            if (dfs.parent[to] == kUnseen)
            {
                dfs.parent[to] = v;
                dfs.depth[to] = dfs.depth[v] + 1;
                dfs.order.push_back(to);
                tree[edge_id] = true;
                path.push_back(to);
            }
        }
    }
    return tree;
}

inline ThreeEdgeDfs three_edge_dfs(int n, std::span<const std::pair<int, int>> edges)
{
    ThreeEdgeDfs dfs{
        std::vector<int>(n, -2), std::vector<int>(n), {}, std::vector<std::ptrdiff_t>(n), std::vector<int>(n, -1)};
    dfs.order.reserve(n);
    const auto tree = three_edge_spanning_forest(n, edges, dfs);

    std::vector<std::vector<int>> back_descendants(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        auto [upper, lower] = edges[i];
        if (tree[i] || upper == lower)
        {
            continue;
        }
        if (dfs.depth[upper] > dfs.depth[lower])
        {
            std::swap(upper, lower);
        }
        ++dfs.crossing[lower];
        --dfs.crossing[upper];
        back_descendants[upper].push_back(lower);
    }
    ThreeEdgeAncestors unassigned(n);
    for (auto it = dfs.order.rbegin(); it != dfs.order.rend(); ++it)
    {
        const int u = *it;
        if (dfs.parent[u] >= 0)
        {
            dfs.crossing[dfs.parent[u]] += dfs.crossing[u];
        }
        // 同じ木辺をまたぐ後退辺の上端は、逆DFS順で深い方から現れる。
        for (const int descendant : back_descendants[u])
        {
            for (int v = unassigned.first(descendant); dfs.depth[v] > dfs.depth[u]; v = unassigned.first(v))
            {
                dfs.highest[v] = dfs.depth[u];
                unassigned.erase(v, dfs.parent[v]);
            }
        }
    }
    return dfs;
}

struct ThreeEdgeCutGroups
{
    std::vector<int> coordinate;
    std::vector<int> period;
};

inline ThreeEdgeCutGroups three_edge_cut_groups(const ThreeEdgeDfs &dfs, std::size_t edge_count)
{
    const auto n = dfs.order.size();
    ThreeEdgeCutGroups result{std::vector<int>(n, -1), {}};
    std::vector<int> previous(n, -1), last(edge_count + 1, -1), path;
    path.reserve(n);
    for (const int v : dfs.order)
    {
        while (!path.empty() && path.back() != dfs.parent[v])
        {
            const int u = path.back();
            path.pop_back();
            if (dfs.parent[u] >= 0 && dfs.crossing[u] > 0)
            {
                last[dfs.crossing[u]] = previous[u];
            }
        }
        if (dfs.parent[v] >= 0)
        {
            const auto size = dfs.crossing[v];
            const int nearest = last[size];
            // 本数が等しく、深い側の後退辺がすべて浅い木辺もまたぐなら、後退辺の集合が一致する。
            if (nearest >= 0 && dfs.highest[v] < dfs.depth[nearest])
            {
                result.coordinate[v] = result.coordinate[nearest];
                ++result.period[result.coordinate[v]];
            }
            else
            {
                result.coordinate[v] = static_cast<int>(result.period.size());
                // 橋は2分割。後退辺1本の群も、木辺の本数より分割が1個多い。
                result.period.push_back(size <= 1 ? 2 : 1);
            }
            if (size > 0)
            {
                previous[v] = nearest;
                last[size] = v;
            }
        }
        path.push_back(v);
    }
    return result;
}

struct ThreeEdgeEvent
{
    std::size_t coordinate;
    std::size_t state;
};

inline void rank_three_edge_events(std::vector<ThreeEdgeEvent> &events, std::size_t coordinates)
{
    const auto size = events.size();
    std::vector<std::size_t> states(coordinates + 1), first(size), second(size), offsets(size + 1);
    std::vector<std::array<std::size_t, 2>> keys(size);
    std::iota(first.begin(), first.end(), std::size_t{0});
    const auto sort_by = [&](int column, std::span<const std::size_t> input, std::span<std::size_t> output)
    {
        std::fill(offsets.begin(), offsets.end(), 0);
        for (const auto i : input)
        {
            ++offsets[keys[i][column]];
        }
        std::size_t prefix = 0;
        for (auto &value : offsets)
        {
            const auto count = value;
            value = prefix;
            prefix += count;
        }
        for (const auto i : input)
        {
            output[offsets[keys[i][column]]++] = i;
        }
    };
    for (; coordinates > 1; coordinates = (coordinates + 1) / 2)
    {
        std::fill_n(states.begin(), coordinates + 1, 0);
        for (std::size_t i = 0; i < size; ++i)
        {
            auto &[position, state] = events[i];
            states[position] = state;
            const auto left = position & ~std::size_t{1};
            keys[i] = {states[left], states[left + 1]};
            position /= 2;
        }
        // 片方ずつ計数ソートして同じ組に同じ番号を割り当てる。全ゼロの状態は0のまま。
        sort_by(1, first, second);
        sort_by(0, second, first);
        std::array<std::size_t, 2> previous{};
        std::size_t rank = 0;
        for (const auto i : first)
        {
            if (keys[i] != previous)
            {
                previous = keys[i];
                ++rank;
            }
            events[i].state = rank;
        }
    }
}

inline ThreeEdgeComponentIds three_edge_profiles(const ThreeEdgeDfs &dfs, const ThreeEdgeCutGroups &groups)
{
    const auto n = dfs.order.size();
    std::vector<ThreeEdgeEvent> events;
    events.reserve(2 * n);
    std::vector<int> value(groups.period.size()), path;
    path.reserve(n);
    std::vector<std::size_t> active_event(n);
    const auto change = [&](int v, bool enter)
    {
        const int coordinate = groups.coordinate[v];
        if (coordinate < 0 || groups.period[coordinate] == 1)
        {
            return;
        }
        const int period = groups.period[coordinate];
        auto &state = value[coordinate];
        if (enter)
        {
            ++state;
            if (state == period)
            {
                state = 0;
            }
        }
        else
        {
            if (state == 0)
            {
                state = period;
            }
            --state;
        }
        events.push_back({static_cast<std::size_t>(coordinate), static_cast<std::size_t>(state)});
    };
    for (const int v : dfs.order)
    {
        while (!path.empty() && path.back() != dfs.parent[v])
        {
            change(path.back(), false);
            path.pop_back();
        }
        change(v, true);
        path.push_back(v);
        active_event[v] = events.size();
    }
    // 最後に残った祖先列は、この後に照会する頂点がないためイベントを戻す必要がない。
    rank_three_edge_events(events, groups.period.size());

    std::vector<int> component(events.size() + 1, -1), id(events.size() + 1);
    ThreeEdgeComponentIds result{0, std::vector<int>(n)};
    int root = -1;
    for (const int v : dfs.order)
    {
        if (dfs.parent[v] < 0)
        {
            root = v;
        }
        const auto time = active_event[v];
        const auto state = time == 0 ? 0 : events[time - 1].state;
        if (component[state] != root)
        {
            component[state] = root;
            id[state] = result.count++;
        }
        result.ids[v] = id[state];
    }
    return result;
}
} // namespace detail

inline ThreeEdgeComponentIds three_edge_connected_component_ids(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("three_edge_connected_components: negative vertex count");
    }
    const auto dfs = detail::three_edge_dfs(n, edges);
    const auto groups = detail::three_edge_cut_groups(dfs, edges.size());
    return detail::three_edge_profiles(dfs, groups);
}

inline std::vector<std::vector<int>> three_edge_connected_components(
    int n, std::span<const std::pair<int, int>> edges)
{
    const auto [count, ids] = three_edge_connected_component_ids(n, edges);
    std::vector<std::vector<int>> result(count);
    for (int v = 0; v < n; ++v)
    {
        result[ids[v]].push_back(v);
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
    const auto groups = three_edge_connected_components(n, edges);
    verify_out << groups.size() << '\n';
    for (const auto &group : groups)
    {
        verify_out << group.size();
        for (int v : group)
        {
            verify_out << ' ' << v;
        }
        verify_out << '\n';
    }
}
