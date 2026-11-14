#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/tree_path_composite_sum

// Library: Graph/Rerooting_DP.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Rerooting_DP.cpp
// 数学的参考: https://nyaannyaan.github.io/library/tree/rerooting.hpp.html
// 参考: https://ei1333.github.io/library/graph/tree/rerooting.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/rerooting_dp.md
//
// API一覧
// RerootingDP<T>(n=0): 頂点0..n-1の森を保持。Tはbool/cv修飾型以外でコピー・代入可能。デフォルト構築は不要。
// add_edge(u,v): 無向辺を追加し、追加順のsize_t辺番号を返す。各頂点の隣接順を保持、償却O(1)。
// reserve_edges(m)/reserve_neighbors(v,d): 辺表/頂点vの隣接列を予約。再確保時は保持要素数に比例。
// size()/empty()/edge_count()/has_results(): 頂点数/0頂点か/辺数/現在のグラフに対する結果があるか、O(1)。
// edge(id)/edges(): 両端pairのコピー/辺表のconst span。neighbors(v): 出辺arc番号のconst span。
// 辺idのarcは2*idがu->v、2*id+1がv->u。隣接列はadd_edgeの挿入順。
// build(identity,merge,to_vertex,to_edge): 森の全根DPを構築し、結果のconst spanを返す。
// merge(const T&,const T&)->Tは結合的でidentityが両側単位元。非可換でもよい。
// to_vertex(aggregate,v)->T、to_edge(vertex_value,from,to,edge_id)->Tをconst参照で呼ぶ。
// 全ての頂点で、入ってくる辺メッセージを隣接の挿入順にmergeし、to_vertexで根の値にする。
// 出辺の値は、その辺だけを除いた入辺集約をto_vertexへ渡し、to_edgeで変換する。
// 値の演算/コピーをO(1)として、buildはO(n+m)時間・追加領域。探索とDPは全て反復。
// values()/operator[](v): 全根結果のconst span/頂点vの結果へのconst参照。
// messages()/message(id,reverse=false): 全arcの辺メッセージのconst span/片方向のconst参照。
// reverse=falseは追加時u->v、trueはv->u。values/messagesの取得にはhas_results()==trueが前提。
// component_count(): 構築済み森の成分数、O(1)。孤立点ではto_vertex(identity,v)が結果。
// reset(n=0)/clear()/swap(): 辺も結果も消してn頂点/0頂点に戻す/交換。コピーは独立、move元は空。
// build失敗時はグラフと以前の結果を保持。add_edge成功後は以前の結果を無効とし、失敗時は保つ。
// add_edge失敗時、辺/隣接配列の容量と既存spanは復元しない。旧結果のspanは維持する。
// コールバックの外部状態は巻き戻さない。同じグラフをコールバックから変更しない。
// span/参照は対応する配列の再確保・再構築・reset/clear/代入で無効化し得る。
// 自己辺/多重辺/循環は不可。既定では自己辺をadd時、循環を実際の探索中に検査する。
// SKIP_RUNTIME_CHECKSで入力/結果の有無/森の検査を除く。内部の件数・添字上限は維持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_REROOTING_DP_DEFINED
#define SIRKIT_REROOTING_DP_DEFINED
template<class T>
class RerootingDP {
    static_assert(std::copyable<T> && !std::is_same_v<T, bool> && std::is_same_v<T, std::remove_cv_t<T>> && std::is_nothrow_destructible_v<T>);
    std::vector<std::vector<std::size_t>> graph_;
    std::vector<std::pair<int,int>> edges_;
    std::vector<T> values_, messages_;
    bool ready_ = false;
    static std::size_t count(int n) {
        if (n < 0) throw std::length_error("RerootingDP vertex count");
        return static_cast<std::size_t>(n);
    }
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || v >= size()) throw std::out_of_range("RerootingDP vertex");
#else
        static_cast<void>(v);
#endif
    }
    void check_edge(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id >= edges_.size()) throw std::out_of_range("RerootingDP edge");
#else
        static_cast<void>(id);
#endif
    }
    void check_results() const {
#ifndef SKIP_RUNTIME_CHECKS
        if (!ready_) throw std::logic_error("RerootingDP build required");
#endif
    }
    int to(std::size_t arc) const noexcept { return arc & 1 ? edges_[arc / 2].first : edges_[arc / 2].second; }
