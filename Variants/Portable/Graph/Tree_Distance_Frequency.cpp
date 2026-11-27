// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Tree_Distance_Frequency.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_distance_frequency.md
//
// API一覧
// tree_distance_frequency(graph): 長さnのvector<int64_t>を返す。d番目は距離dの異なる非順序頂点対の個数。
// graphは0始まりの非重み単純無向木。距離は辺数、添字0は常に0、空入力は空の結果を返す。
// 頂点数は2^23以下。重心分解と整数畳み込みでO(n log²(n+1))時間・O(n)領域。
//
// [同梱重心分解]
// Graph: vector<vector<int>>。頂点0..n-1の単純無向木。n=0も扱う。入力は変更・保持しない。
// CentroidDecomposition()/CentroidDecomposition(graph,store_distances=true): 空/重心分解を構築。
// build(graph,store_distances=true): 再構築、失敗時は元の結果を維持。時間O(n log(n+1))。
// 距離を保存する場合は領域O(n log(n+1))、省く場合はO(n)。全探索を反復で行う。
// size()/empty()/root()/stores_distances(): 頂点数/空か/重心木の根（空なら-1）/距離保存の有無、O(1)。
// parent(v)/depth(v)/component_size(v): 重心木の親（根は-1）/深さ/除去時の成分サイズ、O(1)。
// parents()/depths()/order(): 親配列/深さ配列/親が先に現れる重心列へのconst参照、O(1)。
// Ancestor{int vertex; int distance;}: 祖先重心と元の木における辺数距離。
// ancestors(v): 重心木の根からvまでのAncestor列へのconst参照、O(1)。距離保存時だけ使う。
// distance(u,v): 元の木の距離をint64_tで返す、O(log(n+1))。距離保存が前提。
// centroid_tree(): 重心木の無向隣接リストを所有値で返す、O(n)。
// clear()/swap(other): 空にする/交換する。clearは格納要素数に比例、swapはO(1)。
// コピーは独立。move元は空。build/clear/代入により、以前の配列参照は無効となる。
// SKIP_RUNTIME_CHECKSで頂点・距離保存・初回の探索に組み込んだ木の入力検査を除く。
// 入力検査のためだけの全辺走査はしない。頂点数の表現上限は省略しない。
//
// [同梱NTT]
// NTT<Mod=998244353,Root=3>: Mint不要の素数法NTT。3<=Mod<2^31、Rootは最大2冪位数の根を生成する値。
// NTT(): 動的キャッシュなしで生成。コピーはキャッシュを独立に所有し、時間・領域は保持数に比例。
// modulus()/max_length(): 法/対応する最大変換長をO(1)で返す。
// prepare(length): 必要な根を保持。lengthは0または2冪、上限は2^v2(Mod-1)。新規分O(length)時間・保持O(length)。
// transform(a,inverse=false): 正順の係数と正順の周波数を相互変換しvoidを返す。aはuint32_tのvector。
// transformは空または対応する2冪長、各値0..Mod-1が前提。O(n log n)時間、根以外の追加領域O(1)。
// convolution(a,b): 整数vectorを法Modへ正規化し畳み込みを返す。負数/uint64_t最大値も可。空を含めば{}。
// convolution_normalized(a,b): uint32_tの正規化済みvectorを値で受け取り再利用。std::moveで入力領域を譲渡可。
// square(a): 同じ列の畳み込み。大規模では順変換を1回にする。返却値はvector<uint32_t>。
// 畳み込みは短い方32要素以下ならO(nm)、それ以外はL=bit_ceil(n+m-1)としてO(L log L)、追加O(L)。
// NTT経路のLとtransform長は上限以下。長さ・容量の上限検査はSKIP_RUNTIME_CHECKSでも維持。
// ConvolutionMod<Mod,Root>(a,b): 一時NTTを使う簡易版。同じ長さを繰り返すならNTTオブジェクトを再利用する。
// roots_capacity()/clear_cache(): 動的な根のキャッシュの要素数を返す/解放。通常操作は入力不変、transformは例外までの変更が残り得る。
// Mod<2^30では標準C++のMontgomery演算を使用。GCC/Clangの対応x86 CPUではAVX2で8係数ずつ処理する。
// SIRKIT_DISABLE_SIMDで追加の専用命令とCPU判定を除外できる。代替経路も同じAPI・結果を保つ。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <span>

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

#ifndef SIRKIT_CENTROID_DECOMPOSITION_DEFINED
#define SIRKIT_CENTROID_DECOMPOSITION_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void CheckCentroidVertexCount(std::size_t vertices) {
    if (vertices > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("CentroidDecomposition vertex count");
}
}
class CentroidDecomposition {
public:
    struct Ancestor {
        int vertex;
        int distance;
        friend bool operator==(const Ancestor&, const Ancestor&) = default;
    };
private:
    int root_ = -1;
    bool stores_distances_ = false;
    std::vector<int> parent_, depth_, component_size_, order_;
    std::vector<std::vector<Ancestor>> ancestors_;

    void check_vertex(int vertex) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || static_cast<std::size_t>(vertex) >= size())
            throw std::out_of_range("CentroidDecomposition vertex");
#else
        static_cast<void>(vertex);
