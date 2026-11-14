#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/two_edge_connected_components

// Library: Graph/Low_Link.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Low_Link.cpp
// 参考（low-linkの分類）: https://nyaannyaan.github.io/library/graph/lowlink.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/low_link.md
//
// API一覧
// UndirectedLowLink(n=0): n頂点、空の無向多重グラフ。頂点番号[0,n)。準備O(1)。
// add_edge(u,v): 無向辺を追加しsize_tの辺番号を返す。自己辺可。償却O(1)。reserve_edges(m): m辺予約。
// build(collect_blocks=false): 連結成分数を返す。反復DFSでO(n+m)時間・領域。追加後は再build。
// collect_blocks=trueなら点二重連結ブロックも求める。既定ではその領域確保・辺スタック処理を省く。
// size()/edge_count()/edge(id): 頂点数/辺数/端点pair。O(1)。component_count(): 連結成分数。build後O(1)。
// bridges()/articulation_points(): 橋の辺番号/関節点番号の昇順配列へのconst参照。build後O(1)。
// is_bridge(id)/is_articulation(v): 橋/関節点か。build後O(1)。平行辺は異なるIDとして扱う。
// two_edge_component_count(): 2辺連結成分数。build後O(1)。
// two_edge_component(v)/two_edge_components(): 橋を除いた成分番号/頂点群コピー。O(1)/O(n+m)。
// bridge_forest(): 2辺連結成分を頂点とした橋の森Graph。O(n+m)時間・領域。
// biconnected_edge_components(): 点二重連結ブロックの辺ID群へのconst参照。build(true)後O(1)。
// 自己辺は1辺だけの独立ブロックとする。孤立頂点は辺ブロックに含めない。
// biconnected_vertex_components(include_isolated=true): build(true)後、辺ブロックの頂点群と孤立点のコピー。O(n+m)。
// 成分番号・成分群やブロックの順序は規定しない。
// reset(n=0)/swap(other): 空のn頂点へ戻す/O(1)交換。コピーは独立所有、ムーブ元は空の未分解状態。
// 入力検査と結果有効性だけの状態はSKIP_RUNTIME_CHECKSで除く。結果参照は再build等で無効になる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNWEIGHTED_GRAPH_DEFINED
#define SIRKIT_UNWEIGHTED_GRAPH_DEFINED
using Graph = std::vector<std::vector<int>>;
#endif

