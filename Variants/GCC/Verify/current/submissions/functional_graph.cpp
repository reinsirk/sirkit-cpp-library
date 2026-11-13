#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc367/tasks/abc367_e

// Library: Graph/Functional_Graph.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Functional_Graph.cpp
// 参考: https://nyaannyaan.github.io/library/graph/functional-graph.hpp
// https://atcoder.jp/contests/abc367/tasks/abc367_e
// 入次数除去・周期分解・ダブリングを用いて独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/functional_graph.md
// 行先・頂点・周期IDの検査はSKIP_RUNTIME_CHECKSで除外。配列容量の上限保護は維持。
// API一覧
// FunctionalGraph() / FunctionalGraph(span<const int> successor): 空/各頂点の行先で構築。
// build(successor): 再構築。頂点0..n-1、各頂点に行先1つ。ラベル順に制約なし。自己辺可。
//   O(n log(n+1))時間・領域。入力走査中に行先を検査。失敗時は旧状態とviewを維持。
// size()/empty(), successor(v), successors(): 頂点数/空判定/行先/const span。O(1)。
// kth_successor(v,uint64_t k): k回進んだ頂点。全uint64_t範囲可。O(log(n+1))。
// distance(from,to): 初めて到達する辺数。到達不能は-1。O(log(n+1))。
// reachable(from,to): distance>=0。O(log(n+1))。
// info(v): CycleInfo {id,entry,depth,position,length}。O(1)。entryは最初の周期頂点、
//   depthは到達辺数、positionはそのentryの周期内添字、lengthは周期長。
// on_cycle(v), cycle_count(), component_size(v): 周期上か/周期数/同じ弱連結成分の頂点数。O(1)。
// cycle(id): successor方向に並ぶ周期頂点のconst span。O(1)。周期ID/始点は構築時の内部順。
// clear(): 空へ。copyは独立所有、copy代入は失敗時旧状態維持。move元は空で再利用可。swapはO(1)。
// viewは成功したbuild/clear/代入/所有権移動まで有効。buildの入力に自身のviewを渡してよい。

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_FUNCTIONAL_GRAPH_DEFINED
#define SIRKIT_FUNCTIONAL_GRAPH_DEFINED
class FunctionalGraph {
public:
    struct CycleInfo { int id, entry, depth, position, length; };
private:
    std::vector<int> next_, depth_, entry_, cycle_id_, position_, component_size_, cycle_vertices_, jump_;
    std::vector<std::size_t> cycle_offsets_;
    unsigned levels_ = 0;
    void check_vertex(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (v < 0 || static_cast<std::size_t>(v) >= next_.size()) throw std::out_of_range("FunctionalGraph vertex");
#else
        (void)v;
#endif
    }
    void initialize(std::span<const int> successors) {
        const auto n = successors.size();
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max())) throw std::length_error("FunctionalGraph size");
        next_.assign(successors.begin(), successors.end());
        std::vector<int> indegree(n), order; order.reserve(n);
        for (const int v : next_) {
#ifndef SKIP_RUNTIME_CHECKS
            if (v < 0 || static_cast<std::size_t>(v) >= n) throw std::out_of_range("FunctionalGraph successor");
#endif
            ++indegree[v];
        }
        for (std::size_t v = 0; v < n; ++v) if (!indegree[v]) order.push_back(static_cast<int>(v));
        for (std::size_t i = 0; i < order.size(); ++i) if (--indegree[next_[order[i]]] == 0) order.push_back(next_[order[i]]);
        depth_.resize(n); entry_.resize(n); position_.resize(n); cycle_id_.assign(n, -1);
        cycle_offsets_.push_back(0);
        for (std::size_t i = 0; i < n; ++i) if (indegree[i] && cycle_id_[i] == -1) {
            const auto id = static_cast<int>(component_size_.size());
            int v = static_cast<int>(i), position = 0;
            do {
                cycle_id_[v] = id; entry_[v] = v; position_[v] = position++;
                cycle_vertices_.push_back(v); v = next_[v];
            } while (v != static_cast<int>(i));
            component_size_.push_back(position); cycle_offsets_.push_back(cycle_vertices_.size());
        }
        int max_depth = 0;
        for (auto i = order.size(); i != 0; --i) {
            const int v = order[i - 1], p = next_[v];
            depth_[v] = depth_[p] + 1; entry_[v] = entry_[p]; cycle_id_[v] = cycle_id_[p]; position_[v] = position_[p];
            ++component_size_[cycle_id_[v]]; max_depth = std::max(max_depth, depth_[v]);
        }
        levels_ = std::bit_width(static_cast<unsigned>(max_depth));
        if (levels_ && n > jump_.max_size() / levels_) throw std::length_error("FunctionalGraph lifting capacity");
        jump_.resize(n * levels_);
        if (levels_) std::copy(next_.begin(), next_.end(), jump_.begin());
        for (unsigned level = 1; level < levels_; ++level) {
            const auto before = n * (level - 1), here = n * level;
            for (std::size_t v = 0; v < n; ++v) jump_[here + v] = jump_[before + static_cast<std::size_t>(jump_[before + v])];
        }
    }
    int jump_tree(int v, std::uint64_t k) const noexcept {
        const auto n = next_.size();
        for (unsigned level = 0; k; ++level, k >>= 1) if (k & 1) v = jump_[n * level + static_cast<std::size_t>(v)];
        return v;
    }