#endif
    }
    void check_distances() const {
#ifndef SKIP_RUNTIME_CHECKS
        if (!stores_distances_) throw std::logic_error("CentroidDecomposition distances not stored");
#endif
    }
    void initialize(const Graph& graph, bool store_distances) {
        const auto n = graph.size();
        /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckCentroidVertexCount(n);
        stores_distances_ = store_distances;
        parent_.assign(n, -1); depth_.resize(n); component_size_.resize(n); order_.reserve(n);
        if (store_distances) ancestors_.resize(n);
        if (n == 0) return;
        std::vector<unsigned char> removed(n);
        std::vector<int> traversal_parent(n, -2), subtree(n), component;
        std::vector<std::pair<int,int>> pending{{0,-1}};
        struct Visit { int vertex, parent, distance; };
        std::vector<Visit> visits;
        component.reserve(n);
        while (!pending.empty()) {
            const auto [start, above] = pending.back(); pending.pop_back();
#ifndef SKIP_RUNTIME_CHECKS
            const bool validate = above == -1;
#endif
            component.clear(); component.push_back(start);
            traversal_parent[static_cast<std::size_t>(start)] = -1;
            for (std::size_t i = 0; i < component.size(); ++i) {
                const int vertex = component[i];
                subtree[static_cast<std::size_t>(vertex)] = 1;
#ifndef SKIP_RUNTIME_CHECKS
                std::size_t parent_edges = 0;
#endif
                for (const int to : graph[static_cast<std::size_t>(vertex)]) {
#ifndef SKIP_RUNTIME_CHECKS
                    if (validate && (to < 0 || static_cast<std::size_t>(to) >= n || to == vertex))
                        throw std::invalid_argument("CentroidDecomposition tree edge");
#endif
                    if (to == traversal_parent[static_cast<std::size_t>(vertex)]) {
#ifndef SKIP_RUNTIME_CHECKS
                        if (validate) ++parent_edges;
#endif
                        continue;
                    }
                    if (removed[static_cast<std::size_t>(to)]) continue;
#ifndef SKIP_RUNTIME_CHECKS
                    if (validate && traversal_parent[static_cast<std::size_t>(to)] != -2)
                        throw std::invalid_argument("CentroidDecomposition input is not a tree");
#endif
                    traversal_parent[static_cast<std::size_t>(to)] = vertex;
                    component.push_back(to);
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (validate && vertex != start && parent_edges != 1)
                    throw std::invalid_argument("CentroidDecomposition asymmetric tree");
#endif
            }
#ifndef SKIP_RUNTIME_CHECKS
            if (validate && component.size() != n)
                throw std::invalid_argument("CentroidDecomposition disconnected tree");
#endif
            for (auto it = component.rbegin(); it != component.rend(); ++it) {
                const int above_vertex = traversal_parent[static_cast<std::size_t>(*it)];
                if (above_vertex != -1)
                    subtree[static_cast<std::size_t>(above_vertex)] += subtree[static_cast<std::size_t>(*it)];
            }
            const int count = static_cast<int>(component.size());
            int centroid = start;
            for (;;) {
                int next = -1;
                for (const int to : graph[static_cast<std::size_t>(centroid)]) {
                    if (!removed[static_cast<std::size_t>(to)] &&
                        traversal_parent[static_cast<std::size_t>(to)] == centroid &&
                        subtree[static_cast<std::size_t>(to)] > count / 2) { next = to; break; }
                }
                if (next == -1) break;
                centroid = next;
            }
            parent_[static_cast<std::size_t>(centroid)] = above;
            depth_[static_cast<std::size_t>(centroid)] = above == -1 ? 0 : depth_[static_cast<std::size_t>(above)] + 1;
            component_size_[static_cast<std::size_t>(centroid)] = count;
            order_.push_back(centroid);
            if (above == -1) root_ = centroid;
            if (store_distances) {
                visits.clear(); visits.push_back({centroid, -1, 0});
                for (std::size_t i = 0; i < visits.size(); ++i) {
                    const auto [vertex, from, distance] = visits[i];
                    ancestors_[static_cast<std::size_t>(vertex)].push_back({centroid, distance});
                    for (const int to : graph[static_cast<std::size_t>(vertex)])
                        if (to != from && !removed[static_cast<std::size_t>(to)])
                            visits.push_back({to, vertex, distance + 1});
                }
            }
            removed[static_cast<std::size_t>(centroid)] = 1;
            for (const int to : graph[static_cast<std::size_t>(centroid)])
                if (!removed[static_cast<std::size_t>(to)]) pending.emplace_back(to, centroid);
        }
    }
public:
    CentroidDecomposition() = default;
    explicit CentroidDecomposition(const Graph& graph, bool store_distances = true) { initialize(graph, store_distances); }
    CentroidDecomposition(const CentroidDecomposition&) = default;
    CentroidDecomposition& operator=(const CentroidDecomposition& other) {
        if (this != &other) { CentroidDecomposition next(other); swap(next); }
        return *this;
    }
    CentroidDecomposition(CentroidDecomposition&& other) noexcept { swap(other); }
    CentroidDecomposition& operator=(CentroidDecomposition&& other) noexcept {
        CentroidDecomposition next(std::move(other)); swap(next); return *this;
    }
    void swap(CentroidDecomposition& other) noexcept {
        std::swap(root_, other.root_); std::swap(stores_distances_, other.stores_distances_);
        parent_.swap(other.parent_); depth_.swap(other.depth_); component_size_.swap(other.component_size_);
        order_.swap(other.order_); ancestors_.swap(other.ancestors_);
    }
    friend void swap(CentroidDecomposition& a, CentroidDecomposition& b) noexcept { a.swap(b); }
    void build(const Graph& graph, bool store_distances = true) {
        CentroidDecomposition next(graph, store_distances); swap(next);
    }
    void clear() noexcept {
        root_ = -1; stores_distances_ = false;
        parent_.clear(); depth_.clear(); component_size_.clear(); order_.clear(); ancestors_.clear();
    }
    std::size_t size() const noexcept { return parent_.size(); }
    bool empty() const noexcept { return parent_.empty(); }
    int root() const noexcept { return root_; }
    bool stores_distances() const noexcept { return stores_distances_; }
    int parent(int vertex) const { check_vertex(vertex); return parent_[static_cast<std::size_t>(vertex)]; }
    int depth(int vertex) const { check_vertex(vertex); return depth_[static_cast<std::size_t>(vertex)]; }
    int component_size(int vertex) const { check_vertex(vertex); return component_size_[static_cast<std::size_t>(vertex)]; }
    const std::vector<int>& parents() const noexcept { return parent_; }
    const std::vector<int>& depths() const noexcept { return depth_; }
    const std::vector<int>& order() const noexcept { return order_; }
    const std::vector<Ancestor>& ancestors(int vertex) const {
        check_vertex(vertex); check_distances(); return ancestors_[static_cast<std::size_t>(vertex)];
    }
    std::int64_t distance(int u, int v) const {
        const auto& a = ancestors(u); const auto& b = ancestors(v);
        std::size_t i = 0;
        while (i < a.size() && i < b.size() && a[i].vertex == b[i].vertex) ++i;
        return static_cast<std::int64_t>(a[i - 1].distance) + b[i - 1].distance;
    }
    Graph centroid_tree() const {
        Graph result(size());
        for (std::size_t vertex = 0; vertex < size(); ++vertex) {
            const int above = parent_[vertex];
            if (above != -1) {
                result[vertex].push_back(above);
                result[static_cast<std::size_t>(above)].push_back(static_cast<int>(vertex));
            }
        }
        return result;
    }
};
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <algorithm>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <array>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <bit>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <concepts>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstddef>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <cstdint>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <limits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <stdexcept>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <type_traits>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
#include <immintrin.h>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#endif

