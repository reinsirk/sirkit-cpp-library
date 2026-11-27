#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum

// Library: DataStructure/Euler_Tour_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Euler_Tour_Tree.cpp
// 参考（動的木のEuler Tour表現）: https://doi.org/10.1145/320211.320215
// 参考（splay木）: https://doi.org/10.1145/3828.3835
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/euler_tour_tree.md
//
// API一覧
// EulerTourTree<T>(n,initial={}): n個の孤立頂点を作る。初期値を省略するとすべて0。O(n+1)時間・領域。
// size()/component_count(): 頂点数/連結成分数。各O(1)。頂点は[0,n)、空の森も可。
// connected(u,v): 同じ成分か。link(u,v): 別成分なら辺を追加。cut(u,v): 辺があれば削除。
// link/cutは森の辺を変更したときtrue。自己辺や既に連結な頂点をlinkするとfalse。
// get(v)/add(v,delta): 点の値を読む/加算。component_sum(v)/component_add(v,delta): 成分の和/一括加算。
// subtree_sum(v,p)/subtree_add(v,p,delta): 辺(v,p)から見たv側の和/一括加算。辺がなければnullopt/false。
// 更新・照会は償却O(log(n+1))、再帰なし。照会も内部のsplayを行う。
// T{}は加法の単位元。可換な+=とT(頂点数)との乗算を用い、途中結果もTで表現できること。
// 操作中のT{}・T(頂点数)の構築、演算、代入は例外を送出しないこと。
// 3n−2をintで表せる範囲。コピーは独立でO(n+1)、移動・swapはO(1)。移動元は空の森。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EULER_TOUR_TREE_DEFINED
#define SIRKIT_EULER_TOUR_TREE_DEFINED

template <class T = std::int64_t>
class EulerTourTree
{
    struct Node
    {
        int parent = 0;
        int child[2]{};
        int count = 0;
        bool vertex = false;
        bool pending = false;
        T value{}, sum{}, lazy{};
    };

    using Edge = std::pair<int, int>;

    int vertices_ = 0;
    std::vector<Node> nodes_;
    std::map<Edge, int> darts_;
    std::vector<int> available_, ancestors_;
    std::size_t next_dart_ = 1;

    static std::size_t capacity(int n, std::size_t initial_size)
    {
        constexpr auto kMaxIndex = static_cast<std::size_t>(std::numeric_limits<int>::max());
        if (n < 0 || static_cast<std::size_t>(n) > (kMaxIndex + 2) / 3)
        {
            throw std::length_error("EulerTourTree: vertex count exceeds capacity");
        }
        if (initial_size != 0 && initial_size != static_cast<std::size_t>(n))
        {
            throw std::invalid_argument("EulerTourTree: incorrect number of initial values");
        }
        return n == 0 ? 1 : 3 * static_cast<std::size_t>(n) - 1;
    }

    void reserve_work()
    {
        available_.reserve(static_cast<std::size_t>(std::max(vertices_ - 1, 0)));
        ancestors_.reserve(nodes_.size());
    }

    void check_vertex([[maybe_unused]] int v) const
    {
    }

    void pull(int v)
    {
        auto &node = nodes_[v];
        const auto &left = nodes_[node.child[0]];
        const auto &right = nodes_[node.child[1]];
        node.count = left.count + right.count + static_cast<int>(node.vertex);
        node.sum = left.sum;
        node.sum += node.value;
        node.sum += right.sum;
    }

    void apply(int v, const T &delta)
    {
        auto &node = nodes_[v];
        if (node.count == 0)
        {
            return;
        }
        node.sum += delta * T(node.count);
        if (node.vertex)
        {
            node.value += delta;
        }
        node.lazy += delta;
        node.pending = true;
    }

    void push(int v)
    {
        auto &node = nodes_[v];
        if (node.pending)
        {
            apply(node.child[0], node.lazy);
            apply(node.child[1], node.lazy);
            node.lazy = T{};
            node.pending = false;
        }
    }

