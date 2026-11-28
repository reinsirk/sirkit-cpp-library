#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc359/tasks/abc359_g

// Library: Graph/Virtual_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Virtual_Tree.cpp
// 参考（Auxiliary Treeの性質）: https://atcoder.jp/contests/abc340/editorial/9249
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/virtual_tree.md
//
// API一覧
// VirtualTree(n, edges, root=0): 頂点[0,n)の無向木を前処理する。O(n)時間・領域。n=0も可。
// build(terminals): 指定した頂点とそれらのLCAだけの木を、親が先の順に返す。入力の順序・重複は自由。
// Node{vertex, parent, distance, terminal}: 元の頂点番号・返り値中の親番号・親への辺数距離・指定頂点か。
// 根ではparent=-1、distance=0。指定が空なら空。指定数kに対しO(k log(k+1)+k log(n+1))時間・O(k)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_VIRTUAL_TREE_DEFINED
#define SIRKIT_VIRTUAL_TREE_DEFINED

class VirtualTree
{
public:
    struct Node
    {
        int vertex;
        int parent;
        int distance;
        bool terminal;
    };

    VirtualTree(int n, std::span<const std::pair<int, int>> edges, int root = 0)
    {
        if (n < 0)
        {
            throw std::length_error("VirtualTree: negative vertex count");
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (n == 0)
        {
            if (root != 0 || !edges.empty())
            {
                throw std::invalid_argument("VirtualTree: invalid empty tree");
            }
        }
        else if (root < 0 || root >= n || edges.size() != static_cast<std::size_t>(n - 1))
        {
            throw std::invalid_argument("VirtualTree: invalid root or edge count");
        }
#endif
        if (n == 0)
        {
            return;
        }

        const auto size = static_cast<std::size_t>(n);
        std::vector<std::size_t> begin(size + 1);
        for (const auto &[u, v] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (u < 0 || u >= n || v < 0 || v >= n || u == v)
            {
                throw std::invalid_argument("VirtualTree: invalid edge");
            }
#endif
            ++begin[static_cast<std::size_t>(u) + 1];
            ++begin[static_cast<std::size_t>(v) + 1];
        }
        // edges.size() == n-1 なので2(n-1)は32ビットsize_tにも収まる。
        for (std::size_t i = 0; i < size; ++i)
        {
            begin[i + 1] += begin[i];
        }
        std::vector<int> adjacent(begin.back());
        auto next = begin;
        for (const auto &[u, v] : edges)
        {
            adjacent[next[u]++] = v;
            adjacent[next[v]++] = u;
        }

        parent_.resize(size);
        depth_.resize(size);
        order_.resize(size);
        subtree_size_.assign(size, 1);
        head_.resize(size);
#ifndef SKIP_RUNTIME_CHECKS
        std::fill(parent_.begin(), parent_.end(), -1);
#endif
        parent_[root] = root;
        std::vector<int> stack{root}, preorder;
        preorder.reserve(size);
        while (!stack.empty())
        {
            const int v = stack.back();
            stack.pop_back();
            order_[v] = static_cast<int>(preorder.size());
            preorder.push_back(v);
            for (std::size_t i = begin[v]; i != begin[static_cast<std::size_t>(v) + 1]; ++i)
            {
                const int to = adjacent[i];
                if (to == parent_[v])
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (parent_[to] != -1)
                {
                    throw std::invalid_argument("VirtualTree: cycle");
                }
#endif
                parent_[to] = v;
                depth_[to] = depth_[v] + 1;
                stack.push_back(to);
            }
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (preorder.size() != size)
        {
            throw std::invalid_argument("VirtualTree: disconnected tree");
        }
#endif

        std::vector<int> heavy_child(size, -1);
        for (std::size_t i = size; i > 1; --i)
        {
            const int v = preorder[i - 1], p = parent_[v];
            subtree_size_[p] += subtree_size_[v];
            if (heavy_child[p] == -1 || subtree_size_[heavy_child[p]] < subtree_size_[v])
            {
                heavy_child[p] = v;
            }
        }
        head_[root] = root;
        for (std::size_t i = 1; i < size; ++i)
        {
            const int v = preorder[i], p = parent_[v];
            head_[v] = v == heavy_child[p] ? head_[p] : v;
        }
    }

    [[nodiscard]] std::vector<Node> build(std::span<const int> terminals) const
    {
        if (terminals.empty())
        {
            return {};
        }
#ifndef SKIP_RUNTIME_CHECKS
        for (const int v : terminals)
        {
            if (v < 0 || static_cast<std::size_t>(v) >= order_.size())
            {
                throw std::out_of_range("VirtualTree: terminal vertex");
            }
        }
#endif
        std::vector<int> selected(terminals.begin(), terminals.end());
        const auto before = [this](int a, int b) { return order_[a] < order_[b]; };
        const auto sort_unique = [&before](std::vector<int> &vertices) {
            std::sort(vertices.begin(), vertices.end(), before);
            vertices.erase(std::unique(vertices.begin(), vertices.end()), vertices.end());
        };
        sort_unique(selected);
        std::vector<int> vertices = selected;
        for (std::size_t i = 1; i < selected.size(); ++i)
        {
            vertices.push_back(lca(selected[i - 1], selected[i]));
        }
        sort_unique(vertices);

        std::vector<Node> result;
        result.reserve(vertices.size());
        std::size_t terminal_index = 0;
        for (const int v : vertices)
        {
            // 追加したLCAは対応する指定頂点より前にある。最後まで比較先が残る。
            const bool terminal = selected[terminal_index] == v;
            terminal_index += terminal;
            if (result.empty())
            {
                result.push_back({v, -1, 0, terminal});
                continue;
            }
            // DFS順で直前の頂点から戻る。各頂点を親方向に通過するのは高々1回。
            int parent_index = static_cast<int>(result.size()) - 1;
            int p = result[parent_index].vertex;
            while (order_[v] >= order_[p] + subtree_size_[p])
            {
                parent_index = result[parent_index].parent;
                p = result[parent_index].vertex;
            }
            result.push_back({v, parent_index, depth_[v] - depth_[p], terminal});
        }
        return result;
    }

private:
    [[nodiscard]] int lca(int a, int b) const
    {
        while (head_[a] != head_[b])
        {
            if (depth_[head_[a]] < depth_[head_[b]])
            {
                std::swap(a, b);
            }
            a = parent_[head_[a]];
        }
        return depth_[a] < depth_[b] ? a : b;
    }

    std::vector<int> parent_, depth_, order_, subtree_size_, head_;
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


#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::VirtualTree tree(n, edges);
    std::vector<std::vector<int>> groups(n);
    for (int v = 0; v < n; ++v)
    {
        int color;
        verify_in >> color;
        groups[color - 1].push_back(v);
    }

    std::int64_t answer = 0;
    std::vector<int> counts;
    for (const auto &group : groups)
    {
        if (group.size() < 2)
        {
            continue;
        }
        const auto nodes = tree.build(group);
        const int total = static_cast<int>(group.size());
        counts.assign(nodes.size(), 0);
        for (std::size_t i = nodes.size() - 1; i > 0; --i)
        {
            const auto &[vertex, parent, distance, terminal] = nodes[i];
            counts[i] += terminal;
            answer += std::int64_t{distance} * counts[i] * (total - counts[i]);
            counts[parent] += counts[i];
        }
    }
    verify_out << answer << '\n';
}
