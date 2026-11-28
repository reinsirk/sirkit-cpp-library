#define SKIP_RUNTIME_CHECKS
// Verify-SHA256: c03ad5a8913e7907321ab200daf15c341121a9462211fc5e88b388f3b1fb9b9c
// Verify: https://atcoder.jp/contests/nikkei2019-2-final/tasks/nikkei2019_2_final_d

// Library: Graph/Unrooted_Tree_Isomorphism.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/unrooted_tree_isomorphism.md
//
// API一覧
// unrooted_trees_equal(n,first,second): 2つのn頂点の無向木の同型判定。n=0の両空木は同型。
// tree_leaf_deletion_matches(n,tree,smaller): n頂点の木から葉1つを消して、n-1頂点の木と同型にできる葉を昇順に返す。n>=2。
//   頂点は0始まり。無向辺をpair<int,int>で1回渡す。両関数とも n<=INT_MAX/4。
//   根の候補を重心に絞り、部分木の同型分類を共有する。衝突のない決定的な判定。O(n log(n+1))時間、O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_ROOTED_TREE_ISOMORPHISM_DEFINED
#define SIRKIT_ROOTED_TREE_ISOMORPHISM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline int rooted_tree_vertex_count(std::size_t size)
{
    if (size > std::numeric_limits<int>::max())
    {
        throw std::length_error("rooted_subtree_classes: too many vertices");
    }
    return static_cast<int>(size);
}
} // namespace detail

struct RootedSubtreeClasses
{
    int count;
    std::vector<int> class_id;
};