#ifndef SIRKIT_UNDIRECTED_LOW_LINK_DEFINED
#define SIRKIT_UNDIRECTED_LOW_LINK_DEFINED
class UndirectedLowLink {
    int n_ = 0, components_ = 0;
    std::vector<std::pair<int, int>> edges_;
    std::vector<std::size_t> bridges_;
    std::vector<int> articulation_, two_edge_id_;
    std::vector<unsigned char> bridge_flag_, cut_flag_;
    std::vector<std::vector<std::size_t>> blocks_;
    int two_components_ = 0;
#ifndef SKIP_RUNTIME_CHECKS
    bool built_ = false, blocks_built_ = false;
    void vertex(int v) const { if (v < 0 || v >= n_) throw std::out_of_range("LowLink: vertex out of range"); }
    void edge_id(std::size_t e) const { if (e >= edges_.size()) throw std::out_of_range("LowLink: edge out of range"); }
    void require_build() const { if (!built_) throw std::logic_error("LowLink: call build first"); }
#endif
public:
    explicit UndirectedLowLink(int n = 0) : n_(n) {
        if (n < 0) throw std::length_error("LowLink: negative vertex count");
    }
    UndirectedLowLink(const UndirectedLowLink&) = default;
    UndirectedLowLink(UndirectedLowLink&& other) noexcept { swap(other); }
    UndirectedLowLink& operator=(UndirectedLowLink other) noexcept { swap(other); return *this; }
    void swap(UndirectedLowLink& other) noexcept {
        std::swap(n_, other.n_); std::swap(components_, other.components_); std::swap(two_components_, other.two_components_); edges_.swap(other.edges_);
        bridges_.swap(other.bridges_); articulation_.swap(other.articulation_); two_edge_id_.swap(other.two_edge_id_);
        bridge_flag_.swap(other.bridge_flag_); cut_flag_.swap(other.cut_flag_); blocks_.swap(other.blocks_);
#ifndef SKIP_RUNTIME_CHECKS
        std::swap(built_, other.built_); std::swap(blocks_built_, other.blocks_built_);
#endif
    }
    void reset(int n = 0) { UndirectedLowLink replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t edge_count() const noexcept { return edges_.size(); }
    void reserve_edges(std::size_t m) { edges_.reserve(m); }
    std::size_t add_edge(int u, int v) {
#ifndef SKIP_RUNTIME_CHECKS
        vertex(u); vertex(v);
#endif
        edges_.emplace_back(u, v);
#ifndef SKIP_RUNTIME_CHECKS
        built_ = blocks_built_ = false;
#endif
        return edges_.size() - 1;
    }
    std::pair<int, int> edge(std::size_t id) const {
#ifndef SKIP_RUNTIME_CHECKS
        edge_id(id);
#endif
        return edges_[id];
    }
private:
    template<class Id> struct Adjacent { int to; Id id; };
    template<bool Blocks, class Id>
    int build_impl() {
        const auto m = edges_.size();
        if (m > std::vector<Adjacent<Id>>().max_size() / 2) throw std::length_error("LowLink: too many edges");
        // 接続数2mを表現できる型をCSRの位置にも共有する。
        std::vector<Id> begin(static_cast<std::size_t>(n_) + 1);
        for (auto [u, v] : edges_) { ++begin[u + 1]; ++begin[v + 1]; }
        for (int v = 0; v < n_; ++v) begin[v + 1] += begin[v];
        std::vector<Id> next(begin.begin(), begin.end() - 1);
        std::vector<Adjacent<Id>> incident(m * 2);
        for (std::size_t e = 0; e < m; ++e) { auto [u, v] = edges_[e]; incident[next[u]++] = {v, static_cast<Id>(e)}; incident[next[v]++] = {u, static_cast<Id>(e)}; }
        std::copy_n(begin.begin(), n_, next.begin());
        const auto no_edge = m;
        std::vector<int> entered(n_, -1), low(n_), parent(n_, -1);
        std::vector<Id> parent_edge(n_, static_cast<Id>(no_edge)), pending;
        std::vector<unsigned char> bridge(m), cut(n_);
        std::vector<std::vector<std::size_t>> blocks;
        if constexpr (Blocks) pending.reserve(m);
        int clock = 0, count = 0, two_count = 0;
        for (int root = 0; root < n_; ++root) {
            if (entered[root] != -1) continue;
            int root_children = 0;
            ++count; entered[root] = low[root] = clock++;
            // 親配列を探索中の戻り先にも使う。
            int v = root;
            for (;;) {
                if (next[v] != begin[v + 1]) {
                    const auto [u, edge_id] = incident[next[v]++];
                    if (edge_id == parent_edge[v]) continue;
                    if (u == v) {
                        // 自己辺の2つの接続を、先に現れた側だけで処理する。
                        if constexpr (Blocks) if (next[v] == begin[v] + 1 || incident[next[v] - 2].id != edge_id) blocks.push_back({edge_id});
                        continue;
                    }
                    if (entered[u] == -1) {
                        parent[u] = v; parent_edge[u] = edge_id; root_children += (v == root); if constexpr (Blocks) pending.push_back(edge_id);
                        entered[u] = low[u] = clock++; v = u;
                    } else if (entered[u] < entered[v]) { low[v] = std::min(low[v], entered[u]); if constexpr (Blocks) pending.push_back(edge_id); }
                    continue;
                }
                const int p = parent[v];
                if (p == -1) { cut[v] = root_children > 1; break; }
                low[p] = std::min(low[p], low[v]);
                if (low[v] > entered[p]) bridge[parent_edge[v]] = 1;
                if (low[v] >= entered[p]) {
                    if (parent[p] != -1) cut[p] = 1;
                    if constexpr (Blocks) {
                        if (pending.back() == parent_edge[v]) {
                            blocks.push_back({pending.back()}); pending.pop_back();
                        } else {
                            // 親辺までの接尾辞を、従来と同じ逆順で一度に確保する。
                            auto stop = pending.rbegin();
                            while (*stop != parent_edge[v]) ++stop;
                            ++stop;
                            blocks.emplace_back(pending.rbegin(), stop);
                            pending.resize(pending.size() - static_cast<std::size_t>(stop - pending.rbegin()));
                        }
                    }
                }
                v = p;
            }
        }
        // 探索後は親配列を成分の参照鎖として再利用する。
        auto& component = parent;
        // 橋で区切られる根は負の印、その他は同じ成分の親への参照を置く。
        for (int v = 0; v < n_; ++v)
            component[v] = low[v] == entered[v] ? -(++two_count) : component[v];
        // 全頂点の終了後、親参照を負の印へ圧縮する。ここでは正のIDへ変換しない。
        if (two_count != n_) {
            for (int v = 0; v < n_; ++v) {
                if (component[v] < 0) continue;
                int root = v;
                while (component[root] >= 0) root = component[root];
                const int mark = component[root];
                int u = v;
                while (component[u] >= 0) {
                    const int next_vertex = component[u];
                    component[u] = mark;
                    u = next_vertex;
                }
            }
        }
        for (auto& id : component) id = -1 - id;
        std::vector<std::size_t> bridges; std::vector<int> points;
        for (std::size_t e = 0; e < m; ++e) if (bridge[e]) bridges.push_back(e);
        for (int v = 0; v < n_; ++v) if (cut[v]) points.push_back(v);
        bridges_.swap(bridges); articulation_.swap(points); two_edge_id_.swap(component);
        bridge_flag_.swap(bridge); cut_flag_.swap(cut); blocks_.swap(blocks); components_ = count; two_components_ = two_count;
#ifndef SKIP_RUNTIME_CHECKS
        built_ = true; blocks_built_ = Blocks;
#endif
        return count;
    }
public:
    int build(bool collect_blocks = false) {
        if constexpr (sizeof(std::size_t) > sizeof(std::uint32_t)) {
            if (edges_.size() <= std::numeric_limits<std::uint32_t>::max() / 2)
                return collect_blocks ? build_impl<true, std::uint32_t>() : build_impl<false, std::uint32_t>();
        }
        return collect_blocks ? build_impl<true, std::size_t>() : build_impl<false, std::size_t>();
    }
    int component_count() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return components_;
    }
    const std::vector<std::size_t>& bridges() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return bridges_;
    }
    const std::vector<int>& articulation_points() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return articulation_;
    }
    bool is_bridge(std::size_t e) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); edge_id(e);
