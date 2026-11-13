#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_affine_range_sum

// Library: DataStructure/Link_Cut_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Link_Cut_Tree.cpp
// 参考（数学的構成・機能）: https://nyaannyaan.github.io/library/lct/link-cut-base.hpp
// 参考（数学的構成・機能）: https://ei1333.github.io/library/structure/dynamic-tree/lazy-link-cut-tree.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/link_cut_tree.md
//
// API一覧
// [LinkCutTree<T=int64_t, Op=plus<T>, Action=内部の作用なし型, Mapping=内部既定型, Composition=内部既定型>]
// LinkCutTree(values={}, identity=T{}, op=Op{}, mapping=Mapping{}, composition=Composition{}): vector/spanの各値を持つ孤立頂点を構築。O(n+1)時間・領域。
// Tはcopyable、Opはconst呼出可能な結合演算、identityは左右の単位元。非可換でも可。演算・コピー等をO(1)とする。
// 作用を使う場合はAction/Mapping/Compositionを指定。mapping(f,積,頂点数)、composition(new,old)はnewを後に作用。
// 作用は全頂点へ同じように適用でき、積に分配し、反転と可換であること。値・演算の数値範囲も利用者の前提。
// 値操作・演算・作用等から同じ木へ再入しない。
// 頂点は0..size()-1。n<=INT_MAX。以下の森操作は償却O(log(n+1))、1回は最悪O(n+1)。再帰しない。
// link(u,v): 別成分なら辺を加えてtrue、閉路/自己辺ならfalse。cut(u,v): その辺があれば除いてtrue、なければfalse。
// connected(u,v): 同じ成分か。make_root(v): vを成分の根にする。find_root(v): 現在の根の頂点番号。
// get(v)/set(v,x)/transform(v,fn): 値をコピー/代入/fn(旧値)で更新。fnから同じ木へ再入しない。
// path_prod(u,v): uからvへ端点を含め順に合成したoptional<T>。未接続ならnullopt。
// path_size(u,v): パスの頂点数、未接続なら0。path_length(u,v): 辺数のoptional、未接続ならnullopt。
// kth_on_path(u,v,k): u側から0始まりk番目の頂点。未接続/順位範囲外ならnullopt。
// apply(v,f)/apply(u,v,f): 点/パス作用。作用あり型だけ。パス版は未接続なら変更せずfalse、成功true。
// lca(root,u,v): rootを根にしたLCA。3頂点が同じ成分でなければnullopt。
// パス操作・link/cutは根の向きを変える場合がある。根が重要ならmake_root/lcaで明示する。
// size()/empty()/usable(): 頂点数/空か/例外後も森操作できるか。O(1)。reset(values): 同じ演算で孤立頂点へ再構築、O(旧n+n+1)。
// clear(): 空へ戻して頂点領域を解放、O(n+1)。swap(other): O(1)。コピーは頂点/辺/遅延の領域が独立、演算設定はconst共有。
// コピー/再構築の確保・コピー失敗時は元を保つ。moveはO(1)・noexcept、移動元は同じ設定の空の森として再利用できる。
// 森操作中に値操作/演算/作用が例外を送出したらusable()==false。以後はreset/clear/代入等で復旧するまで森操作不可。
// 値/Actionの失敗後も安全に破棄できること。外部状態は巻き戻さない。値/Actionはcopyable、設定はmove_constructibleで破棄はnoexcept。
// 添字検査だけをSKIP_RUNTIME_CHECKSで除く。容量・添字表現の上限、接続/辺/順位の戻り値、例外後の無効化は常に維持。

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_LINK_CUT_TREE_SUPPORT_DEFINED
#define SIRKIT_LINK_CUT_TREE_SUPPORT_DEFINED
namespace reinsirk::detail {
struct NoLinkCutAction {};
template<class T> struct NoLinkCutMapping {
    T operator()(NoLinkCutAction, const T& value, std::size_t) const { return value; }
};
struct NoLinkCutComposition {
    NoLinkCutAction operator()(NoLinkCutAction, NoLinkCutAction) const noexcept { return {}; }
};
template<class Action, bool Enabled> struct LinkCutLazyState { std::optional<Action> pending; };
template<class Action> struct LinkCutLazyState<Action, false> {};
}
#endif