inline RootedSubtreeClasses rooted_subtree_classes(std::span<const int> parent)
{
    const auto n = detail::rooted_tree_vertex_count(parent.size());
    std::vector<int> remaining(n);
    for (int p : parent)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (p < -1 || p >= n)
        {
            throw std::out_of_range("rooted_subtree_classes: invalid parent");
        }
#endif
        if (p != -1)
        {
            ++remaining[p];
        }
    }
    std::vector<int> order;
    order.reserve(n);
    for (int v = 0; v < n; ++v)
    {
        if (remaining[v] == 0)
        {
            order.push_back(v);
        }
    }

    std::vector<std::vector<int>> signature(n);
    std::map<std::vector<int>, int> classes;
    RootedSubtreeClasses result{0, std::vector<int>(n)};
    for (std::size_t cursor = 0; cursor < order.size(); ++cursor)
    {
        const int v = order[cursor];
        auto &key = signature[v];
        std::ranges::sort(key);
        const auto [pos, inserted] = classes.try_emplace(std::move(key), result.count);
        result.count += inserted;
        result.class_id[v] = pos->second;
        const int p = parent[v];
        if (p != -1)
        {
            signature[p].push_back(pos->second);
            if (--remaining[p] == 0)
            {
                order.push_back(p);
            }
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (order.size() != parent.size())
    {
        throw std::invalid_argument("rooted_subtree_classes: parent cycle");
    }
#endif
    return result;
}

#endif

#ifndef SIRKIT_UNROOTED_TREE_ISOMORPHISM_DEFINED
#define SIRKIT_UNROOTED_TREE_ISOMORPHISM_DEFINED

namespace detail
{
using TreeShapeEdge = std::pair<int, int>;

inline void check_tree_shape_size(int n)
{
    if (n < 0 || n > std::numeric_limits<int>::max() / 4)
    {
        throw std::length_error("unrooted tree isomorphism: vertex count");
    }
}

class TreeShapeRoots
{
public:
    TreeShapeRoots(int n, std::span<const TreeShapeEdge> edges)
        : graph_(n)
    {
#ifndef SKIP_RUNTIME_CHECKS
        const int count = std::max(0, n - 1);
        if (edges.size() != static_cast<std::size_t>(count))
        {
            throw std::invalid_argument("unrooted tree isomorphism: number of edges");
        }
#endif
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (static_cast<unsigned>(u) >= static_cast<unsigned>(n) || static_cast<unsigned>(v) >= static_cast<unsigned>(n))
            {
                throw std::out_of_range("unrooted tree isomorphism: edge endpoint");
            }
#endif
            graph_[u].push_back(v);
            graph_[v].push_back(u);
        }
        if (n == 0)
        {
            return;
        }

        std::vector<int> parent(n, -2);
        const auto order = orient(0, 0, parent);
        std::vector<int> subtree(n, 1), largest_child(n);
        for (auto i = order.rbegin(); i != order.rend(); ++i)
        {
            const int v = *i;
            const int p = parent[v];
            if (p != -1)
            {
                subtree[p] += subtree[v];
                largest_child[p] = std::max(largest_child[p], subtree[v]);
            }
        }
        for (int v = 0; v < n; ++v)
        {
            if (std::max(largest_child[v], n - subtree[v]) <= n / 2)
            {
                centers_.push_back(v);
            }
        }
    }

    [[nodiscard]] std::vector<int> append_parents(std::vector<int> &combined) const
    {
        std::vector<int> roots;
        for (int center : centers_)
        {
            const int base = static_cast<int>(combined.size());
            combined.resize(combined.size() + graph_.size(), -2);
            (void)orient(center, base, std::span<int>(combined).subspan(base));
            roots.push_back(base + center);
        }
        return roots;
    }

private:
    [[nodiscard]] std::vector<int> orient(int root, int offset, std::span<int> parent) const
    {
        std::vector<int> order;
        order.reserve(graph_.size());
        order.push_back(root);
        parent[root] = -1;
        for (std::size_t i = 0; i < order.size(); ++i)
        {
            const int v = order[i];
            for (int w : graph_[v])
            {
                if (w + offset == parent[v])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent[w] != -2)
                {
                    throw std::invalid_argument("unrooted tree isomorphism: cycle");
                }
#endif
                parent[w] = v + offset;
                order.push_back(w);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (order.size() != graph_.size())
        {
            throw std::invalid_argument("unrooted tree isomorphism: disconnected");
        }
#endif
        return order;
    }

    std::vector<std::vector<int>> graph_;
    std::vector<int> centers_;
};

struct TreeChildDifference
{
    int extra_class = -1;
    int missing = -1;
};

inline std::optional<TreeChildDifference> tree_child_difference(
    std::span<const int> left, std::span<const int> right, std::span<const int> classes)
{
    if (left.size() < right.size() || left.size() - right.size() > 1)
    {
        return std::nullopt;
    }
    TreeChildDifference difference;
    std::size_t i = 0, j = 0;
    while (i < left.size() || j < right.size())
    {
        if (i < left.size() && j < right.size() && classes[left[i]] == classes[right[j]])
        {
            ++i;
            ++j;
        }
        else if (j == right.size() || (i < left.size() && classes[left[i]] < classes[right[j]]))
        {
            if (difference.extra_class != -1)
            {
                return std::nullopt;
            }
            difference.extra_class = classes[left[i++]];
        }
        else
        {
            if (difference.missing != -1)
            {
                return std::nullopt;
            }
            difference.missing = right[j++];
        }
    }
    return difference;
}
}

inline bool unrooted_trees_equal(int n, std::span<const std::pair<int, int>> first, std::span<const std::pair<int, int>> second)
{
    detail::check_tree_shape_size(n);
    std::vector<int> parents;
    const auto a = detail::TreeShapeRoots(n, first).append_parents(parents);
    const auto b = detail::TreeShapeRoots(n, second).append_parents(parents);
    if (n == 0)
    {
        return true;
    }
    const auto classes = rooted_subtree_classes(parents).class_id;
    for (int x : a)
    {
        for (int y : b)
        {
            if (classes[x] == classes[y])
            {
                return true;
            }
        }
    }
    return false;
}

inline std::vector<int> tree_leaf_deletion_matches(
    int n, std::span<const std::pair<int, int>> tree, std::span<const std::pair<int, int>> smaller)
{
    detail::check_tree_shape_size(n);
    if (n < 2)
    {
        throw std::invalid_argument("tree_leaf_deletion_matches: at least two vertices required");
    }
    std::vector<int> parents;
    const auto a = detail::TreeShapeRoots(n, tree).append_parents(parents);
    const auto b = detail::TreeShapeRoots(n - 1, smaller).append_parents(parents);
    const auto classes = rooted_subtree_classes(parents).class_id;
    std::vector<std::vector<int>> children(parents.size());
    for (int v = 0; v < static_cast<int>(parents.size()); ++v)
    {
        if (parents[v] != -1)
        {
            children[parents[v]].push_back(v);
        }
    }
    for (auto &group : children)
    {
        std::ranges::sort(group, {}, [&](int v)
                          {
                              return classes[v];
                          });
    }

    std::vector<bool> matching(n);
    std::vector<std::pair<int, int>> pending;
    for (int x : a)
    {
        for (int y : b)
        {
            pending.emplace_back(x, y);
            while (!pending.empty())
            {
                const auto [u, v] = pending.back();
                pending.pop_back();
                const auto difference = detail::tree_child_difference(children[u], children[v], classes);
                if (!difference)
                {
                    continue;
                }
                for (int child : children[u])
                {
                    if (classes[child] == difference->extra_class)
                    {
                        if (difference->missing == -1)
                        {
                            matching[child % n] = true;
                        }
                        else
                        {
                            pending.emplace_back(child, difference->missing);
                        }
                    }
                }
            }
        }
    }
    std::vector<int> result;
    for (int v = 0; v < n; ++v)
    {
        if (matching[v])
        {
            result.push_back(v);
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


#include <utility>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    auto read = [](int edges)
    {
        std::vector<std::pair<int, int>> tree(edges);
        for (int i = 0; i < edges; ++i)
        {
            int parent{};
            verify_in >> parent;
            tree[i] = {i, parent - 1};
        }
        return tree;
    };
    const auto full = read(n);
    const auto smaller = read(n - 1);
    const auto answers = sirkit::tree_leaf_deletion_matches(n + 1, full, smaller);
    for (std::size_t i = 0; i < answers.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << answers[i] + 1;
    }
    verify_out << '\n';
}