#ifndef SIRKIT_NTT_DETAIL_DEFINED
#define SIRKIT_NTT_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
consteval bool ntt_prime(std::uint32_t n) noexcept {
    if (n < 2 || n % 2 == 0) return n == 2;
    for (std::uint32_t d = 3; d <= n / d; d += 2) if (n % d == 0) return false;
    return true;
}
consteval std::uint32_t ntt_power(std::uint32_t a, std::uint32_t exponent, std::uint32_t mod) noexcept {
    std::uint32_t result = 1;
    for (; exponent != 0; exponent >>= 1) {
        if (exponent & 1) result = static_cast<std::uint32_t>(std::uint64_t{result} * a % mod);
        a = static_cast<std::uint32_t>(std::uint64_t{a} * a % mod);
    }
    return result;
}

inline std::size_t convolution_result_size(std::size_t a, std::size_t b, const char* overflow_message) {
    if (a == 0 || b == 0) return 0;
    if (a - 1 > std::numeric_limits<std::size_t>::max() - b)
        throw std::length_error(overflow_message);
    return a - 1 + b;
}
}
#endif

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#ifndef SIRKIT_NTT_SIMD_DETAIL_DEFINED
#define SIRKIT_NTT_SIMD_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline bool ntt_avx2_available() noexcept {
    static const bool available=__builtin_cpu_supports("avx2");
    return available;
}
}
#endif
#endif

