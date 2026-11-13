#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/practice2/tasks/practice2_e

// Library: Graph/Min_Cost_Flow.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Min_Cost_Flow.cpp
// 参考（最小費用流の分類）: https://ei1333.github.io/library/graph/flow/primal-dual.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/min_cost_flow.md
//
// API一覧
// MinCostFlow<Capacity=std::int64_t,Cost=std::int64_t>(n=0): n頂点。整数型、Costは符号付き、各型64bit以下。
// add_edge(u,v,capacity,cost): 有向辺追加、size_tのIDを返す。自己辺・多重辺・負費用可。償却O(1)。
// 容量は非負、辺費用は[-Cost::max,Cost::max]。逆費用を作れないCost::minは常にoverflow_error。
// flow(s,t,limit=max): 追加流量と追加費用のpair。s==t/limit==0は{0,0}。残余を更新して継続可能。
// slope(s,t,limit=max): {0,0}からの追加流量・費用の折れ線。限界費用が同じ区間は統合する。
// 初期化は仮想始点からBellman-Ford（負辺なしO(n+m)、一般O(nm)）。残余の負閉路は到達性に関係なく拒否。
// 負閉路の検出はdomain_error。ただし先に費用範囲を越えればoverflow_error。
// 増流はポテンシャル付きDijkstra。A回の増流でO(nm+A(n+m)log(n+m+1))、領域O(n+m)。再帰なし。
// 全残余辺のポテンシャルを保ち、同じ始点・終点への追加呼び出しを許す。流量保存の保証は利用側の責任。
// 換算費用・距離候補・ポテンシャル・限界費用・追加費用の各段階はCostに収まること。範囲外はSKIPでもoverflow_error。
// get_edge(id)/edges(): Edge{from,to,capacity,flow,cost}のコピー/一覧。O(1)/O(m)。
// change_edge(id,capacity,flow)/change_cost(id,cost): 元辺の状態/費用を変更しポテンシャルを無効化。O(1)。
// reset_flow(): 全元辺の流量を0へ戻す。O(m)。min_cut(s): 残余で到達可能なbool配列。O(n+m)。
// size()/edge_count(): 頂点数/元辺数。O(1)。reserve_edges(m): 元辺の位置表を予約。
// reset(n=0)/swap(other): 空のn頂点へ/O(1)交換。コピーは残余を独立所有、ムーブ元は空。
// 例外時にも完了した増流は残る場合があるが、途中の増流を片方向だけ更新しない。
// SKIP_RUNTIME_CHECKSは端点・負容量等の入力検査だけを除く。負閉路・費用範囲判定は維持する。
// SIRKIT_FORCE_STANDARD_ARITHMETIC: 128bit整数拡張・算術組込み関数を除き、標準C++23の演算を使う。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_MIN_COST_FLOW_DEFINED
#define SIRKIT_MIN_COST_FLOW_DEFINED
template<class Capacity = std::int64_t, class Cost = std::int64_t>
class MinCostFlow {
    static_assert(std::is_integral_v<Capacity> && !std::is_same_v<Capacity, bool> && sizeof(Capacity) <= 8,
                  "capacity must be a non-bool integer of at most 64 bits");
    static_assert(std::is_integral_v<Cost> && std::is_signed_v<Cost> && sizeof(Cost) <= 8,
                  "cost must be a signed integer of at most 64 bits");
public:
    struct Edge { int from; int to; Capacity capacity; Capacity flow; Cost cost; };
private:
    struct Arc { int to; int reverse; Capacity residual; Cost cost; };
    using HeapEntry = std::pair<Cost, int>;
    std::vector<std::vector<Arc>> graph_;
    std::vector<std::pair<int, int>> positions_;
    std::vector<Cost> potential_, distance_;
    std::vector<int> previous_vertex_, previous_edge_, settled_;
    std::vector<unsigned char> known_;
    std::vector<HeapEntry> heap_;
    bool potential_valid_ = false;
    static std::size_t checked_size(int n) {
        if (n < 0) throw std::length_error("MinCostFlow: negative vertex count");
        return static_cast<std::size_t>(n);
    }
    [[noreturn]] static void cost_overflow() { throw std::overflow_error("MinCostFlow: cost arithmetic out of range"); }
    static Cost add(Cost a, Cost b) {
#if !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        Cost result;
        if (__builtin_add_overflow(a, b, &result)) cost_overflow();
        return result;
#else
        if ((b > 0 && a > std::numeric_limits<Cost>::max() - b) ||
            (b < 0 && a < std::numeric_limits<Cost>::min() - b)) cost_overflow();
        return static_cast<Cost>(a + b);
#endif
    }
    static Cost subtract(Cost a, Cost b) {
#if !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        Cost result;
        if (__builtin_sub_overflow(a, b, &result)) cost_overflow();
        return result;
#else
        if ((b > 0 && a < std::numeric_limits<Cost>::min() + b) ||
            (b < 0 && a > std::numeric_limits<Cost>::max() + b)) cost_overflow();
        return static_cast<Cost>(a - b);
#endif
    }
    static Cost add_minus(Cost a, Cost b, Cost c) {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        const __int128_t value = static_cast<__int128_t>(a) + b - c;
        if (value < std::numeric_limits<Cost>::min() || value > std::numeric_limits<Cost>::max()) cost_overflow();
        return static_cast<Cost>(value);
#else
        // 同符号の差を先に取れば、中間だけがはみ出す場合も正しく扱える。
        return (b < 0) == (c < 0) ? add(a, subtract(b, c)) : subtract(add(a, b), c);
#endif
    }
    static Cost add_product(Cost total, Cost unit, Capacity amount) {
#if (defined(__SIZEOF_INT128__)) && (!defined(SIRKIT_FORCE_STANDARD_ARITHMETIC))
        const __int128_t value = static_cast<__int128_t>(total) + static_cast<__int128_t>(unit) * static_cast<__int128_t>(amount);
        if (value < std::numeric_limits<Cost>::min() || value > std::numeric_limits<Cost>::max()) cost_overflow();
        return static_cast<Cost>(value);
#else
        if (unit == 0) return total;
        using U = std::make_unsigned_t<Cost>;
        const U magnitude = unit < 0 ? static_cast<U>(U{0} - static_cast<U>(unit)) : static_cast<U>(unit);
        const U allowance = unit > 0 ? static_cast<U>(static_cast<U>(std::numeric_limits<Cost>::max()) - static_cast<U>(total))
                                     : static_cast<U>(static_cast<U>(total) - static_cast<U>(std::numeric_limits<Cost>::min()));
        const auto count = static_cast<std::uint64_t>(amount);
        if (count > static_cast<std::uint64_t>(allowance) / static_cast<std::uint64_t>(magnitude)) cost_overflow();
        const U product = static_cast<U>(static_cast<std::uint64_t>(magnitude) * count);
        const U bits = unit > 0 ? static_cast<U>(static_cast<U>(total) + product) : static_cast<U>(static_cast<U>(total) - product);
        return static_cast<Cost>(bits);
#endif
    }
    static void representable_reverse(Cost cost) {
        if (cost == std::numeric_limits<Cost>::min()) cost_overflow();
    }
    void initialize_potential() {
        std::vector<Cost> candidate(graph_.size(), Cost{});
        for (int pass = 0; pass < size(); ++pass) {
            bool changed = false;
            for (int u = 0; u < size(); ++u) for (const auto& edge : graph_[u]) if (edge.residual != 0) {
                const Cost next = add(candidate[u], edge.cost);
                if (next < candidate[edge.to]) { candidate[edge.to] = next; changed = true; }
            }
            if (!changed) { potential_.swap(candidate); potential_valid_ = true; return; }
            if (pass == size() - 1) throw std::domain_error("MinCostFlow: negative residual cycle");
        }
        potential_.swap(candidate); potential_valid_ = true;
    }
    void queue(Cost distance, int vertex) {
        heap_.emplace_back(distance, vertex); std::push_heap(heap_.begin(), heap_.end(), std::greater<HeapEntry>{});
    }
    bool shortest_path(int source, int sink) {
        std::fill(known_.begin(), known_.end(), 0); settled_.clear(); heap_.clear();
        known_[source] = 1; distance_[source] = 0; queue(0, source);
        while (!heap_.empty()) {
            std::pop_heap(heap_.begin(), heap_.end(), std::greater<HeapEntry>{});
            const auto [distance, v] = heap_.back(); heap_.pop_back();
            if (distance != distance_[v]) continue;
            settled_.push_back(v);
            if (v == sink) return true;
            for (std::size_t i = 0; i < graph_[v].size(); ++i) {
                const auto& edge = graph_[v][i]; if (edge.residual == 0) continue;
                const Cost reduced = add_minus(edge.cost, potential_[v], potential_[edge.to]);
                if (reduced < 0) throw std::domain_error("MinCostFlow: invalid reduced cost");
                const Cost next = add(distance, reduced);
                if (!known_[edge.to] || next < distance_[edge.to]) {
                    known_[edge.to] = 1; distance_[edge.to] = next;
                    previous_vertex_[edge.to] = v; previous_edge_[edge.to] = static_cast<int>(i); queue(next, edge.to);
                }
            }
        }
        return false;
    }
    template<class Progress>
    std::pair<Capacity, Cost> run(int source, int sink, Capacity limit, Progress&& progress) {
        if (source == sink || limit == 0) return {0, 0};
        settled_.reserve(graph_.size());
        if (!potential_valid_) initialize_potential();
        Capacity result = 0; Cost total = 0;
        while (result != limit && shortest_path(source, sink)) {
            Capacity amount = limit - result;
            for (int v = sink; v != source; v = previous_vertex_[v])
                amount = std::min(amount, graph_[previous_vertex_[v]][previous_edge_[v]].residual);
            const Cost unit = add_minus(distance_[sink], potential_[sink], potential_[source]);
            const Cost next_total = add_product(total, unit, amount);
            potential_valid_ = false;
            for (int v : settled_) potential_[v] = add_minus(potential_[v], distance_[v], distance_[sink]);
            potential_valid_ = true;
            for (int v = sink; v != source; v = previous_vertex_[v]) {
                auto& edge = graph_[previous_vertex_[v]][previous_edge_[v]];
                edge.residual -= amount; graph_[edge.to][edge.reverse].residual += amount;
            }
            result += amount; total = next_total; progress(result, total, unit);
        }
        return {result, total};
    }
public:
    explicit MinCostFlow(int n = 0) : graph_(checked_size(n)), potential_(graph_.size()), distance_(graph_.size()),
        previous_vertex_(graph_.size()), previous_edge_(graph_.size()), known_(graph_.size()) {}
    MinCostFlow(const MinCostFlow&) = default;
    MinCostFlow(MinCostFlow&& other) noexcept { swap(other); }
    MinCostFlow& operator=(MinCostFlow other) noexcept { swap(other); return *this; }
    void swap(MinCostFlow& other) noexcept {
        graph_.swap(other.graph_); positions_.swap(other.positions_); potential_.swap(other.potential_); distance_.swap(other.distance_);
        previous_vertex_.swap(other.previous_vertex_); previous_edge_.swap(other.previous_edge_); settled_.swap(other.settled_);
        known_.swap(other.known_); heap_.swap(other.heap_); std::swap(potential_valid_, other.potential_valid_);
    }
    void reset(int n = 0) { MinCostFlow replacement(n); swap(replacement); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    std::size_t edge_count() const noexcept { return positions_.size(); }
    void reserve_edges(std::size_t count) { positions_.reserve(count); }
    std::size_t add_edge(int from, int to, Capacity capacity, Cost cost) {
        representable_reverse(cost);
        const auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
        if (graph_[from].size() >= maximum || graph_[to].size() >= maximum ||
            (from == to && graph_[from].size() >= maximum - 1)) throw std::length_error("MinCostFlow: too many incident edges");
        const int forward = static_cast<int>(graph_[from].size()), reverse = static_cast<int>(graph_[to].size()) + static_cast<int>(from == to);
        graph_[from].push_back({to, reverse, capacity, cost});
        try { graph_[to].push_back({from, forward, 0, static_cast<Cost>(-cost)}); }
        catch (...) { graph_[from].pop_back(); throw; }
        try { positions_.emplace_back(from, forward); }
        catch (...) { graph_[to].pop_back(); graph_[from].pop_back(); throw; }
        potential_valid_ = false; return positions_.size() - 1;
    }
    Edge get_edge(std::size_t id) const {
        const auto [from, position] = positions_[id]; const auto& edge = graph_[from][position];
        const Capacity sent = graph_[edge.to][edge.reverse].residual;
        return {from, edge.to, static_cast<Capacity>(edge.residual + sent), sent, edge.cost};
    }
    std::vector<Edge> edges() const {
        std::vector<Edge> result; result.reserve(positions_.size());
        for (std::size_t id = 0; id < positions_.size(); ++id) result.push_back(get_edge(id));
        return result;
    }
    void change_edge(std::size_t id, Capacity capacity, Capacity sent) {
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.residual = capacity - sent; graph_[edge.to][edge.reverse].residual = sent; potential_valid_ = false;
    }
    void change_cost(std::size_t id, Cost cost) {
        representable_reverse(cost);
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.cost = cost; graph_[edge.to][edge.reverse].cost = static_cast<Cost>(-cost); potential_valid_ = false;
    }
    void reset_flow() noexcept {
        for (const auto [from, position] : positions_) {
            auto& edge = graph_[from][position]; auto& reverse = graph_[edge.to][edge.reverse];
            edge.residual += reverse.residual; reverse.residual = 0;
        }
        potential_valid_ = false;
    }
    std::pair<Capacity, Cost> flow(int source, int sink, Capacity limit = std::numeric_limits<Capacity>::max()) {
        return run(source, sink, limit, [](Capacity, Cost, Cost) {});
    }
    std::vector<std::pair<Capacity, Cost>> slope(int source, int sink, Capacity limit = std::numeric_limits<Capacity>::max()) {
        std::vector<std::pair<Capacity, Cost>> result{{0, 0}};
        bool previous = false; Cost last = 0;
        run(source, sink, limit, [&](Capacity flow, Cost cost, Cost unit) {
            if (previous && last == unit) result.back() = {flow, cost}; else result.emplace_back(flow, cost);
            previous = true; last = unit;
        });
        return result;
    }
    std::vector<bool> min_cut(int source) const {
        std::vector<bool> reached(graph_.size()); std::vector<int> queue;
        queue.reserve(graph_.size()); queue.push_back(source); reached[source] = true;
        for (std::size_t i = 0; i < queue.size(); ++i) for (const auto& edge : graph_[queue[i]])
            if (edge.residual != 0 && !reached[edge.to]) { reached[edge.to] = true; queue.push_back(edge.to); }
        return reached;
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

#include <cstdint>
#include <string>
#include <vector>
int main(){
 int n{},k{};verify_in>>n>>k;const int source=2*n,sink=source+1;MinCostFlow<int,std::int64_t> network(sink+1);
 for(int i=0;i<n;++i){network.add_edge(source,i,k,0);network.add_edge(n+i,sink,k,0);}
 std::vector<std::vector<std::size_t>> id(n,std::vector<std::size_t>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j){std::int64_t value{};verify_in>>value;id[i][j]=network.add_edge(i,n+j,1,-value);}
 network.add_edge(source,sink,n*k,0);const auto result=network.flow(source,sink,n*k);verify_out<<-result.second<<'\n';
 for(int i=0;i<n;++i){std::string row(n,'.');for(int j=0;j<n;++j)if(network.get_edge(id[i][j]).flow)row[j]='X';verify_out<<row<<'\n';}
}
