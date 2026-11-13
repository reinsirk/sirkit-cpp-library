#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/general_matching

// Library: Graph/General_Matching.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/General_Matching.cpp
// 数学的参考: https://doi.org/10.4153/CJM-1965-045-4
// 参考: https://ei1333.github.io/library/graph/flow/gabow-edmonds.hpp.html
// 奇数閉路の縮約という数学的構成を参考に、単純な反復Edmonds探索を独立実装。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/general_matching.md
// API一覧
// GeneralMatching(n=0): 頂点0..n-1の無向グラフ。n>=0、構築O(n)。empty()は頂点数0の判定。
// add_edge(u,v): 無向辺を追加、自己loopは無視、多重辺可。償却O(1)。対応は保ち最大性を無効化。
// reserve_edges(cap): 辺容量を予約、再確保時O(m)。edge_count()は自己loopを除いた追加件数。
// edges(): 追加順のconst span<pair<int32_t,int32_t>>。重複・端点の入力順も保持する。
// solve(): 現在の対応を最大マッチングへ拡張して総本数を返す。再帰なし。最大性既知ならO(1)。
//   n頂点m辺としてO(n^3+nm)時間・追加O(n+m)領域。単純グラフではO(n^3)。
// size()/empty()/matching_size()/is_maximum()/is_perfect(): O(1)。空グラフはperfect。
// mate(v): 対応先のint32_t、未対応は-1。partners(): 全対応のconst span。各O(1)。
// pairs(): u<vの対応辺をuの昇順で返す。O(n)時間・O(matching_size())領域。
// solve前/辺追加後の対応も有効だが、is_maximum()==falseなら最大とは限らない。
// clear_matching(): 辺を保ち対応を消す、O(n)。clear_edges(): 頂点数を保ち辺/対応を消す、O(n+m)。
// reset(n=0)/clear(): 指定頂点数/0頂点に初期化。copyはO(n+m)で独立、move元は0頂点、swapはO(1)。
// copyはO(n+m)。reset/clearは旧データ破棄と新頂点初期化に比例。
// add_edge/reset/copy代入は内容に強い例外保証。solveは全作業領域確保後のみ対応を変更する。
// partners viewはreset/clear/代入/所有権移動まで有効。edges viewは追加/reserve/clear_edgesでも無効化し得る。
// 頂点番号/負数の検査はSKIP_RUNTIME_CHECKSで除外。容量・n+1/2mの添字保護は常に維持する。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_GENERAL_MATCHING_WORKSPACE_DEFINED
#define SIRKIT_GENERAL_MATCHING_WORKSPACE_DEFINED
namespace reinsirk::detail {
struct GeneralMatchingWorkspace {
    std::vector<std::size_t> offsets, cursor;
    std::vector<std::int32_t> neighbors, parent, base, queue;
    std::vector<std::uint8_t> outer, ancestor, contracted;
    GeneralMatchingWorkspace(std::size_t n, std::size_t arcs)
        : offsets(n + 1), cursor(n), neighbors(arcs), parent(n), base(n), queue(n),
          outer(n), ancestor(n), contracted(n) {}
};
}
#endif

#ifndef SIRKIT_GENERAL_MATCHING_DEFINED
#define SIRKIT_GENERAL_MATCHING_DEFINED
class GeneralMatching {
    std::vector<std::pair<std::int32_t, std::int32_t>> edges_;
    std::vector<std::int32_t> partners_;
    std::int32_t cardinality_ = 0;
    bool maximum_ = true;
    void check_vertex(std::int32_t vertex) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= partners_.size())
            throw std::out_of_range("GeneralMatching vertex");
#else
        (void)vertex;
