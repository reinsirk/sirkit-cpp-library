#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum

// Library: Graph/Offline_Dynamic_Connectivity.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Offline_Dynamic_Connectivity.cpp
// 参考: https://nyaannyaan.github.io/library/graph/offline-dynamic-connectivity.hpp
// 区間分割とrollback Union-Findの数学的構成を参考に独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/offline_dynamic_connectivity.md
// 頂点番号・有効辺IDの検査はSKIP_RUNTIME_CHECKSで除外。内部の容量・添字上限保護は維持。
// API一覧
// OfflineDynamicConnectivity(n=0) / (span<const int64_t> weights): n頂点、初期値0/指定値。O(n)。
// 頂点は0..n-1。自己辺・多重辺可。辺IDは発行元（独立コピーを含む）の履歴内でのみ使用する。
// add_edge(u,v): 有効な辺を追加しIDを返す。remove_edge(id): 有効なその辺を削除。償却O(1)。
// add_vertex(v,delta): 以降の照会の頂点値に加算。償却O(1)。
// query_same(u,v) / query_size(v) / query_component_count() / query_component_sum(v):
//   現時点の照会を記録し回答添字を返す。償却O(1)。sameの回答は0/1。
// solve() const: 照会順のvector<int64_t>。記録は消さず、追記・再実行可。空照会は空vector。
//   E=追加辺数、A=頂点加算数、Q=照会数としてO(n+(E+A+Q)log(Q+1)log(n+1))時間、
//   O(n+(E+A)log(Q+1)+Q)作業領域。rollbackは内部処理で、外部への状態巻戻しAPIではない。
// size()/empty(), edge_count()/query_count()/addition_count(): O(1)。edge_countは削除済みも含む。
// reserve_edges/additions/queries(cap): 対応する記録領域を予約。O(現在の記録数)。
// clear_history(): 初期頂点値を維持し全記録を消去。reset(n)/reset(weights): 初期化。clear(): 0頂点。
// copyは全記録を独立所有。move元は0頂点で再利用可。swapはO(1)。
// 確保失敗時は内容を維持。solveは常に元オブジェクトを維持。span引数との自己参照可。
// 頂点値と成分和は、内部の加算順を含め全てint64_tに収まること。

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_CONNECTIVITY_ROLLBACK_STATE_DEFINED
#define SIRKIT_CONNECTIVITY_ROLLBACK_STATE_DEFINED
namespace reinsirk::detail {
class ConnectivityRollbackState {
    struct Change { int root, child, root_size, child_size; std::int64_t root_sum; };
    std::vector<int> parent_;
    std::vector<std::int64_t> sum_;
    std::vector<Change> history_;
    int components_;
public:
    explicit ConnectivityRollbackState(std::span<const std::int64_t> weights, std::size_t reserve)
        : parent_(weights.size(), -1), sum_(weights.begin(), weights.end()), components_(static_cast<int>(weights.size())) {
        history_.reserve(reserve);
    }
    int leader(int v) const noexcept { while (parent_[v] >= 0) v = parent_[v]; return v; }
    bool same(int u, int v) const noexcept { return leader(u) == leader(v); }
    int size(int v) const noexcept { return -parent_[leader(v)]; }
    std::int64_t sum(int v) const noexcept { return sum_[leader(v)]; }
    int components() const noexcept { return components_; }
    std::size_t snapshot() const noexcept { return history_.size(); }
    void merge(int u, int v) {
        u = leader(u); v = leader(v);
        if (u == v) return;
        if (parent_[u] > parent_[v]) std::swap(u, v);
        history_.push_back({u, v, parent_[u], parent_[v], sum_[u]});
        parent_[u] += parent_[v]; parent_[v] = u; sum_[u] += sum_[v]; --components_;
    }
    void add(int v, std::int64_t delta) {
        v = leader(v);
        history_.push_back({v, -1, 0, 0, sum_[v]});
        sum_[v] += delta;
    }
    void rollback(std::size_t snapshot) noexcept {
        while (history_.size() != snapshot) {
            const auto c = history_.back(); history_.pop_back(); sum_[c.root] = c.root_sum;
            if (c.child != -1) { parent_[c.root] = c.root_size; parent_[c.child] = c.child_size; ++components_; }
        }
    }
};
}
#endif