public:
    FunctionalGraph() = default;
    explicit FunctionalGraph(std::span<const int> successors) { initialize(successors); }
    FunctionalGraph(const FunctionalGraph&) = default;
    FunctionalGraph& operator=(const FunctionalGraph& other) { if (this != &other) { FunctionalGraph copy(other); swap(copy); } return *this; }
    FunctionalGraph(FunctionalGraph&& other) noexcept { swap(other); }
    FunctionalGraph& operator=(FunctionalGraph&& other) noexcept { if (this != &other) { clear(); swap(other); } return *this; }
    void swap(FunctionalGraph& other) noexcept {
        next_.swap(other.next_); depth_.swap(other.depth_); entry_.swap(other.entry_); cycle_id_.swap(other.cycle_id_);
        position_.swap(other.position_); component_size_.swap(other.component_size_); cycle_vertices_.swap(other.cycle_vertices_);
        jump_.swap(other.jump_); cycle_offsets_.swap(other.cycle_offsets_); std::swap(levels_, other.levels_);
    }
    friend void swap(FunctionalGraph& a, FunctionalGraph& b) noexcept { a.swap(b); }
    void build(std::span<const int> successors) { FunctionalGraph next(successors); swap(next); }
    int size() const noexcept { return static_cast<int>(next_.size()); }
    bool empty() const noexcept { return next_.empty(); }
    int successor(int v) const { check_vertex(v); return next_[v]; }
    std::span<const int> successors() const noexcept { return next_; }
    int cycle_count() const noexcept { return static_cast<int>(component_size_.size()); }
    bool on_cycle(int v) const { check_vertex(v); return depth_[v] == 0; }
    int component_size(int v) const { check_vertex(v); return component_size_[cycle_id_[v]]; }
    std::span<const int> cycle(int id) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (id < 0 || id >= cycle_count()) throw std::out_of_range("FunctionalGraph cycle");
#endif
        const auto first = cycle_offsets_[id], last = cycle_offsets_[id + 1];
        return std::span<const int>(cycle_vertices_).subspan(first, last - first);
    }
    CycleInfo info(int v) const {
        check_vertex(v); const int id = cycle_id_[v];
        return {id, entry_[v], depth_[v], position_[v], static_cast<int>(cycle_offsets_[id + 1] - cycle_offsets_[id])};
    }
    int kth_successor(int v, std::uint64_t k) const {
        check_vertex(v);
        const auto depth = static_cast<std::uint64_t>(depth_[v]);
        if (k < depth) return jump_tree(v, k);
        const auto id = cycle_id_[v];
        const auto length = cycle_offsets_[id + 1] - cycle_offsets_[id];
        const auto offset = (static_cast<std::size_t>((k - depth) % length) + static_cast<std::size_t>(position_[v])) % length;
        return cycle_vertices_[cycle_offsets_[id] + offset];
    }
    std::int64_t distance(int from, int to) const {
        check_vertex(from); check_vertex(to);
        if (cycle_id_[from] != cycle_id_[to] || depth_[from] < depth_[to]) return -1;
        if (depth_[to]) {
            const auto d = static_cast<std::uint64_t>(depth_[from] - depth_[to]);
            return jump_tree(from, d) == to ? static_cast<std::int64_t>(d) : -1;
        }
        const auto id = cycle_id_[from];
        const auto length = static_cast<std::int64_t>(cycle_offsets_[id + 1] - cycle_offsets_[id]);
        const auto offset = (static_cast<std::int64_t>(position_[to]) - position_[from] + length) % length;
        return static_cast<std::int64_t>(depth_[from]) + offset;
    }
    bool reachable(int from, int to) const { return distance(from, to) >= 0; }
    void clear() noexcept {
        next_.clear(); depth_.clear(); entry_.clear(); cycle_id_.clear(); position_.clear(); component_size_.clear();
        cycle_vertices_.clear(); jump_.clear(); cycle_offsets_.clear(); levels_ = 0;
    }
};
#endif


// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <cstdint>
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
    int n{};
    std::uint64_t k{};
    if (!(verify_in >> n)) return 0;
    verify_in >> k;
    std::vector<int> next(static_cast<std::size_t>(n)), values(static_cast<std::size_t>(n));
    for (auto& v : next) { verify_in >> v; --v; }
    for (auto& value : values) verify_in >> value;
    FunctionalGraph graph(next);
    for (int v = 0; v < n; ++v) {
        if (v) verify_out << ' ';
        verify_out << values[static_cast<std::size_t>(graph.kth_successor(v, k))];
    }
    verify_out << '\n';
}