    void rotate(int v)
    {
        const int parent = nodes_[v].parent;
        const int grand = nodes_[parent].parent;
        const int side = nodes_[parent].child[1] == v;
        const int middle = nodes_[v].child[side ^ 1];
        if (grand != 0)
        {
            nodes_[grand].child[nodes_[grand].child[1] == parent] = v;
        }
        nodes_[v].parent = grand;
        nodes_[parent].child[side] = middle;
        if (middle != 0)
        {
            nodes_[middle].parent = parent;
        }
        nodes_[v].child[side ^ 1] = parent;
        nodes_[parent].parent = v;
        pull(parent);
        pull(v);
    }

    void splay(int v)
    {
        ancestors_.clear();
        for (int at = v; at != 0; at = nodes_[at].parent)
        {
            ancestors_.push_back(at);
        }
        for (auto it = ancestors_.rbegin(); it != ancestors_.rend(); ++it)
        {
            push(*it);
        }
        while (nodes_[v].parent != 0)
        {
            const int parent = nodes_[v].parent, grand = nodes_[parent].parent;
            if (grand != 0)
            {
                if ((nodes_[grand].child[0] == parent) == (nodes_[parent].child[0] == v))
                {
                    rotate(parent);
                }
                else
                {
                    rotate(v);
                }
            }
            rotate(v);
        }
    }

    int concatenate(int left, int right)
    {
        int last = left;
        while (nodes_[last].child[1] != 0)
        {
            last = nodes_[last].child[1];
        }
        splay(last);
        nodes_[last].child[1] = right;
        nodes_[right].parent = last;
        pull(last);
        return last;
    }

    int rotate_front(int v)
    {
        splay(v);
        const int left = nodes_[v].child[0];
        if (left == 0)
        {
            return v;
        }
        nodes_[left].parent = 0;
        nodes_[v].child[0] = 0;
        pull(v);
        return concatenate(v, left);
    }

    bool connected_nodes(int a, int b)
    {
        if (a == b)
        {
            return true;
        }
        splay(a);
        splay(b);
        return nodes_[a].parent != 0;
    }

    static Edge undirected(int u, int v)
    {
        return std::minmax(u, v);
    }

    int subtree_node(int v, int parent)
    {
        const auto it = darts_.find(undirected(v, parent));
        if (it == darts_.end())
        {
            return 0;
        }
        const int return_dart = it->second + static_cast<int>(v > parent);
        // 親から入る辺を先頭にして、逆向きの辺までに出会う頂点を集める。
        rotate_front(it->second + static_cast<int>(parent > v));
        splay(return_dart);
        return nodes_[return_dart].child[0];
    }

public:
    explicit EulerTourTree(int n, std::span<const T> initial = {})
        : vertices_(n), nodes_(capacity(n, initial.size())), next_dart_(static_cast<std::size_t>(n) + 1)
    {
        reserve_work();
        for (int v = 0; v < n; ++v)
        {
            auto &node = nodes_[v + 1];
            node.vertex = true;
            node.count = 1;
            if (!initial.empty())
            {
                node.value = node.sum = initial[v];
            }
        }
    }

    EulerTourTree(const EulerTourTree &other)
        : vertices_(other.vertices_), nodes_(other.nodes_), darts_(other.darts_),
          available_(other.available_), next_dart_(other.next_dart_)
    {
        reserve_work();
    }

    EulerTourTree(EulerTourTree &&other)
    {
        swap(other);
    }

