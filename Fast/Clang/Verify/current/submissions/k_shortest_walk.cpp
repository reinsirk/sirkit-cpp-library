#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/k_shortest_walk

// Library: Graph/K_Shortest_Walks.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/K_Shortest_Walks.cpp
// 参考（最短路木から外れる辺による列挙）: https://doi.org/10.1137/S0097539795290477
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/k_shortest_walks.md
//
// API一覧
// KShortestWalkEdge: 有向辺{from,to,weight}。頂点番号は0始まり。重みは非負。
// k_shortest_walk_lengths(n,source,target,edges,k): 長さの小さい順に最大k個を返す。
// 自己辺・多重辺・辺の再利用を許す。同じ重さでも辺列が異なるウォークは個別に数える。
// source==targetなら空ウォークも数える。存在数がk未満ならその分だけ返す。
// k番目までに符号付き64ビットの範囲を超える長さがあればoverflow_error。
// O(n+m log(m+1)+k log(k+1))時間・O(n+m log(m+1)+k)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <limits>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_K_SHORTEST_WALKS_DEFINED
#define SIRKIT_K_SHORTEST_WALKS_DEFINED

struct KShortestWalkEdge
{
    int from;
    int to;
    std::int64_t weight;
};

namespace detail
{
inline constexpr auto kWalkNoNode = std::numeric_limits<std::size_t>::max();
inline constexpr auto kWalkUnreachable = std::numeric_limits<std::uint64_t>::max();
inline constexpr auto kWalkOverflow = std::uint64_t{1} << 63;

inline std::uint64_t add_walk_lengths(std::uint64_t a, std::uint64_t b)
{
    return a <= kWalkOverflow - b ? a + b : kWalkOverflow;
}

using WalkDistanceQueue = std::priority_queue<
    std::pair<std::uint64_t, std::size_t>,
    std::vector<std::pair<std::uint64_t, std::size_t>>,
    std::greater<>>;

struct WalkShortestTree
{
    std::vector<std::uint64_t> distance;
    std::vector<std::size_t> next_edge;
    std::vector<int> settled;
};

inline WalkShortestTree shortest_tree_to_walk_target(
    int n, int target, std::span<const KShortestWalkEdge> edges)
{
    const auto count = static_cast<std::size_t>(n);
    std::vector<std::size_t> offsets(count + 1);
    for (const auto &edge : edges)
    {
        ++offsets[static_cast<std::size_t>(edge.to) + 1];
    }
    for (std::size_t i = 1; i < offsets.size(); ++i)
    {
        offsets[i] += offsets[i - 1];
    }

    std::vector<std::size_t> incoming(edges.size());
    {
        auto cursor = offsets;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            incoming[cursor[edges[i].to]++] = i;
        }
    }

    WalkShortestTree tree{
        std::vector<std::uint64_t>(count, kWalkUnreachable),
        std::vector<std::size_t>(count, kWalkNoNode),
        {},
    };
    WalkDistanceQueue queue;
    tree.distance[target] = 0;
    queue.emplace(0, target);
    while (!queue.empty())
    {
        const auto [distance, v] = queue.top();
        queue.pop();
        if (distance != tree.distance[v])
        {
            continue;
        }
        tree.settled.push_back(static_cast<int>(v));
        for (auto p = offsets[v]; p < offsets[v + 1]; ++p)
        {
            const auto index = incoming[p];
            const auto &edge = edges[index];
            const auto next = add_walk_lengths(distance, static_cast<std::uint64_t>(edge.weight));
            if (next < tree.distance[edge.from])
            {
                tree.distance[edge.from] = next;
                // 確定済み頂点への辺を選ぶので、零重みの閉路でも木は循環しない。
                tree.next_edge[edge.from] = index;
                queue.emplace(next, edge.from);
            }
        }
    }
    return tree;
}

class WalkSidetrackHeap
{
public:
    struct Node
    {
        std::size_t left = kWalkNoNode;
        std::size_t right = kWalkNoNode;
        std::uint64_t extra = 0;
        int to = 0;
        int rank = 1;
    };

    const Node &operator[](std::size_t index) const
    {
        return nodes_[index];
    }

    std::size_t insert(std::size_t root, std::uint64_t extra, int to)
    {
        nodes_.push_back(Node{.extra = extra, .to = to});
        return meld<false>(root, nodes_.size() - 1);
    }