public:
    explicit RerootingDP(int n = 0) : graph_(count(n)) {}
    RerootingDP(const RerootingDP&) = default;
    RerootingDP& operator=(const RerootingDP& other) {
        if (this != &other) { RerootingDP next(other); swap(next); }
        return *this;
    }
    RerootingDP(RerootingDP&& other) noexcept { swap(other); }
    RerootingDP& operator=(RerootingDP&& other) noexcept { RerootingDP next(std::move(other)); swap(next); return *this; }
    void swap(RerootingDP& other) noexcept {
        graph_.swap(other.graph_); edges_.swap(other.edges_); values_.swap(other.values_); messages_.swap(other.messages_);
        std::swap(ready_, other.ready_);
    }
    friend void swap(RerootingDP& a, RerootingDP& b) noexcept { a.swap(b); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    bool empty() const noexcept { return graph_.empty(); }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    bool has_results() const noexcept { return ready_; }
    std::span<const std::size_t> neighbors(int v) const { check_vertex(v); return graph_[v]; }
    std::pair<int,int> edge(std::size_t id) const { check_edge(id); return edges_[id]; }
    std::span<const std::pair<int,int>> edges() const noexcept { return edges_; }
    void reserve_edges(std::size_t capacity) { edges_.reserve(capacity); }
    void reserve_neighbors(int v, std::size_t capacity) { check_vertex(v); graph_[v].reserve(capacity); }
    std::size_t add_edge(int u, int v) {
        check_vertex(u); check_vertex(v);
#ifndef SKIP_RUNTIME_CHECKS
        if (u == v) throw std::invalid_argument("RerootingDP self loop");
#endif
        if (edges_.size() >= std::numeric_limits<std::size_t>::max() / 2)
            throw std::length_error("RerootingDP arc count");
        const auto id = edges_.size();
        edges_.emplace_back(u,v);
        try { graph_[u].push_back(2 * id); }
        catch (...) { edges_.pop_back(); throw; }
        try { graph_[v].push_back(2 * id + 1); }
        catch (...) { graph_[u].pop_back(); edges_.pop_back(); throw; }
        ready_ = false; return id;
    }
    template<class Merge, class ToVertex, class ToEdge>
    std::span<const T> build(const T& identity, const Merge& merge, const ToVertex& to_vertex, const ToEdge& to_edge) {
        static_assert(std::is_invocable_r_v<T,const Merge&,const T&,const T&>);
        static_assert(std::is_invocable_r_v<T,const ToVertex&,const T&,int>);
        static_assert(std::is_invocable_r_v<T,const ToEdge&,const T&,int,int,std::size_t>);
        const auto n = graph_.size(), m = edges_.size();
        const auto none = std::numeric_limits<std::size_t>::max();
        std::vector<std::size_t> parent(n, none);
        std::vector<int> order; order.reserve(n);
        std::size_t maximum_degree = 0;
        for (int start = 0; start < size(); ++start) if (parent[start] == none) {
            parent[start] = none - 1;
            const auto begin = order.size(); order.push_back(start);
            for (std::size_t position = begin; position < order.size(); ++position) {
                const int v = order[position];
                maximum_degree = std::max(maximum_degree, graph_[v].size());
                for (const auto arc : graph_[v]) {
                    if (arc == parent[v]) continue;
                    const int next = to(arc);
#ifndef SKIP_RUNTIME_CHECKS
                    if (parent[next] != none) throw std::invalid_argument("RerootingDP input is not a forest");
#endif
                    parent[next] = arc ^ 1; order.push_back(next);
                }
            }
        }
        std::vector<T> answer(n, identity), messages(2 * m, identity), prefix;
        if (maximum_degree == std::numeric_limits<std::size_t>::max())
            throw std::length_error("RerootingDP prefix count");
        prefix.reserve(maximum_degree + 1);
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            const int v = *it; T aggregate = identity;
            for (const auto arc : graph_[v]) if (arc != parent[v])
                aggregate = std::invoke(merge, std::as_const(aggregate), std::as_const(messages[arc ^ 1]));
            if (parent[v] < 2 * m) {
                const auto arc = parent[v];
                T value = std::invoke(to_vertex, std::as_const(aggregate), v);
                messages[arc] = std::invoke(to_edge, std::as_const(value), v, to(arc), arc / 2);
            }
        }
        for (const int v : order) {
            prefix.clear(); prefix.push_back(identity);
            for (const auto arc : graph_[v]) prefix.push_back(std::invoke(merge, std::as_const(prefix.back()), std::as_const(messages[arc ^ 1])));
            answer[v] = std::invoke(to_vertex, std::as_const(prefix.back()), v);
            T suffix = identity;
            for (std::size_t i = graph_[v].size(); i > 0; --i) {
                const auto arc = graph_[v][i - 1];
                if (arc != parent[v]) {
                    T aggregate = std::invoke(merge, std::as_const(prefix[i - 1]), std::as_const(suffix));
                    T value = std::invoke(to_vertex, std::as_const(aggregate), v);
                    messages[arc] = std::invoke(to_edge, std::as_const(value), v, to(arc), arc / 2);
                }
                suffix = std::invoke(merge, std::as_const(messages[arc ^ 1]), std::as_const(suffix));
            }
        }
        values_.swap(answer); messages_.swap(messages); ready_ = true;
        return values_;
    }
    std::span<const T> values() const { check_results(); return values_; }
    const T& operator[](int v) const { check_vertex(v); check_results(); return values_[v]; }
    std::span<const T> messages() const { check_results(); return messages_; }
    const T& message(std::size_t id, bool reverse = false) const { check_edge(id); check_results(); return messages_[2 * id + reverse]; }
    std::size_t component_count() const { check_results(); return graph_.size() - edges_.size(); }
    void reset(int n = 0) { RerootingDP next(n); swap(next); }
    void clear() noexcept { graph_.clear(); edges_.clear(); values_.clear(); messages_.clear(); ready_ = false; }
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
#include <vector>
struct State { std::int64_t sum{},count{}; };
int main() {
    constexpr std::int64_t mod=998244353;
    int n{}; verify_in >> n;
    std::vector<std::int64_t> values(static_cast<std::size_t>(n));
    for (auto& value : values) verify_in >> value;
    std::vector<std::int64_t> slope(static_cast<std::size_t>(n-1)),constant(static_cast<std::size_t>(n-1));
    RerootingDP<State> tree(n);
    for (int i=0;i<n-1;++i) {
        int u{},v{}; verify_in >> u >> v >> slope[static_cast<std::size_t>(i)] >> constant[static_cast<std::size_t>(i)];
        tree.add_edge(u,v);
    }
    tree.build(State{},
        [](const State& a,const State& b){ return State{(a.sum+b.sum)%mod,a.count+b.count}; },
        [&](const State& a,int v){ return State{(a.sum+values[static_cast<std::size_t>(v)])%mod,a.count+1}; },
        [&](const State& a,int,int,std::size_t id){ return State{(a.sum*slope[id]+a.count*constant[id])%mod,a.count}; });
    for (int v=0;v<n;++v) { if (v!=0) verify_out << ' '; verify_out << tree[v].sum; }
    verify_out << '\n';
}