#ifndef SIRKIT_NTT_DEFINED
#define SIRKIT_NTT_DEFINED
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3>
class NTT {
    static_assert(Mod >= 3 && Mod < (std::uint32_t{1} << 31) && /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_prime(Mod),
                  "NTT modulus must be an odd prime below 2^31");
    static_assert(Root > 0 && Root < Mod, "NTT root outside modulus");
    static constexpr int rank_ = std::countr_zero(Mod - 1);
    static constexpr std::uint32_t highest_root_ = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_power(Root, (Mod - 1) >> rank_, Mod);
    static_assert(/*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_power(highest_root_, std::uint32_t{1} << (rank_ - 1), Mod) == Mod - 1,
                  "NTT root has insufficient power-of-two order");
    std::vector<std::uint32_t> roots_;

    static constexpr std::uint32_t multiply(std::uint32_t a, std::uint32_t b) noexcept {
        return static_cast<std::uint32_t>(std::uint64_t{a} * b % Mod);
    }
    static constexpr std::uint32_t add(std::uint32_t a, std::uint32_t b) noexcept {
        const auto sum = a + b;
        return sum >= Mod ? sum - Mod : sum;
    }
    static constexpr std::uint32_t subtract(std::uint32_t a, std::uint32_t b) noexcept {
        return a >= b ? a - b : a + Mod - b;
    }
    static constexpr bool montgomery_ = Mod < (std::uint32_t{1} << 30);
    static constexpr std::uint32_t negative_inverse_ = [] {
        std::uint32_t x=Mod;for(int i=0;i<5;++i)x*=2u-Mod*x;return 0u-x;
    }();
    static constexpr std::uint32_t montgomery_one_ = static_cast<std::uint32_t>((std::uint64_t{1} << 32)%Mod);
    static constexpr std::uint32_t r_squared_ = static_cast<std::uint32_t>(std::uint64_t{montgomery_one_}*montgomery_one_%Mod);
    static constexpr std::array<std::uint32_t, rank_ + 1> stage_roots_ = [] {
        std::array<std::uint32_t, rank_ + 1> table{};
        table[rank_] = highest_root_;
        for (int k = rank_; k >= 2; --k)
            table[k - 1] = static_cast<std::uint32_t>(std::uint64_t{table[k]} * table[k] % Mod);
        if constexpr (montgomery_) {
            for (auto& value : table) {
                const auto product = std::uint64_t{value} * r_squared_;
                const auto q = static_cast<std::uint32_t>(std::uint64_t{static_cast<std::uint32_t>(product)} * negative_inverse_);
                value = static_cast<std::uint32_t>((product + std::uint64_t{q} * Mod) >> 32);
            }
        }
        return table;
    }();
    static constexpr std::uint32_t working_multiply(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_) {
            // 内部値は2Mod未満。t+qMod<2^63で、還元後も2Mod未満。
            const auto product=std::uint64_t{a}*b;
            const auto q=static_cast<std::uint32_t>(product)*negative_inverse_;
            const auto value=static_cast<std::uint32_t>((product+std::uint64_t{q}*Mod)>>32);
            return value;
        } else return multiply(a,b);
    }
    static constexpr std::uint32_t working_normalize(std::uint32_t value) noexcept {
        if constexpr(montgomery_)return value>=Mod?value-Mod:value;
        else return value;
    }
    static constexpr std::uint32_t working_add(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_){const auto value=a+b;return value>=2*Mod?value-2*Mod:value;}
        else return add(a,b);
    }
    static constexpr std::uint32_t working_subtract(std::uint32_t a,std::uint32_t b) noexcept {
        if constexpr(montgomery_)return a>=b?a-b:a+2*Mod-b;
        else return subtract(a,b);
    }
    static constexpr std::uint32_t encode(std::uint32_t value) noexcept {
        if constexpr(montgomery_)return working_multiply(value,r_squared_);
        else return value;
    }
    template<bool Encode,bool Check>
    static void bit_reverse_convert(std::vector<std::uint32_t>& a) {
        const auto convert=[](std::uint32_t value) {
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check)if(value>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            if constexpr(Encode)return encode(value);
            else return working_normalize(working_multiply(value,1));
        };
        a[0]=convert(a[0]);
        for(std::size_t i=1,reverse=0;i<a.size();++i) {
            std::size_t bit=a.size()>>1;
            for(;reverse&bit;bit>>=1)reverse^=bit;
            reverse^=bit;
            if(i<reverse){const auto lo=convert(a[i]),hi=convert(a[reverse]);a[i]=hi;a[reverse]=lo;}
            else if(i==reverse)a[i]=convert(a[i]);
        }
    }
    template <std::integral T>
    static std::uint32_t normalize(T value) noexcept {
        static_assert(sizeof(T) <= sizeof(std::uint64_t));
        if constexpr (std::is_signed_v<T>) {
            const auto remainder = static_cast<std::int64_t>(value) % static_cast<std::int64_t>(Mod);
            return static_cast<std::uint32_t>(remainder < 0 ? remainder + Mod : remainder);
        } else return static_cast<std::uint32_t>(static_cast<std::uint64_t>(value) % Mod);
    }
    static std::size_t result_size(std::size_t a, std::size_t b) {
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::convolution_result_size(a, b, "NTT: convolution size overflow");
    }
    static std::size_t transform_size(std::size_t output) {
        if (output > max_length()) throw std::length_error("NTT: transform too long");
        return std::bit_ceil(output);
    }
    static void bit_reverse(std::vector<std::uint32_t>& a) noexcept {
        const std::size_t n = a.size();
        for (std::size_t i = 1, reverse = 0; i < n; ++i) {
            std::size_t bit = n >> 1;
            for (; reverse & bit; bit >>= 1) reverse ^= bit;
            reverse ^= bit;
            if (i < reverse) std::swap(a[i], a[reverse]);
        }
    }

#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    static bool use_avx2(std::size_t n) noexcept {
        if constexpr(montgomery_)return n>=16 && /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::ntt_avx2_available();
        else return false;
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_reduce(__m256i value) noexcept {
        const auto mod=_mm256_set1_epi32(static_cast<int>(Mod));
        return _mm256_min_epu32(value,_mm256_sub_epi32(value,mod));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_add(__m256i a,__m256i b) noexcept {
        const auto sum=_mm256_add_epi32(a,b);return _mm256_min_epu32(sum,_mm256_sub_epi32(sum,_mm256_set1_epi32(static_cast<int>(2*Mod))));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_subtract(__m256i a,__m256i b) noexcept {
        const auto difference=_mm256_sub_epi32(a,b);return _mm256_min_epu32(difference,_mm256_add_epi32(difference,_mm256_set1_epi32(static_cast<int>(2*Mod))));
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_multiply(__m256i a,__m256i b) noexcept {
        const auto mod=_mm256_set1_epi32(static_cast<int>(Mod));
        const auto inverse=_mm256_set1_epi32(static_cast<int>(negative_inverse_));
        const auto even=_mm256_mul_epu32(a,b);
        const auto odd=_mm256_mul_epu32(_mm256_srli_epi64(a,32),_mm256_srli_epi64(b,32));
        const auto q_even=_mm256_mul_epu32(even,inverse),q_odd=_mm256_mul_epu32(odd,inverse);
        const auto reduced_even=_mm256_srli_epi64(_mm256_add_epi64(even,_mm256_mul_epu32(q_even,mod)),32);
        const auto reduced_odd=_mm256_add_epi64(odd,_mm256_mul_epu32(q_odd,mod));
        return _mm256_blend_epi32(reduced_even,reduced_odd,0xaa);
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    // 4点段の偶数レーンの根は1。奇数レーンだけ虚数単位の根を掛ける。
    static inline __m256i vector_multiply_quarter_root(__m256i value, __m256i imaginary) noexcept {
        const auto odd = _mm256_mul_epu32(_mm256_srli_epi64(value, 32), imaginary);
        const auto q = _mm256_mul_epu32(odd, _mm256_set1_epi32(static_cast<int>(negative_inverse_)));
        const auto reduced = _mm256_add_epi64(odd, _mm256_mul_epu32(q, _mm256_set1_epi32(static_cast<int>(Mod))));
        return _mm256_blend_epi32(value, reduced, 0xaa);
    }
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline __m256i vector_multiply_prepared(__m256i a, __m256i b,
                                                  __m256i b_times_negative_inverse) noexcept {
        const auto mod = _mm256_set1_epi32(static_cast<int>(Mod));
        const auto even = _mm256_mul_epu32(a, b);
        const auto odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
        const auto q_even = _mm256_mul_epu32(a, b_times_negative_inverse);
        const auto q_odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b_times_negative_inverse, 32));
        const auto reduced_even = _mm256_srli_epi64(_mm256_add_epi64(even, _mm256_mul_epu32(q_even, mod)), 32);
        const auto reduced_odd = _mm256_add_epi64(odd, _mm256_mul_epu32(q_odd, mod));
        return _mm256_blend_epi32(reduced_even, reduced_odd, 0xaa);
    }
#ifndef SKIP_RUNTIME_CHECKS
    [[gnu::always_inline,gnu::target("avx2")]]
    static inline void vector_check(__m256i value) {
        const auto maximum=_mm256_set1_epi32(static_cast<int>(Mod-1));
        if(_mm256_movemask_epi8(_mm256_cmpeq_epi32(value,_mm256_min_epu32(value,maximum)))!=-1)
            throw std::invalid_argument("NTT: coefficient not normalized");
    }
#endif
    [[gnu::target("avx2")]]
    static void pointwise_avx2(std::vector<std::uint32_t>& a,const std::vector<std::uint32_t>& b) noexcept {
        for(std::size_t i=0;i<a.size();i+=8) {
            const auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+i));
            const auto y=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b.data()+i));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+i),vector_multiply(x,y));
        }
    }

    [[gnu::target("avx2")]]
    void prepare_avx2(std::size_t previous, std::size_t length) {
        for (std::size_t half = previous; half < length; half <<= 1) {
            const auto step = stage_roots_[std::countr_zero(half) + 1];
            const auto scale = _mm256_set1_epi32(static_cast<int>(step));
            std::size_t j = 0;
            for (; j + 8 <= half / 2; j += 8) {
                const auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data() + half / 2 + j));
                const auto product = vector_multiply(value, scale);
                // 偶数べきと奇数べきを交互に並べる。
                const auto low = _mm256_unpacklo_epi32(value, product);
                const auto high = _mm256_unpackhi_epi32(value, product);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(roots_.data() + half + 2 * j), _mm256_permute2x128_si256(low, high, 0x20));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(roots_.data() + half + 2 * j + 8), _mm256_permute2x128_si256(low, high, 0x31));
            }
            for (; j < half / 2; ++j) {
                const auto value = roots_[half / 2 + j];
                roots_[half + 2 * j] = value;
                roots_[half + 2 * j + 1] = working_multiply(value, step);
            }
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    static std::vector<std::uint32_t> naive_avx2(std::vector<std::uint32_t>& a,std::vector<std::uint32_t>& b,std::size_t output) {
        if(a.size()>b.size())a.swap(b);
        std::vector<std::uint32_t> result(output);
        for(std::size_t i=0;i<a.size();++i) {
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check)if(a[i]>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            const auto encoded_coefficient = encode(a[i]);
            const auto coefficient=_mm256_set1_epi32(static_cast<int>(encoded_coefficient));
            const auto coefficient_times_inverse = static_cast<std::uint32_t>(std::uint64_t{encoded_coefficient} * negative_inverse_);
            const auto qcoefficient = _mm256_set1_epi32(static_cast<int>(coefficient_times_inverse));
            std::size_t j=0;
            for(;j+8<=b.size();j+=8) {
                const auto input=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(b.data()+j));
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)if(i==0)vector_check(input);
#endif
                const auto old=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(result.data()+i+j));
                const auto product=vector_reduce(vector_multiply_prepared(input,coefficient,qcoefficient));
                const auto sum=vector_reduce(_mm256_add_epi32(old,product));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(result.data()+i+j),sum);
            }
            for(;j<b.size();++j) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)if(i==0&&b[j]>=Mod)throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                result[i+j]=add(result[i+j],multiply(a[i],b[j]));
            }
        }
        return result;
    }
    template<bool Check, bool SkipFirst = false>
    [[gnu::target("avx2")]]
    void forward_avx2(std::vector<std::uint32_t>& a, [[maybe_unused]] std::size_t remaining_length=0) const {
        const auto n=a.size();

        std::size_t length=SkipFirst ? remaining_length : n;
        for(;length>=32;length>>=2) {
            const auto quarter=length/4;
            for(std::size_t block=0;block<n;block+=length)for(std::size_t j=0;j<quarter;j+=8) {
                auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                auto x1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+quarter));
                auto x2=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+2*quarter));
                auto x3=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+3*quarter));
                if constexpr (!SkipFirst) if(length==n) {
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check){vector_check(x0);vector_check(x1);vector_check(x2);vector_check(x3);}
#endif
                    const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
                    x0=vector_multiply(x0,r2);x1=vector_multiply(x1,r2);x2=vector_multiply(x2,r2);x3=vector_multiply(x3,r2);
                }
                const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
                const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
                const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
                const auto v0=vector_add(x0,x2),v1=vector_add(x1,x3);
                const auto v2=vector_multiply(vector_subtract(x0,x2),first),v3=vector_multiply(vector_subtract(x1,x3),second);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(v0,v1));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+quarter),vector_multiply(vector_subtract(v0,v1),third));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+2*quarter),vector_add(v2,v3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+3*quarter),vector_multiply(vector_subtract(v2,v3),third));
            }
        }
        if(length==16) {
            const auto half=length>>1;
            for(std::size_t block=0;block<n;block+=length) {
                std::size_t j=0;
                for(;j+8<=half;j+=8) {
                    auto u=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                    auto v=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+half));
                    if constexpr (!SkipFirst) if(length==n) {
#ifndef SKIP_RUNTIME_CHECKS
                        if constexpr(Check){vector_check(u);vector_check(v);}
#endif
                        const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
                        u=vector_multiply(u,r2);v=vector_multiply(v,r2);
                    }
                    const auto roots=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+half+j));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(u,v));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+half),vector_multiply(vector_subtract(u,v),roots));
                }
                for(;j<half;++j) {
                    auto u=a[block+j],v=a[block+j+half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check)if(length==n&&(u>=Mod||v>=Mod))throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    if constexpr (!SkipFirst) if(length==n){u=encode(u);v=encode(v);}
                    a[block+j]=working_add(u,v);a[block+j+half]=working_multiply(working_subtract(u,v),roots_[half+j]);
                }
            }
        }
        const auto u4=_mm256_setr_epi32(0,1,0,1,4,5,4,5),v4=_mm256_setr_epi32(2,3,2,3,6,7,6,7);
        const auto r4=_mm256_set1_epi32(static_cast<int>(roots_[3]));
        const auto r8=_mm256_setr_epi32(static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]),static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]));
        for(std::size_t block=0;block<n;block+=8) {
            auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block));
            auto u=_mm256_permute2x128_si256(x,x,0),v=_mm256_permute2x128_si256(x,x,0x11);
            x=_mm256_blend_epi32(vector_add(u,v),vector_multiply(vector_subtract(u,v),r8),0xf0);
            u=_mm256_permutevar8x32_epi32(x,u4);v=_mm256_permutevar8x32_epi32(x,v4);
            x=_mm256_blend_epi32(vector_add(u,v),vector_multiply_quarter_root(vector_subtract(u,v),r4),0xcc);
            u=_mm256_shuffle_epi32(x,_MM_SHUFFLE(2,2,0,0));v=_mm256_shuffle_epi32(x,_MM_SHUFFLE(3,3,1,1));
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xaa);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block),x);
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    void forward_zero_padded_avx2(std::vector<std::uint32_t>& a) const {
        const auto n=a.size(), quarter=n/4;
        const auto r2=_mm256_set1_epi32(static_cast<int>(r_squared_));
        // 後半はresizeが補ったゼロ。初段のゼロの符号化と積を省く。
        for(std::size_t j=0;j<quarter;j+=8) {
            auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+j));
            auto x1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+quarter+j));
#ifndef SKIP_RUNTIME_CHECKS
            if constexpr(Check){vector_check(x0);vector_check(x1);}
#endif
            x0=vector_multiply(x0,r2);x1=vector_multiply(x1,r2);
            const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
            const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
            const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
            const auto v2=vector_multiply(x0,first),v3=vector_multiply(x1,second);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+j),vector_add(x0,x1));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+quarter+j),vector_multiply(vector_subtract(x0,x1),third));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+2*quarter+j),vector_add(v2,v3));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+3*quarter+j),vector_multiply(vector_subtract(v2,v3),third));
        }
        forward_avx2<false,true>(a,n/4);
    }
    template<bool Check, bool Encode>
    [[gnu::target("avx2")]]
    void forward_zero_quarters_avx2(std::vector<std::uint32_t>& a,
                                    std::size_t length, std::size_t active) const {
        const auto quarter=length/4;
        // 各区間の先頭active要素以外は、以前の段でもゼロのまま。
        for(std::size_t block=0;block<a.size();block+=length)for(std::size_t j=0;j<active;j+=8) {
            auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
            if constexpr(Encode) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr(Check)vector_check(x0);
#endif
                x0=vector_multiply(x0,_mm256_set1_epi32(static_cast<int>(r_squared_)));
            }
            const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
            const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
            const auto v2=vector_multiply(x0,first);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),x0);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+quarter+j),vector_multiply(x0,third));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+2*quarter+j),v2);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+3*quarter+j),vector_multiply(v2,third));
        }
    }
    template<bool Check>
    [[gnu::target("avx2")]]
    void backward_avx2(std::vector<std::uint32_t>& a) const {
        const auto n=a.size();

        const auto u4=_mm256_setr_epi32(0,1,0,1,4,5,4,5),v4=_mm256_setr_epi32(2,3,2,3,6,7,6,7);
        const auto r4=_mm256_set1_epi32(static_cast<int>(roots_[3]));
        const auto r8=_mm256_setr_epi32(static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]),static_cast<int>(roots_[4]),static_cast<int>(roots_[5]),static_cast<int>(roots_[6]),static_cast<int>(roots_[7]));
        for(std::size_t block=0;block<n;block+=8) {
            auto x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block));
            auto u=_mm256_shuffle_epi32(x,_MM_SHUFFLE(2,2,0,0)),v=_mm256_shuffle_epi32(x,_MM_SHUFFLE(3,3,1,1));
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xaa);
            u=_mm256_permutevar8x32_epi32(x,u4);v=vector_multiply_quarter_root(_mm256_permutevar8x32_epi32(x,v4),r4);
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xcc);
            u=_mm256_permute2x128_si256(x,x,0);v=vector_multiply(_mm256_permute2x128_si256(x,x,0x11),r8);
            x=_mm256_blend_epi32(vector_add(u,v),vector_subtract(u,v),0xf0);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block),x);
        }

        std::size_t completed=8;
        for(std::size_t length=32;length<=n;) {
            const auto quarter=length/4;
            for(std::size_t block=0;block<n;block+=length)for(std::size_t j=0;j<quarter;j+=8) {
                const auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                const auto raw1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+quarter));
                const auto x2=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+2*quarter));
                const auto raw3=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+3*quarter));
                const auto first=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+quarter+j));
                const auto second=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+2*quarter+j));
                const auto third=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+3*quarter+j));
                const auto x1=vector_multiply(raw1,first),x3=vector_multiply(raw3,first);
                const auto v0=vector_add(x0,x1),v1=vector_subtract(x0,x1);
                const auto v2=vector_multiply(vector_add(x2,x3),second),v3=vector_multiply(vector_subtract(x2,x3),third);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(v0,v2));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+quarter),vector_add(v1,v3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+2*quarter),vector_subtract(v0,v2));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+3*quarter),vector_subtract(v1,v3));
            }
            completed=length;
            if(length>n/4)break;
            length*=4;
        }
        if(completed<n) {
            const auto length=n;
            const auto half=length>>1;
            for(std::size_t block=0;block<n;block+=length) {
                std::size_t j=0;
                for(;j+8<=half;j+=8) {
                    const auto u=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j));
                    const auto raw=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+block+j+half));
                    const auto roots=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(roots_.data()+half+j));
                    const auto v=vector_multiply(raw,roots);
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j),vector_add(u,v));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+block+j+half),vector_subtract(u,v));
                }
                for(;j<half;++j) {
                    const auto u=a[block+j],raw=a[block+j+half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr(Check)if(length==2&&(u>=Mod||raw>=Mod))throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    const auto v=working_multiply(raw,roots_[half+j]);
                    a[block+j]=working_add(u,v);a[block+j+half]=working_subtract(u,v);
                }
            }
        }
        // nはMod-1の約数（変換長は対応する2冪）。
        // d=(Mod-1)/nなら、n*(Mod-d)=(n-1)*Mod+1。
        // よってMod-dがnの逆元になる。
        const auto inverse_n=static_cast<std::uint32_t>(Mod - (Mod - 1) / static_cast<std::uint32_t>(n));
        const auto scale=_mm256_set1_epi32(static_cast<int>(inverse_n));
        const auto reverse=_mm256_setr_epi32(7,6,5,4,3,2,1,0);
        a[0]=working_normalize(working_multiply(a[0],inverse_n));
        std::size_t i=1;
        for(;i+8<=n/2;i+=8) {
            const auto lo=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+i));
            const auto hi=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(a.data()+n-i-7));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+i),_mm256_permutevar8x32_epi32(vector_reduce(vector_multiply(hi,scale)),reverse));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(a.data()+n-i-7),_mm256_permutevar8x32_epi32(vector_reduce(vector_multiply(lo,scale)),reverse));
        }
        for(;i<n-i;++i) {
            const auto lo=working_normalize(working_multiply(a[i],inverse_n)),hi=working_normalize(working_multiply(a[n-i],inverse_n));
            a[i]=hi;a[n-i]=lo;
        }
        a[n/2]=working_normalize(working_multiply(a[n/2],inverse_n));
    }
#endif

    template <bool Check>
    void forward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if constexpr(montgomery_)if(use_avx2(n)){forward_avx2<Check>(a);return;}
#endif
        for (std::size_t length = n; length >= 2; length >>= 1) {
            const std::size_t half = length >> 1;
            for (std::size_t block = 0; block < n; block += length)
                for (std::size_t j = 0; j < half; ++j) {
                    auto u = a[block + j], v = a[block + j + half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (length == n && (u >= Mod || v >= Mod))
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    if constexpr(montgomery_)if(length==n){u=encode(u);v=encode(v);}
                    a[block + j] = working_add(u, v);
                    a[block + j + half] = working_multiply(working_subtract(u, v), roots_[half + j]);
                }
        }
    }
    template<bool Check>
    void forward_padded(std::vector<std::uint32_t>& a, [[maybe_unused]] std::size_t input_size) const {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if constexpr(montgomery_) if(a.size()>=32 && input_size<=a.size()/2 && use_avx2(a.size())) {
            if(input_size<=a.size()/4) {
                // 長さは上限検査済み。8要素へ丸めても各区間の1/4以内に収まる。
                const auto active=(input_size+7)&~std::size_t{7};
                auto length=a.size();
                forward_zero_quarters_avx2<Check,true>(a,length,active);
                length/=4;
                while(length>=32 && active<=length/4) {
                    forward_zero_quarters_avx2<false,false>(a,length,active);
                    length/=4;
                }
                forward_avx2<false,true>(a,length);return;
            }
            forward_zero_padded_avx2<Check>(a);return;
        }
#endif
        forward<Check>(a);
    }
    template <bool Check>
    void backward(std::vector<std::uint32_t>& a) const {
        const std::size_t n = a.size();
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if constexpr(montgomery_)if(use_avx2(n)){backward_avx2<Check>(a);return;}
#endif
        for (std::size_t length = 2; length <= n; length <<= 1) {
            const std::size_t half = length >> 1;
            for (std::size_t block = 0; block < n; block += length)
                for (std::size_t j = 0; j < half; ++j) {
                    const auto u = a[block + j], raw = a[block + j + half];
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (length == 2 && (u >= Mod || raw >= Mod))
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    const auto v = working_multiply(raw, roots_[half + j]);
                    a[block + j] = working_add(u, v);
                    a[block + j + half] = working_subtract(u, v);
                }
        }
        // nはMod-1の約数（変換長は対応する2冪）。
        // d=(Mod-1)/nなら、n*(Mod-d)=(n-1)*Mod+1。
        // よってMod-dがnの逆元になる。
        const auto inverse_n = static_cast<std::uint32_t>(Mod - (Mod - 1) / static_cast<std::uint32_t>(n));
        a[0] = working_normalize(working_multiply(a[0], inverse_n));
        for (std::size_t i = 1; i < n - i; ++i) {
            const auto lo = working_normalize(working_multiply(a[i], inverse_n)), hi = working_normalize(working_multiply(a[n - i], inverse_n));
            a[i] = hi; a[n - i] = lo;
        }
        if (n > 1) a[n / 2] = working_normalize(working_multiply(a[n / 2], inverse_n));
    }
    template <bool Check>
    std::vector<std::uint32_t> compute(std::vector<std::uint32_t> a, std::vector<std::uint32_t> b) {
        const std::size_t output = result_size(a.size(), b.size());
        if (output == 0) return {};
        if (std::min(a.size(), b.size()) <= 32) {
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
            if(use_avx2(output))return naive_avx2<Check>(a,b,output);
#endif
            std::vector<std::uint32_t> result(output);
            for (std::size_t i = 0; i < a.size(); ++i) {
#ifndef SKIP_RUNTIME_CHECKS
                if constexpr (Check) if (a[i] >= Mod) throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                for (std::size_t j = 0; j < b.size(); ++j) {
#ifndef SKIP_RUNTIME_CHECKS
                    if constexpr (Check) if (i == 0 && b[j] >= Mod)
                        throw std::invalid_argument("NTT: coefficient not normalized");
#endif
                    result[i + j] = add(result[i + j], multiply(a[i], b[j]));
                }
            }
            return result;
        }
        const auto length = transform_size(output);
        prepare(length);
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        const auto input_a=a.size(), input_b=b.size();
#endif
        a.resize(length); b.resize(length);
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        forward_padded<Check>(a,input_a); forward_padded<Check>(b,input_b);
#else
        forward<Check>(a); forward<Check>(b);
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if(use_avx2(length))pointwise_avx2(a,b);
        else
#endif
        for (std::size_t i = 0; i < length; ++i) a[i] = working_multiply(a[i], b[i]);
        backward<false>(a); a.resize(output);
        return a;
    }

public:
    static constexpr std::uint32_t modulus() noexcept { return Mod; }
    static constexpr std::size_t max_length() noexcept { return std::size_t{1} << rank_; }
    [[nodiscard]] std::size_t roots_capacity() const noexcept { return roots_.size(); }
    void clear_cache() noexcept { std::vector<std::uint32_t>{}.swap(roots_); }
    void prepare(std::size_t length) {
        if (length == 0) return;
        if (!std::has_single_bit(length) || length > max_length())
            throw std::length_error("NTT: length must be a supported power of two");
        if (length <= 1 || length <= roots_.size()) return;
        const std::size_t previous = std::max<std::size_t>(2, roots_.size());
        roots_.resize(length); roots_[1] = encode(1);
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if constexpr (montgomery_) if (length >= 256 && use_avx2(length)) {
            prepare_avx2(previous, length);
            return;
        }
#endif
        for (std::size_t half = previous; half < length; half <<= 1) {
            const auto step = stage_roots_[std::countr_zero(half) + 1];
            for (std::size_t j = 0; j < half/2; ++j) {
                const auto value=roots_[half/2+j];
                roots_[half+2*j]=value;
                roots_[half+2*j+1]=working_multiply(value,step);
            }
        }
    }
    void transform(std::vector<std::uint32_t>& a, bool inverse = false) {
        if (a.empty()) return;
        prepare(a.size());
        if (a.size() == 1) {
#ifndef SKIP_RUNTIME_CHECKS
            if (a[0] >= Mod) throw std::invalid_argument("NTT: coefficient not normalized");
#endif
            return;
        }
        if constexpr(montgomery_) {
            if(inverse){bit_reverse_convert<true,true>(a);backward<false>(a);}
            else{forward<true>(a);bit_reverse_convert<false,false>(a);}
        }else{
        if (inverse) { bit_reverse(a); backward<true>(a); }
        else { forward<true>(a); bit_reverse(a); }
        }
    }
    template <std::integral T, std::integral U>
    std::vector<std::uint32_t> convolution(const std::vector<T>& a, const std::vector<U>& b) {
        if (a.empty() || b.empty()) return {};
        const auto output = result_size(a.size(), b.size());
        if (std::min(a.size(), b.size()) > 32) (void)transform_size(output);
        std::vector<std::uint32_t> x(a.size()), y(b.size());
        for (std::size_t i = 0; i < a.size(); ++i) x[i] = normalize<T>(a[i]);
        for (std::size_t i = 0; i < b.size(); ++i) y[i] = normalize<U>(b[i]);
        return compute<false>(std::move(x), std::move(y));
    }
    std::vector<std::uint32_t> convolution_normalized(std::vector<std::uint32_t> a,
                                                      std::vector<std::uint32_t> b) {
        return compute<true>(std::move(a), std::move(b));
    }
    template <std::integral T>
    std::vector<std::uint32_t> square(const std::vector<T>& a) {
        if (a.size() <= 32) return convolution(a, a);
        const auto output = result_size(a.size(), a.size()), length = transform_size(output);
        prepare(length);
        std::vector<std::uint32_t> x(length);
        for (std::size_t i = 0; i < a.size(); ++i) x[i] = normalize<T>(a[i]);
        #if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        forward_padded<false>(x,a.size());
#else
        forward<false>(x);
#endif
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
        if(use_avx2(length))pointwise_avx2(x,x);
        else
#endif
        for (auto& value : x) value = working_multiply(value, value);
        backward<false>(x); x.resize(output);
        return x;
    }
};
#endif

#ifndef SIRKIT_CONVOLUTION_MOD_DEFINED
#define SIRKIT_CONVOLUTION_MOD_DEFINED
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3, std::integral T, std::integral U>
std::vector<std::uint32_t> ConvolutionMod(const std::vector<T>& a, const std::vector<U>& b) {
    return NTT<Mod, Root>{}.convolution(a, b);
}
#endif

#ifndef SIRKIT_TREE_DISTANCE_FREQUENCY_DEFINED
#define SIRKIT_TREE_DISTANCE_FREQUENCY_DEFINED

namespace detail
{
using TreeDistanceNtt = NTT<998244353, 3>;
using TreeDistanceSecondNtt = NTT<469762049, 3>;

inline void check_tree_distance_frequency_size(std::size_t n)
{
    if (n > TreeDistanceNtt::max_length())
    {
        throw std::length_error("tree_distance_frequency: vertex count exceeds transform capacity");
    }
}

class TreeDistanceProducts
{
    TreeDistanceNtt first_;
    TreeDistanceSecondNtt second_;
    std::vector<std::uint32_t> work_;

public:
    void add(std::vector<std::int64_t> &frequency, const std::vector<std::uint32_t> &prefix,
             std::span<const std::uint32_t> branch, std::uint64_t coefficient_bound)
    {
        if (std::min(prefix.size(), branch.size()) <= 32)
        {
            for (std::size_t i = 0; i < prefix.size(); ++i)
            {
                for (std::size_t j = 0; j < branch.size(); ++j)
                {
                    frequency[i + j + 1] += std::int64_t{prefix[i]} * branch[j];
                }
            }
            return;
        }

        constexpr auto kFirstMod = TreeDistanceNtt::modulus();
        work_.assign(branch.begin(), branch.end());
        const auto first = first_.convolution(prefix, work_);
        if (coefficient_bound < kFirstMod)
        {
            for (std::size_t i = 0; i < first.size(); ++i)
            {
                frequency[i + 1] += first[i];
            }
            return;
        }

        // 頂点数の制限より、各係数は2つの法の積より小さい。
        constexpr auto kSecondMod = TreeDistanceSecondNtt::modulus();
        constexpr auto kInverse = ntt_power(kFirstMod % kSecondMod, kSecondMod - 2, kSecondMod);
        const auto second = second_.convolution(prefix, work_);
        for (std::size_t i = 0; i < first.size(); ++i)
        {
            const auto delta = (second[i] + kSecondMod - first[i] % kSecondMod) % kSecondMod;
            const auto quotient = std::uint64_t{delta} * kInverse % kSecondMod;
            frequency[i + 1] += static_cast<std::int64_t>(first[i] + quotient * kFirstMod);
        }
    }
};
} // namespace detail

inline std::vector<std::int64_t> tree_distance_frequency(const Graph &graph)
{
    detail::check_tree_distance_frequency_size(graph.size());
    const CentroidDecomposition decomposition(graph, false);
    std::vector<std::int64_t> frequency(graph.size());
    std::vector<unsigned char> removed(graph.size());
    std::vector<std::size_t> active_degree;
    active_degree.reserve(graph.size());
    for (const auto &neighbors : graph)
    {
        active_degree.push_back(neighbors.size());
    }

    struct Visit
    {
        int vertex;
        int parent;
        std::size_t depth;
    };
    struct Branch
    {
        std::size_t start;
        std::size_t width;
        std::size_t count;
        std::uint32_t largest;
    };

    detail::TreeDistanceProducts products;
    std::vector<Visit> visits;
    std::vector<Branch> branches;
    std::vector<std::uint32_t> coefficients, prefix;
    for (int centroid : decomposition.order())
    {
        removed[centroid] = 1;
        if (decomposition.component_size(centroid) == 1)
        {
            continue;
        }

        coefficients.clear();
        branches.clear();
        std::uint32_t leaves = 0;
        for (int root : graph[centroid])
        {
            if (removed[root])
            {
                continue;
            }
            --active_degree[root];
            if (active_degree[root] == 0)
            {
                ++leaves;
                continue;
            }
            const auto start = coefficients.size();
            visits.clear();
            visits.push_back({root, centroid, 0});
            for (std::size_t i = 0; i < visits.size(); ++i)
            {
                const auto [v, parent, depth] = visits[i];
                if (start + depth == coefficients.size())
                {
                    coefficients.push_back(0);
                }
                ++coefficients[start + depth];
                for (int to : graph[v])
                {
                    if (to != parent && !removed[to])
                    {
                        visits.push_back({to, v, depth + 1});
                    }
                }
            }
            const auto largest = *std::max_element(coefficients.begin() + start, coefficients.end());
            branches.push_back({start, coefficients.size() - start, visits.size(), largest});
        }

        prefix.assign(1, 1);
        frequency[1] += leaves;
        if (leaves != 0)
        {
            prefix.push_back(leaves);
            if (leaves > 1)
            {
                frequency[2] += std::int64_t{leaves} * (leaves - 1) / 2;
            }
        }

        // 短い枝から混ぜると、畳み込みの長さが現在の枝の深さに比例する。
        std::ranges::sort(branches, {}, &Branch::width);
        std::uint64_t prefix_count = std::uint64_t{leaves} + 1;
        std::uint32_t prefix_largest = std::max(std::uint32_t{1}, leaves);
        for (const auto &branch : branches)
        {
            const auto part = std::span<const std::uint32_t>(coefficients).subspan(branch.start, branch.width);
            const auto bound = std::min(std::uint64_t{prefix_largest} * branch.count, prefix_count * branch.largest);
            products.add(frequency, prefix, part, bound);

            prefix.resize(part.size() + 1);
            for (std::size_t i = 0; i < part.size(); ++i)
            {
                prefix[i + 1] += part[i];
                prefix_largest = std::max(prefix_largest, prefix[i + 1]);
            }
            prefix_count += branch.count;
        }
    }
    return frequency;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