    EulerTourTree &operator=(EulerTourTree other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(EulerTourTree &other) noexcept
    {
        std::swap(vertices_, other.vertices_);
        std::swap(next_dart_, other.next_dart_);
        nodes_.swap(other.nodes_);
        darts_.swap(other.darts_);
        available_.swap(other.available_);
        ancestors_.swap(other.ancestors_);
    }

    int size() const noexcept
    {
        return vertices_;
    }

    int component_count() const noexcept
    {
        return vertices_ - static_cast<int>(darts_.size());
    }

    bool connected(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        return connected_nodes(u + 1, v + 1);
    }

    bool link(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        if (connected_nodes(u + 1, v + 1))
        {
            return false;
        }
        const bool new_pair = available_.empty();
        const int base = new_pair ? static_cast<int>(next_dart_) : available_.back();
        darts_.emplace(undirected(u, v), base);
        if (new_pair)
        {
            next_dart_ += 2;
        }
        else
        {
            available_.pop_back();
        }

        const int outward = base + static_cast<int>(u > v);
        const int inward = base + static_cast<int>(v > u);
        const int first = rotate_front(u + 1);
        const int second = rotate_front(v + 1);
        concatenate(concatenate(first, outward), concatenate(second, inward));
        return true;
    }

    bool cut(int u, int v)
    {
        check_vertex(u);
        check_vertex(v);
        const auto it = darts_.find(undirected(u, v));
        if (it == darts_.end())
        {
            return false;
        }
        const int outgoing = it->second, incoming = outgoing + 1;
        rotate_front(outgoing);
        splay(incoming);
        const auto &between = nodes_[incoming].child;
        const int on_path = between[0], outside = between[1];
        nodes_[on_path].parent = 0;
        nodes_[outside].parent = 0;
        // 先頭の往路、各側の巡回、復路の形になっている。往路と復路だけを外す。
        splay(outgoing);
        nodes_[nodes_[outgoing].child[1]].parent = 0;
        nodes_[outgoing] = Node{};
        nodes_[incoming] = Node{};
        available_.push_back(outgoing);
        darts_.erase(it);
        return true;
    }

    T get(int v)
    {
        check_vertex(v);
        splay(v + 1);
        return nodes_[v + 1].value;
    }

    void add(int v, const T &delta)
    {
        check_vertex(v);
        splay(v + 1);
        nodes_[v + 1].value += delta;
        pull(v + 1);
    }

    T component_sum(int v)
    {
        check_vertex(v);
        splay(v + 1);
        return nodes_[v + 1].sum;
    }

    void component_add(int v, const T &delta)
    {
        check_vertex(v);
        splay(v + 1);
        apply(v + 1, delta);
    }

    std::optional<T> subtree_sum(int v, int parent)
    {
        check_vertex(v);
        check_vertex(parent);
        const int node = subtree_node(v, parent);
        if (node == 0)
        {
            return std::nullopt;
        }
        return nodes_[node].sum;
    }

    bool subtree_add(int v, int parent, const T &delta)
    {
        check_vertex(v);
        check_vertex(parent);
        const int node = subtree_node(v, parent);
        if (node == 0)
        {
            return false;
        }
        apply(node, delta);
        // 復路マーカーが根なので、部分木を変えた後はその集約も更新する。
        pull(nodes_[node].parent);
        return true;
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
#include <vector>

int main()
{
    int n, q;
    verify_in >> n >> q;
    std::vector<std::int64_t> initial(n);
    for (auto &value : initial)
    {
        verify_in >> value;
    }
    EulerTourTree<> forest(n, initial);
    for (int i = 1; i < n; ++i)
    {
        int u, v;
        verify_in >> u >> v;
        static_cast<void>(forest.link(u, v));
    }
    for (int i = 0; i < q; ++i)
    {
        int type;
        verify_in >> type;
        if (type == 0)
        {
            int u, v, w, x;
            verify_in >> u >> v >> w >> x;
            static_cast<void>(forest.cut(u, v));
            static_cast<void>(forest.link(w, x));
        }
        else if (type == 1)
        {
            int v;
            std::int64_t delta = 0;
            verify_in >> v >> delta;
            forest.add(v, delta);
        }
        else
        {
            int v, p;
            verify_in >> v >> p;
            verify_out << *forest.subtree_sum(v, p) << '\n';
        }
    }
}