#ifndef SIRKIT_OFFLINE_DYNAMIC_CONNECTIVITY_DEFINED
#define SIRKIT_OFFLINE_DYNAMIC_CONNECTIVITY_DEFINED
class OfflineDynamicConnectivity {
    static constexpr std::size_t ongoing = std::numeric_limits<std::size_t>::max();
    struct Edge { int u, v; std::size_t first, last; };
    struct Addition { int v; std::int64_t delta; std::size_t first; };
    enum class Kind { same, size, count, sum };
    struct Query { Kind kind; int u, v; };
    std::vector<std::int64_t> initial_;
    std::vector<Edge> edges_;
    std::vector<Addition> additions_;
    std::vector<Query> queries_;
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= initial_.size()) throw std::out_of_range("OfflineDynamicConnectivity vertex");
#else
        (void)v;
#endif
    }
    std::size_t query(Kind kind, int u, int v = 0) {
        const auto id = queries_.size(); queries_.push_back({kind, u, v}); return id;
    }
    template<class F> void intervals(std::size_t base, F&& f) const {
        const auto q = queries_.size();
        const auto visit = [&](std::size_t id, std::size_t first, std::size_t last) {
            for (auto l = first + base, r = last + base; l < r; l >>= 1, r >>= 1) {
                if (l & 1) f(l++, id);
                if (r & 1) f(--r, id);
            }
        };
        for (std::size_t i = 0; i < edges_.size(); ++i) {
            const auto& e = edges_[i]; visit(i, e.first, e.last == ongoing ? q : e.last);
        }
        for (std::size_t i = 0; i < additions_.size(); ++i) visit(edges_.size() + i, additions_[i].first, q);
    }