#ifndef SIRKIT_LINK_CUT_TREE_DEFINED
#define SIRKIT_LINK_CUT_TREE_DEFINED
template<class T = std::int64_t, class Op = std::plus<T>,
         class Action = reinsirk::detail::NoLinkCutAction,
         class Mapping = reinsirk::detail::NoLinkCutMapping<T>,
         class Composition = reinsirk::detail::NoLinkCutComposition>
class LinkCutTree {
    static constexpr bool has_action = !std::is_same_v<Action, reinsirk::detail::NoLinkCutAction>;
    static_assert(std::copyable<T>);
    static_assert(has_action || (std::is_same_v<Mapping, reinsirk::detail::NoLinkCutMapping<T>> && std::is_same_v<Composition, reinsirk::detail::NoLinkCutComposition>), "LinkCutTree: specify Action with Mapping/Composition");
    static_assert(std::move_constructible<Op> && std::move_constructible<Mapping> && std::move_constructible<Composition>);
    static_assert(std::is_invocable_r_v<T, const Op&, const T&, const T&>);
    static_assert(!has_action || std::copyable<Action>);
    static_assert(!has_action || std::is_invocable_r_v<T, const Mapping&, const Action&, const T&, std::size_t>);
    static_assert(!has_action || std::is_invocable_r_v<Action, const Composition&, const Action&, const Action&>);
    struct Config {
        T identity;
        [[no_unique_address]] Op op;
        [[no_unique_address]] Mapping mapping;
        [[no_unique_address]] Composition composition;
        Config(const T& id, Op operation, Mapping map, Composition compose)
            : identity(id), op(std::move(operation)), mapping(std::move(map)), composition(std::move(compose)) {}
    };
    struct Node {
        int parent = 0, child[2]{0, 0};
        bool reversed = false;
        std::size_t count = 1;
        T value, forward, backward;
        [[no_unique_address]] reinsirk::detail::LinkCutLazyState<Action, has_action> lazy;
        explicit Node(const T& input, std::size_t length = 1)
            : count(length), value(input), forward(input), backward(input) {}
    };
    // 親子は非所有の配列添字。設定と頂点配列が所有する。
    std::shared_ptr<const Config> config_;
    std::vector<Node> nodes_;
    std::vector<int> ancestors_;
    std::size_t size_ = 0;
    bool failed_ = false;

    template<class Values> void initialize(const Values& values) {
        const auto n = values.size();
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()) || n >= nodes_.max_size() || n > ancestors_.max_size())
            throw std::length_error("LinkCutTree: capacity");
        nodes_.reserve(n + 1); ancestors_.reserve(n);
        nodes_.emplace_back(config_->identity, 0);
        for (std::size_t i = 0; i < n; ++i) nodes_.emplace_back(values[i]);
        size_ = n;
    }
    template<class Values> LinkCutTree(const Values& values, std::shared_ptr<const Config> config)
        : config_(std::move(config)) { initialize(values); }
    void check_vertex(int vertex) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size_) throw std::out_of_range("LinkCutTree: vertex");
#else
        (void)vertex;