#endif
    }
    bool augment(std::int32_t root, reinsirk::detail::GeneralMatchingWorkspace& work) noexcept {
        std::fill(work.parent.begin(), work.parent.end(), -1);
        std::iota(work.base.begin(), work.base.end(), std::int32_t{0});
        std::fill(work.outer.begin(), work.outer.end(), std::uint8_t{0});
        std::size_t head = 0, tail = 0;
        work.outer[static_cast<std::size_t>(root)] = 1; work.queue[tail++] = root;
        auto join = [&](std::int32_t a, std::int32_t b) {
            std::fill(work.ancestor.begin(), work.ancestor.end(), std::uint8_t{0});
            for (;;) {
                a = work.base[static_cast<std::size_t>(a)];
                work.ancestor[static_cast<std::size_t>(a)] = 1;
                const auto mate = partners_[static_cast<std::size_t>(a)];
                if (mate == -1) break;
                a = work.parent[static_cast<std::size_t>(mate)];
            }
            for (;;) {
                b = work.base[static_cast<std::size_t>(b)];
                if (work.ancestor[static_cast<std::size_t>(b)]) return b;
                b = work.parent[static_cast<std::size_t>(partners_[static_cast<std::size_t>(b)])];
            }
        };
        auto mark = [&](std::int32_t vertex, std::int32_t common, std::int32_t child) {
            while (work.base[static_cast<std::size_t>(vertex)] != common) {
                const auto mate = partners_[static_cast<std::size_t>(vertex)];
                work.contracted[static_cast<std::size_t>(work.base[static_cast<std::size_t>(vertex)])] = 1;
                work.contracted[static_cast<std::size_t>(work.base[static_cast<std::size_t>(mate)])] = 1;
                work.parent[static_cast<std::size_t>(vertex)] = child;
                child = mate; vertex = work.parent[static_cast<std::size_t>(mate)];
            }
        };
        while (head < tail) {
            const auto u = work.queue[head++]; const auto ui = static_cast<std::size_t>(u);
            for (auto edge = work.offsets[ui]; edge < work.offsets[ui + 1]; ++edge) {
                const auto v = work.neighbors[edge]; const auto vi = static_cast<std::size_t>(v);
                if (work.base[ui] == work.base[vi] || partners_[ui] == v) continue;
                if (work.outer[vi]) {
                    const auto common = join(u, v);
                    std::fill(work.contracted.begin(), work.contracted.end(), std::uint8_t{0});
                    mark(u, common, v); mark(v, common, u);
                    for (std::size_t i = 0; i < partners_.size(); ++i) {
                        if (!work.contracted[static_cast<std::size_t>(work.base[i])]) continue;
                        work.base[i] = common;
                        if (!work.outer[i]) { work.outer[i] = 1; work.queue[tail++] = static_cast<std::int32_t>(i); }
                    }
                } else if (work.parent[vi] == -1) {
                    work.parent[vi] = u;
                    if (partners_[vi] == -1) {
                        // 縮約時に付け替えた親をたどり、増加路の対応を反転する。
                        auto vertex = v;
                        while (vertex != -1) {
                            const auto parent = work.parent[static_cast<std::size_t>(vertex)];
                            const auto next = parent == -1 ? -1 : partners_[static_cast<std::size_t>(parent)];
                            partners_[static_cast<std::size_t>(vertex)] = parent;
                            if (parent != -1) partners_[static_cast<std::size_t>(parent)] = vertex;
                            vertex = next;
                        }
                        ++cardinality_; return true;
                    }
                    const auto mate = partners_[vi];
                    work.outer[static_cast<std::size_t>(mate)] = 1; work.queue[tail++] = mate;
                }
            }
        }
        return false;
    }
public:
    explicit GeneralMatching(std::int32_t n = 0) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("GeneralMatching size");