public:
    explicit OfflineDynamicConnectivity(int n = 0) {
        if (n < 0) throw std::length_error("OfflineDynamicConnectivity size");
        initial_.resize(static_cast<std::size_t>(n));
    }
    explicit OfflineDynamicConnectivity(std::span<const std::int64_t> weights) {
        if (weights.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("OfflineDynamicConnectivity size");
        initial_.assign(weights.begin(), weights.end());
    }
    OfflineDynamicConnectivity(const OfflineDynamicConnectivity&) = default;
    OfflineDynamicConnectivity& operator=(const OfflineDynamicConnectivity& other) {
        if (this != &other) { OfflineDynamicConnectivity copy(other); swap(copy); } return *this;
    }
    OfflineDynamicConnectivity(OfflineDynamicConnectivity&& other) noexcept { swap(other); }
    OfflineDynamicConnectivity& operator=(OfflineDynamicConnectivity&& other) noexcept {
        if (this != &other) { clear(); swap(other); } return *this;
    }
    void swap(OfflineDynamicConnectivity& other) noexcept {
        initial_.swap(other.initial_); edges_.swap(other.edges_); additions_.swap(other.additions_); queries_.swap(other.queries_);
    }
    friend void swap(OfflineDynamicConnectivity& a, OfflineDynamicConnectivity& b) noexcept { a.swap(b); }
    int size() const noexcept { return static_cast<int>(initial_.size()); }
    bool empty() const noexcept { return initial_.empty(); }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    std::size_t addition_count() const noexcept { return additions_.size(); }
    std::size_t query_count() const noexcept { return queries_.size(); }
    void reserve_edges(std::size_t cap) { edges_.reserve(cap); }
    void reserve_additions(std::size_t cap) { additions_.reserve(cap); }
    void reserve_queries(std::size_t cap) { queries_.reserve(cap); }
    std::size_t add_edge(int u, int v) {
        check_vertex(u); check_vertex(v);
        const auto id = edges_.size(); edges_.push_back({u, v, queries_.size(), ongoing}); return id;
    }
    void remove_edge(std::size_t id) {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edges_.size() || edges_[id].last != ongoing) throw std::out_of_range("OfflineDynamicConnectivity active edge");
#endif
        edges_[id].last = queries_.size();
    }
    void add_vertex(int v, std::int64_t delta) { check_vertex(v); additions_.push_back({v, delta, queries_.size()}); }
    std::size_t query_same(int u, int v) { check_vertex(u); check_vertex(v); return query(Kind::same, u, v); }
    std::size_t query_size(int v) { check_vertex(v); return query(Kind::size, v); }
    std::size_t query_component_count() { return query(Kind::count, 0); }
    std::size_t query_component_sum(int v) { check_vertex(v); return query(Kind::sum, v); }
    std::vector<std::int64_t> solve() const {
        const auto q = queries_.size(); if (!q) return {};
        const auto limit = std::vector<std::size_t>().max_size();
        std::size_t base = 1;
        while (base < q) {
            if (base > (limit - 1) / 4) throw std::length_error("OfflineDynamicConnectivity timeline");
            base *= 2;
        }
        if (base > (limit - 1) / 2 || additions_.size() > ongoing - edges_.size()) throw std::length_error("OfflineDynamicConnectivity capacity");
        const auto nodes = base * 2;
        std::vector<std::size_t> offsets(nodes + 1);
        intervals(base, [&](std::size_t node, std::size_t) {
            if (offsets[node + 1] == limit) throw std::length_error("OfflineDynamicConnectivity intervals");
            ++offsets[node + 1];
        });
        for (std::size_t i = 1; i <= nodes; ++i) {
            if (offsets[i] > limit - offsets[i - 1]) throw std::length_error("OfflineDynamicConnectivity intervals");
            offsets[i] += offsets[i - 1];
        }
        std::vector<std::size_t> ids(offsets.back()), cursor(offsets.begin(), offsets.end() - 1);
        intervals(base, [&](std::size_t node, std::size_t id) { ids[cursor[node]++] = id; });
        const auto merge_bound = std::min(edges_.size(), initial_.empty() ? std::size_t{0} : initial_.size() - 1);
        reinsirk::detail::ConnectivityRollbackState uf(initial_, merge_bound + additions_.size());
        struct Frame { std::size_t node, snapshot; bool exit; };
        std::vector<Frame> stack; stack.reserve(2 * std::bit_width(base) + 1); stack.push_back({1, 0, false});
        std::vector<std::int64_t> answers(q);
        while (!stack.empty()) {
            const auto frame = stack.back(); stack.pop_back();
            if (frame.exit) { uf.rollback(frame.snapshot); continue; }
            const auto state = uf.snapshot();
            for (auto i = offsets[frame.node]; i < offsets[frame.node + 1]; ++i) {
                const auto id = ids[i];
                if (id < edges_.size()) { const auto& e = edges_[id]; uf.merge(e.u, e.v); }
                else { const auto& a = additions_[id - edges_.size()]; uf.add(a.v, a.delta); }
            }
            if (frame.node >= base) {
                const auto index = frame.node - base;
                if (index < q) {
                    const auto& x = queries_[index];
                    switch (x.kind) {
                    case Kind::same: answers[index] = uf.same(x.u, x.v); break;
                    case Kind::size: answers[index] = uf.size(x.u); break;
                    case Kind::count: answers[index] = uf.components(); break;
                    case Kind::sum: answers[index] = uf.sum(x.u); break;
                    }
                }
                uf.rollback(state);
            } else {
                stack.push_back({frame.node, state, true});
                stack.push_back({frame.node * 2 + 1, 0, false}); stack.push_back({frame.node * 2, 0, false});
            }
        }
        return answers;
    }
    void clear_history() noexcept { edges_.clear(); additions_.clear(); queries_.clear(); }
    void reset(int n = 0) { OfflineDynamicConnectivity next(n); swap(next); }
    void reset(std::span<const std::int64_t> weights) { OfflineDynamicConnectivity next(weights); swap(next); }
    void clear() noexcept { initial_.clear(); clear_history(); }
};
#endif


// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>
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


int main() {
    int n{}, q{};
    if (!(verify_in >> n)) return 0;
    verify_in >> q;
    std::vector<std::int64_t> weights(static_cast<std::size_t>(n));
    for (auto& value : weights) verify_in >> value;
    OfflineDynamicConnectivity graph(weights);
    std::map<std::pair<int, int>, std::size_t> edges;
    for (int i = 0; i < q; ++i) {
        int type{}, u{};
        verify_in >> type; verify_in >> u;
        if (type < 2) {
            int v{}; verify_in >> v;
            const std::pair<int, int> endpoints = std::minmax(u, v);
            if (type == 0) edges[endpoints] = graph.add_edge(u, v);
            else { graph.remove_edge(edges.at(endpoints)); edges.erase(endpoints); }
        } else if (type == 2) {
            std::int64_t delta{}; verify_in >> delta; graph.add_vertex(u, delta);
        } else graph.query_component_sum(u);
    }
    for (const auto answer : graph.solve()) verify_out << answer << '\n';
}