#endif
    }
    template<class Fn> decltype(auto) operate(Fn&& operation) {
        if (failed_) throw std::logic_error("LinkCutTree: reset required after exception");
        try { return std::forward<Fn>(operation)(); }
        catch (...) { failed_ = true; throw; }
    }
    bool auxiliary_root(int vertex) const noexcept {
        const int parent = nodes_[vertex].parent;
        return parent == 0 || (nodes_[parent].child[0] != vertex && nodes_[parent].child[1] != vertex);
    }
    void pull(int vertex) {
        auto& node = nodes_[vertex];
        const int left = node.child[0], right = node.child[1];
        node.count = nodes_[left].count + 1 + nodes_[right].count;
        T forward = left == 0 ? node.value : std::invoke(config_->op, nodes_[left].forward, node.value);
        if (right != 0) forward = std::invoke(config_->op, forward, nodes_[right].forward);
        T backward = right == 0 ? node.value : std::invoke(config_->op, nodes_[right].backward, node.value);
        if (left != 0) backward = std::invoke(config_->op, backward, nodes_[left].backward);
        node.forward = std::move(forward); node.backward = std::move(backward);
    }
    void reverse(int vertex) {
        if (vertex == 0) return;
        auto& node = nodes_[vertex];
        using std::swap;
        swap(node.child[0], node.child[1]); swap(node.forward, node.backward);
        node.reversed = !node.reversed;
    }
    void apply_node(int vertex, const Action& action) requires(has_action) {
        if (vertex == 0) return;
        auto& node = nodes_[vertex];
        T value = std::invoke(config_->mapping, action, node.value, std::size_t{1});
        T forward = std::invoke(config_->mapping, action, node.forward, node.count);
        T backward = std::invoke(config_->mapping, action, node.backward, node.count);
        std::optional<Action> pending;
        if (node.lazy.pending) pending.emplace(std::invoke(config_->composition, action, *node.lazy.pending));
        else pending.emplace(action);
        node.value = std::move(value); node.forward = std::move(forward); node.backward = std::move(backward);
        node.lazy.pending = std::move(pending);
    }
    void push(int vertex) {
        auto& node = nodes_[vertex];
        if (node.reversed) {
            reverse(node.child[0]); reverse(node.child[1]); node.reversed = false;
        }
        if constexpr (has_action) {
            if (node.lazy.pending) {
                apply_node(node.child[0], *node.lazy.pending); apply_node(node.child[1], *node.lazy.pending);
                node.lazy.pending.reset();
            }
        }
    }
    void rotate(int vertex) {
        const int parent = nodes_[vertex].parent, grandparent = nodes_[parent].parent;
        const int side = nodes_[parent].child[1] == vertex;
        const int middle = nodes_[vertex].child[side ^ 1];
        if (!auxiliary_root(parent)) nodes_[grandparent].child[nodes_[grandparent].child[1] == parent] = vertex;
        nodes_[vertex].parent = grandparent;
        nodes_[parent].child[side] = middle;
        if (middle != 0) nodes_[middle].parent = parent;
        nodes_[vertex].child[side ^ 1] = parent; nodes_[parent].parent = vertex;
        pull(parent); pull(vertex);
    }
    void splay(int vertex) {
        ancestors_.clear();
        int current = vertex;
        ancestors_.push_back(current);
        while (!auxiliary_root(current)) { current = nodes_[current].parent; ancestors_.push_back(current); }
        for (auto i = ancestors_.size(); i != 0; --i) push(ancestors_[i - 1]);
        while (!auxiliary_root(vertex)) {
            const int parent = nodes_[vertex].parent, grandparent = nodes_[parent].parent;
            if (!auxiliary_root(parent)) {
                if ((nodes_[parent].child[0] == vertex) == (nodes_[grandparent].child[0] == parent)) rotate(parent);
                else rotate(vertex);
            }
            rotate(vertex);
        }
    }
    int access(int vertex) {
        int last = 0;
        for (int current = vertex; current != 0; current = nodes_[current].parent) {
            splay(current);
            nodes_[current].child[1] = last;
            if (last != 0) nodes_[last].parent = current;
            pull(current); last = current;
        }
        splay(vertex); return last;
    }
    void make_root_impl(int vertex) { access(vertex); reverse(vertex); }
    int find_root_impl(int vertex) {
        access(vertex);
        for (;;) { push(vertex); if (nodes_[vertex].child[0] == 0) break; vertex = nodes_[vertex].child[0]; }
        splay(vertex); return vertex;
    }
    bool expose(int first, int last) {
        make_root_impl(first); access(last);
        // 接続していればfirstはlastの補助木内へ入り、別成分なら親0のまま。
        return first == last || nodes_[first].parent != 0;
    }