#endif
        partners_.assign(static_cast<std::size_t>(n), -1);
    }
    GeneralMatching(const GeneralMatching&) = default;
    GeneralMatching& operator=(const GeneralMatching& other) {
        if (this != &other) { GeneralMatching copy(other); swap(copy); }
        return *this;
    }
    GeneralMatching(GeneralMatching&& other) noexcept { swap(other); }
    GeneralMatching& operator=(GeneralMatching&& other) noexcept {
        if (this != &other) { GeneralMatching moved(std::move(other)); swap(moved); }
        return *this;
    }
    void swap(GeneralMatching& other) noexcept {
        edges_.swap(other.edges_); partners_.swap(other.partners_);
        std::swap(cardinality_, other.cardinality_); std::swap(maximum_, other.maximum_);
    }
    friend void swap(GeneralMatching& a, GeneralMatching& b) noexcept { a.swap(b); }
    std::int32_t size() const noexcept { return static_cast<std::int32_t>(partners_.size()); }
    bool empty() const noexcept { return partners_.empty(); }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    std::int32_t matching_size() const noexcept { return cardinality_; }
    bool is_maximum() const noexcept { return maximum_; }
    bool is_perfect() const noexcept { return static_cast<std::size_t>(cardinality_) * 2 == partners_.size(); }
    std::int32_t mate(std::int32_t vertex) const { check_vertex(vertex); return partners_[static_cast<std::size_t>(vertex)]; }
    std::span<const std::int32_t> partners() const noexcept { return partners_; }
    std::span<const std::pair<std::int32_t, std::int32_t>> edges() const noexcept { return edges_; }
    void reserve_edges(std::size_t capacity) { edges_.reserve(capacity); }
    void add_edge(std::int32_t u, std::int32_t v) {
        check_vertex(u); check_vertex(v); if (u == v) return;
        edges_.emplace_back(u, v); maximum_ = false;
    }
    std::int32_t solve() {
        if (maximum_) return cardinality_;
        if (is_perfect()) { maximum_ = true; return cardinality_; }
        const auto n = partners_.size(), m = edges_.size();
        if (n == std::numeric_limits<std::size_t>::max() ||
            m > std::vector<std::int32_t>{}.max_size() / 2)
            throw std::length_error("GeneralMatching workspace capacity");
        reinsirk::detail::GeneralMatchingWorkspace work(n, m * 2);
        for (const auto& [u,v] : edges_) { ++work.offsets[static_cast<std::size_t>(u) + 1]; ++work.offsets[static_cast<std::size_t>(v) + 1]; }
        for (std::size_t i = 0; i < n; ++i) { work.offsets[i + 1] += work.offsets[i]; work.cursor[i] = work.offsets[i]; }
        for (const auto& [u,v] : edges_) {
            work.neighbors[work.cursor[static_cast<std::size_t>(u)]++] = v;
            work.neighbors[work.cursor[static_cast<std::size_t>(v)]++] = u;
        }
        // 空いている端点同士を先に対応させ、探索開始点を減らす。
        for (std::size_t u = 0; u < n; ++u) if (partners_[u] == -1) {
            for (auto edge = work.offsets[u]; edge < work.offsets[u + 1]; ++edge) {
                const auto v = work.neighbors[edge];
                if (partners_[static_cast<std::size_t>(v)] != -1) continue;
                partners_[u] = v; partners_[static_cast<std::size_t>(v)] = static_cast<std::int32_t>(u);
                ++cardinality_; break;
            }
        }
        for (std::int32_t root = 0; root < size(); ++root) {
            if (partners_[static_cast<std::size_t>(root)] == -1) augment(root, work);
        }
        maximum_ = true; return cardinality_;
    }
    std::vector<std::pair<std::int32_t, std::int32_t>> pairs() const {
        std::vector<std::pair<std::int32_t, std::int32_t>> result;
        result.reserve(static_cast<std::size_t>(cardinality_));
        for (std::int32_t u = 0; u < size(); ++u) if (u < partners_[static_cast<std::size_t>(u)]) result.emplace_back(u, partners_[static_cast<std::size_t>(u)]);
        return result;
    }
    void clear_matching() noexcept {
        std::fill(partners_.begin(), partners_.end(), -1); cardinality_ = 0; maximum_ = edges_.empty();
    }
    void clear_edges() noexcept { edges_.clear(); clear_matching(); }
    void reset(std::int32_t n = 0) { GeneralMatching next(n); swap(next); }
    void clear() { reset(); }
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
int main() {
    std::int32_t n{},m{}; verify_in >> n >> m;
    GeneralMatching graph(n);
    for (std::int32_t i=0; i<m; ++i) {
        std::int32_t u{},v{}; verify_in >> u >> v; graph.add_edge(u,v);
    }
    verify_out << graph.solve() << '\n';
    for (const auto& [u,v] : graph.pairs()) verify_out << u << ' ' << v << '\n';
}
