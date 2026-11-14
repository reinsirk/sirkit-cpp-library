#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/eulerian_trail_directed

// Library: Graph/Eulerian_Trail.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Eulerian_Trail.cpp
// 参考（オイラー路の条件）: https://ei1333.github.io/library/graph/others/eulerian-trail.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/eulerian_trail.md
//
// API一覧
// EulerianTrail<Directed=false>(n=0): n頂点。有向/無向を型で選ぶ。自己辺・多重辺可。準備O(1)。
// DirectedEulerianTrail/UndirectedEulerianTrail: 上記の有向/無向の別名。
// add_edge(u,v): 辺を追加しsize_tのIDを返す。償却O(1)。reserve_edges(m): m辺予約。
// size()/edge_count()/edge(id): 頂点数/辺数/追加時の端点pair。O(1)。
// find(start=-1,circuit_only=false): 全辺を一度ずつ通るTrailのoptional。存在しなければnullopt。O(n+m)時間・領域。
// Trail.verticesは通過頂点列、Trail.edgesは対応する辺ID列。circuit()は閉路か。無向辺の向きは頂点列で分かる。
// start=-1は自動選択。指定開始点が次数条件に合わない場合もnullopt。circuit_only=trueは閉路だけを許す。
// 0辺なら指定開始点（省略時0）だけを返す。0頂点0辺なら空列のTrailを返す。
// 辺を持つ成分が複数ならnullopt。孤立頂点は路に含めない。findはグラフを変更せず繰り返し呼べる。
// reset(n=0)/swap(other): 空のn頂点へ/O(1)交換。コピーは独立所有、ムーブ元は空。
// 頂点・辺IDの入力検査はSKIP_RUNTIME_CHECKSで除く。次数条件と全辺の消費判定は維持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_EULERIAN_TRAIL_DEFINED
#define SIRKIT_EULERIAN_TRAIL_DEFINED
template<bool Directed = false>
class EulerianTrail {
public:
    struct Trail {
        std::vector<int> vertices;
        std::vector<std::size_t> edges;
        bool circuit() const noexcept { return vertices.empty() || vertices.front() == vertices.back(); }
    };
private:
    int n_ = 0;
    std::vector<std::pair<int, int>> edges_;
public:
    explicit EulerianTrail(int n = 0) : n_(n) {
        if (n < 0) throw std::length_error("EulerianTrail: negative vertex count");
    }
    EulerianTrail(const EulerianTrail&) = default;
    EulerianTrail(EulerianTrail&& other) noexcept { swap(other); }
    EulerianTrail& operator=(EulerianTrail other) noexcept { swap(other); return *this; }
    void swap(EulerianTrail& other) noexcept { std::swap(n_, other.n_); edges_.swap(other.edges_); }
    void reset(int n = 0) { EulerianTrail replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    void reserve_edges(std::size_t count) { edges_.reserve(count); }
    std::size_t add_edge(int u, int v) {
        edges_.emplace_back(u, v); return edges_.size() - 1;
    }
    std::pair<int, int> edge(std::size_t id) const {
        return edges_[id];
    }
    std::optional<Trail> find(int start = -1, bool circuit_only = false) const {
        const auto m = edges_.size();
        if (m == 0) {
            Trail result;
            if (n_ != 0) result.vertices.push_back(start == -1 ? 0 : start);
            return result;
        }
        constexpr std::size_t copies = Directed ? 1 : 2;
        if (m > std::vector<std::size_t>().max_size() / copies)
            throw std::length_error("EulerianTrail: too many edges");
        std::vector<std::size_t> degree(n_), incoming;
        if constexpr (Directed) incoming.resize(n_);
        for (auto [u, v] : edges_) {
            ++degree[u];
            if constexpr (Directed) ++incoming[v]; else ++degree[v];
        }
        int preferred = -1, second = -1, any = -1;
        for (int v = 0; v < n_; ++v) {
            if (degree[v] != 0 && any == -1) any = v;
            if constexpr (Directed) {
                if (degree[v] == incoming[v]) continue;
                if (circuit_only) return std::nullopt;
                if (degree[v] > incoming[v] && degree[v] - incoming[v] == 1) {
                    if (preferred != -1) return std::nullopt;
                    preferred = v;
                } else if (incoming[v] > degree[v] && incoming[v] - degree[v] == 1) {
                    if (second != -1) return std::nullopt;
                    second = v;
                } else return std::nullopt;
            } else if ((degree[v] & 1) != 0) {
                if (circuit_only) return std::nullopt;
                if (preferred == -1) preferred = v;
                else if (second == -1) second = v;
                else return std::nullopt;
            }
        }
        if ((preferred == -1) != (second == -1)) return std::nullopt;
        if (start == -1) start = preferred == -1 ? any : preferred;
        else if constexpr (Directed) {
            if ((preferred != -1 && start != preferred) || degree[start] == 0) return std::nullopt;
        } else if ((preferred != -1 && start != preferred && start != second) || degree[start] == 0) return std::nullopt;
        std::vector<std::size_t> begin(static_cast<std::size_t>(n_) + 1);
        for (int v = 0; v < n_; ++v) begin[v + 1] = begin[v] + degree[v];
        std::vector<std::size_t> cursor(begin.begin(), begin.end() - 1), incident(m * copies);
        for (std::size_t id = 0; id < m; ++id) {
            const auto [u, v] = edges_[id]; incident[cursor[u]++] = id;
            if constexpr (!Directed) incident[cursor[v]++] = id;
        }
        std::copy_n(begin.begin(), n_, cursor.begin());
        std::vector<unsigned char> used;
        if constexpr (!Directed) used.resize(m);
        std::vector<int> vertices; std::vector<std::size_t> edge_stack;
        vertices.reserve(m + 1); edge_stack.reserve(m); vertices.push_back(start);
        Trail result; result.vertices.reserve(m + 1); result.edges.reserve(m);
        while (!vertices.empty()) {
            const int v = vertices.back();
            if constexpr (!Directed) while (cursor[v] != begin[v + 1] && used[incident[cursor[v]]]) ++cursor[v];
            if (cursor[v] == begin[v + 1]) {
                result.vertices.push_back(v); vertices.pop_back();
                if (!edge_stack.empty()) { result.edges.push_back(edge_stack.back()); edge_stack.pop_back(); }
                continue;
            }
            const auto id = incident[cursor[v]++]; const auto [a, b] = edges_[id];
            if constexpr (!Directed) used[id] = 1;
            vertices.push_back(Directed ? b : (a == v ? b : a)); edge_stack.push_back(id);
        }
        if (result.edges.size() != m) return std::nullopt;
        std::reverse(result.vertices.begin(), result.vertices.end()); std::reverse(result.edges.begin(), result.edges.end());
        return result;
    }
};
using DirectedEulerianTrail = EulerianTrail<true>;
using UndirectedEulerianTrail = EulerianTrail<false>;
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

int main(){
 int cases{};verify_in>>cases;while(cases--){int n{},m{};verify_in>>n>>m;DirectedEulerianTrail graph(n);
 for(int i=0;i<m;++i){int u{},v{};verify_in>>u>>v;graph.add_edge(u,v);}auto path=graph.find();if(!path){verify_out<<"No\n";continue;}
 verify_out<<"Yes\n";for(std::size_t i=0;i<path->vertices.size();++i)verify_out<<(i?" ":"")<<path->vertices[i];verify_out<<'\n';for(std::size_t i=0;i<path->edges.size();++i)verify_out<<(i?" ":"")<<path->edges[i];verify_out<<'\n';}
}