#endif
        return bridge_flag_[e] != 0;
    }
    bool is_articulation(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); vertex(v);
#endif
        return cut_flag_[v] != 0;
    }
    int two_edge_component_count() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        return two_components_;
    }
    int two_edge_component(int v) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); vertex(v);
#endif
        return two_edge_id_[v];
    }
    std::vector<std::vector<int>> two_edge_components() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        const int count = two_components_;
        std::vector<std::vector<int>> result(count);
        // 全頂点が1成分なら頂点番号を直接並べる。
        if (count == 1) {
            auto& vertices = result.front();
            vertices.resize(n_);
            for (int v = 0; v < n_; ++v) vertices[v] = v;
            return result;
        }
        for (int v = 0; v < n_; ++v) result[two_edge_id_[v]].push_back(v);
        return result;
    }
    Graph bridge_forest() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build();
#endif
        const int count = two_components_;
        Graph result(count);
        for (auto id : bridges_) { auto [u, v] = edges_[id]; const int a = two_edge_id_[u], b = two_edge_id_[v]; result[a].push_back(b); result[b].push_back(a); }
        return result;
    }
    const std::vector<std::vector<std::size_t>>& biconnected_edge_components() const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); if (!blocks_built_) throw std::logic_error("LowLink: call build(true) for biconnected blocks");
#endif
        return blocks_;
    }
    std::vector<std::vector<int>> biconnected_vertex_components(bool include_isolated = true) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_build(); if (!blocks_built_) throw std::logic_error("LowLink: call build(true) for biconnected blocks");
#endif
        auto collect = [&]<class Stamp>() {
            std::vector<std::vector<int>> result; result.reserve(blocks_.size());
            std::vector<Stamp> last(n_, static_cast<Stamp>(blocks_.size()));
            for (std::size_t i = 0; i < blocks_.size(); ++i) {
                result.emplace_back();
                result.back().reserve(std::min(static_cast<std::size_t>(n_), blocks_[i].size() + 1));
                for (auto e : blocks_[i]) { const auto [u, v] = edges_[e]; for (int vertex : {u, v}) {
                    if (last[vertex] != i) { last[vertex] = static_cast<Stamp>(i); result.back().push_back(vertex); }
                } }
            }
            if (include_isolated) for (int v = 0; v < n_; ++v) if (last[v] == blocks_.size()) result.push_back({v});
            return result;
        };
        if constexpr (sizeof(std::size_t) > sizeof(std::uint32_t)) {
            if (blocks_.size() <= std::numeric_limits<std::uint32_t>::max())
                return collect.template operator()<std::uint32_t>();
        }
        return collect.template operator()<std::size_t>();
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

#include <vector>
int main(){
 int n{},m{};verify_in>>n>>m;UndirectedLowLink graph(n);
 for(int i=0;i<m;++i){int u{},v{};verify_in>>u>>v;graph.add_edge(u,v);}graph.build();
 const int count=graph.two_edge_component_count();
 // 成分が1個なら全頂点を直接並べる。
 if(count==1){verify_out<<1<<'\n'<<n;for(int v=0;v<n;++v)verify_out<<' '<<v;verify_out<<'\n';return 0;}
 std::vector<int> first(count+2),members(n);
 for(int v=0;v<n;++v)++first[graph.two_edge_component(v)+2];
 for(int c=2;c<count+2;++c)first[c]+=first[c-1];
 for(int v=0;v<n;++v)members[first[graph.two_edge_component(v)+1]++]=v;
 verify_out<<count<<'\n';
 for(int c=0;c<count;++c){verify_out<<first[c+1]-first[c];for(int i=first[c];i<first[c+1];++i)verify_out<<' '<<members[i];verify_out<<'\n';}
}