public:
    explicit LinkCutTree(std::span<const T> values = {}, const T& identity = T{}, Op op = Op{},
                         Mapping mapping = Mapping{}, Composition composition = Composition{})
        : config_(std::make_shared<Config>(identity, std::move(op), std::move(mapping), std::move(composition))) { initialize(values); }
    explicit LinkCutTree(const std::vector<T>& values, const T& identity = T{}, Op op = Op{},
                         Mapping mapping = Mapping{}, Composition composition = Composition{})
        : config_(std::make_shared<Config>(identity, std::move(op), std::move(mapping), std::move(composition))) { initialize(values); }
    LinkCutTree(const LinkCutTree& other)
        : config_(other.config_), nodes_(other.nodes_), size_(other.size_), failed_(other.failed_) { ancestors_.reserve(size_); }
    LinkCutTree& operator=(const LinkCutTree& other) {
        if (this != &other) { LinkCutTree replacement(other); swap(replacement); }
        return *this;
    }
    LinkCutTree(LinkCutTree&& other) noexcept
        : config_(other.config_), nodes_(std::move(other.nodes_)), ancestors_(std::move(other.ancestors_)),
          size_(std::exchange(other.size_, 0)), failed_(std::exchange(other.failed_, false)) {
        other.nodes_.clear(); other.ancestors_.clear();
    }
    LinkCutTree& operator=(LinkCutTree&& other) noexcept {
        if (this != &other) { LinkCutTree replacement(std::move(other)); swap(replacement); }
        return *this;
    }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    bool usable() const noexcept { return !failed_; }
    void reset(std::span<const T> values) { LinkCutTree replacement(values, config_); swap(replacement); }
    void reset(const std::vector<T>& values) { LinkCutTree replacement(values, config_); swap(replacement); }
    void clear() noexcept {
        std::vector<Node>().swap(nodes_); std::vector<int>().swap(ancestors_);
        size_ = 0; failed_ = false;
    }
    void swap(LinkCutTree& other) noexcept {
        config_.swap(other.config_); nodes_.swap(other.nodes_); ancestors_.swap(other.ancestors_);
        std::swap(size_, other.size_); std::swap(failed_, other.failed_);
    }
    friend void swap(LinkCutTree& first, LinkCutTree& second) noexcept { first.swap(second); }
    bool connected(int first, int last) {
        check_vertex(first); check_vertex(last);
        return operate([&] { return first == last || find_root_impl(first + 1) == find_root_impl(last + 1); });
    }
    void make_root(int vertex) { check_vertex(vertex); operate([&] { make_root_impl(vertex + 1); }); }
    int find_root(int vertex) { check_vertex(vertex); return operate([&] { return find_root_impl(vertex + 1) - 1; }); }
    bool link(int first, int last) {
        check_vertex(first); check_vertex(last);
        return operate([&] {
            const int u = first + 1, v = last + 1;
            if (expose(u, v)) return false;
            nodes_[u].parent = v; return true;
        });
    }
    bool cut(int first, int last) {
        check_vertex(first); check_vertex(last);
        return operate([&] {
            const int u = first + 1, v = last + 1;
            if (!expose(u, v) || nodes_[v].child[0] != u || nodes_[u].child[1] != 0) return false;
            nodes_[v].child[0] = 0; nodes_[u].parent = 0; pull(v); return true;
        });
    }
    T get(int vertex) {
        check_vertex(vertex);
        return operate([&] { access(vertex + 1); return nodes_[vertex + 1].value; });
    }
    void set(int vertex, const T& value) {
        check_vertex(vertex);
        operate([&] { const int u = vertex + 1; access(u); nodes_[u].value = value; pull(u); });
    }
    template<class Fn> void transform(int vertex, Fn&& function) {
        check_vertex(vertex);
        operate([&] {
            const int u = vertex + 1; access(u);
            nodes_[u].value = std::invoke(std::forward<Fn>(function), std::as_const(nodes_[u].value)); pull(u);
        });
    }
    std::optional<T> path_prod(int first, int last) {
        check_vertex(first); check_vertex(last);
        return operate([&]() -> std::optional<T> {
            if (!expose(first + 1, last + 1)) return std::nullopt;
            return nodes_[last + 1].forward;
        });
    }
    std::size_t path_size(int first, int last) {
        check_vertex(first); check_vertex(last);
        return operate([&] { return expose(first + 1, last + 1) ? nodes_[last + 1].count : std::size_t{0}; });
    }
    std::optional<std::size_t> path_length(int first, int last) {
        const auto count = path_size(first, last);
        return count == 0 ? std::nullopt : std::optional<std::size_t>(count - 1);
    }
    std::optional<int> kth_on_path(int first, int last, std::size_t k) {
        check_vertex(first); check_vertex(last);
        return operate([&]() -> std::optional<int> {
            if (!expose(first + 1, last + 1) || k >= nodes_[last + 1].count) return std::nullopt;
            int current = last + 1;
            for (;;) {
                push(current);
                const auto left = nodes_[nodes_[current].child[0]].count;
                if (k < left) current = nodes_[current].child[0];
                else if (k == left) { splay(current); return current - 1; }
                else { k -= left + 1; current = nodes_[current].child[1]; }
            }
        });
    }
    void apply(int vertex, const Action& action) requires(has_action) {
        transform(vertex, [&](const T& value) { return std::invoke(config_->mapping, action, value, std::size_t{1}); });
    }
    bool apply(int first, int last, const Action& action) requires(has_action) {
        check_vertex(first); check_vertex(last);
        return operate([&] {
            if (!expose(first + 1, last + 1)) return false;
            apply_node(last + 1, action); return true;
        });
    }
    std::optional<int> lca(int root, int first, int last) {
        check_vertex(root); check_vertex(first); check_vertex(last);
        return operate([&]() -> std::optional<int> {
            const int r = root + 1, u = first + 1, v = last + 1;
            make_root_impl(r);
            if (find_root_impl(u) != r || find_root_impl(v) != r) return std::nullopt;
            access(u); return access(v) - 1;
        });
    }
};
#endif


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
#include <vector>
inline constexpr std::uint64_t modulus=998244353;
struct AddMod {std::uint32_t operator()(std::uint32_t a,std::uint32_t b)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{a}+b)%modulus);}};
struct AffineAction {std::uint32_t a,b;};
struct AffineMapping {std::uint32_t operator()(const AffineAction&f,std::uint32_t value,std::size_t n)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{f.a}*value+std::uint64_t{f.b}*(n%modulus))%modulus);}};
struct AffineComposition {AffineAction operator()(const AffineAction&f,const AffineAction&g)const noexcept{return {static_cast<std::uint32_t>(std::uint64_t{f.a}*g.a%modulus),static_cast<std::uint32_t>((std::uint64_t{f.a}*g.b+f.b)%modulus)};}};
int main(){
    int n{},q{};verify_in>>n>>q;
    std::vector<std::uint32_t> values(n);for(auto& value:values)verify_in>>value;
    LinkCutTree<std::uint32_t,AddMod,AffineAction,AffineMapping,AffineComposition> tree(values,0);
    for(int i=1;i<n;++i)tree.link(i-1,i);
    while(q--){
        int type{},left{},right{};verify_in>>type>>left>>right;
        if(type==0){AffineAction f{};verify_in>>f.a>>f.b;tree.apply(left,right-1,f);}
        else verify_out<<*tree.path_prod(left,right-1)<<'\n';
    }
}