    std::size_t inherit(std::size_t local, std::size_t ancestor)
    {
        return ancestor == kWalkNoNode ? local : meld<true>(local, ancestor);
    }

private:
    std::deque<Node> nodes_;

    int rank(std::size_t node) const
    {
        return node == kWalkNoNode ? 0 : nodes_[node].rank;
    }

    // bは空でない。コピーは祖先のヒープを受け継ぐときだけ行う。
    template <bool Persistent>
    std::size_t meld(std::size_t a, std::size_t b)
    {
        if (a == kWalkNoNode)
        {
            return b;
        }
        if (nodes_[b].extra < nodes_[a].extra)
        {
            std::swap(a, b);
        }
        // 既存の版にあるノードは書き換えず、結合に通った箇所だけ複製する。
        auto node = nodes_[a];
        node.right = meld<Persistent>(node.right, b);
        if (rank(node.left) < rank(node.right))
        {
            std::swap(node.left, node.right);
        }
        node.rank = rank(node.right) + 1;
        if constexpr (Persistent)
        {
            nodes_.push_back(node);
            return nodes_.size() - 1;
        }
        else
        {
            nodes_[a] = node;
            return a;
        }
    }
};

inline std::int64_t checked_walk_length(std::uint64_t length)
{
    if (length == kWalkOverflow)
    {
        throw std::overflow_error("k_shortest_walk_lengths: walk length does not fit int64_t");
    }
    return static_cast<std::int64_t>(length);
}
} // namespace detail

inline std::vector<std::int64_t> k_shortest_walk_lengths(
    int n, int source, int target, std::span<const KShortestWalkEdge> edges, std::size_t k)
{
    if (n <= 0)
    {
        throw std::invalid_argument("k_shortest_walk_lengths: nonpositive vertex count");
    }
    if (k == 0)
    {
        return {};
    }
    const auto tree = detail::shortest_tree_to_walk_target(n, target, edges);
    const auto shortest = tree.distance[source];
    if (shortest == detail::kWalkUnreachable)
    {
        return {};
    }
    std::vector<std::int64_t> result{detail::checked_walk_length(shortest)};
    if (k == 1)
    {
        return result;
    }

    constexpr auto kNone = detail::kWalkNoNode;
    detail::WalkSidetrackHeap heap;
    std::vector<std::size_t> alternatives(n, kNone);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &edge = edges[i];
        if (tree.distance[edge.to] == detail::kWalkUnreachable || tree.next_edge[edge.from] == i)
        {
            continue;
        }
        // 64ビットに収まらない長さは上限直後へまとめる。そこまで到達した場合だけ例外にする。
        const auto length = detail::add_walk_lengths(static_cast<std::uint64_t>(edge.weight), tree.distance[edge.to]);
        const auto extra = length - tree.distance[edge.from];
        alternatives[edge.from] = heap.insert(alternatives[edge.from], extra, edge.to);
    }
    for (int v : tree.settled)
    {
        const auto parent = tree.next_edge[v];
        if (parent != kNone)
        {
            alternatives[v] = heap.inherit(alternatives[v], alternatives[edges[parent].to]);
        }
    }

    detail::WalkDistanceQueue queue;
    if (const auto first = alternatives[source]; first != kNone)
    {
        queue.emplace(detail::add_walk_lengths(shortest, heap[first].extra), first);
    }
    while (!queue.empty())
    {
        const auto [length, selected] = queue.top();
        queue.pop();
        result.push_back(detail::checked_walk_length(length));
        if (result.size() == k)
        {
            break;
        }
        const auto &node = heap[selected];
        // 最後の木外辺を置き換えるか、着点から新たな木外辺を加える。
        for (const auto child : {node.left, node.right})
        {
            if (child != kNone)
            {
                queue.emplace(detail::add_walk_lengths(length, heap[child].extra - node.extra), child);
            }
        }
        if (const auto next = alternatives[node.to]; next != kNone)
        {
            queue.emplace(detail::add_walk_lengths(length, heap[next].extra), next);
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
    int n, m, source, target;
    std::size_t k = 0;
    verify_in >> n >> m >> source >> target >> k;
    std::vector<KShortestWalkEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
    }

    const auto lengths = k_shortest_walk_lengths(n, source, target, edges, k);
    for (const auto length : lengths)
    {
        verify_out << length << '\n';
    }
    for (auto i = lengths.size(); i < k; ++i)
    {
        verify_out << -1 << '\n';
    }
}
