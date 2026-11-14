// SPDX-License-Identifier: CC0-1.0
// 数学的参考: https://info.atcoder.jp/entry/algorithm_lectures/prufer_code
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/pruefer_code.md
//
// API一覧
// complete_tree_with_degrees以外はすべてO(n)時間・領域。
// 頂点は0..n-1、n>=1。辺はpair<int,int>。入力を変更せず、結果を所有するvectorで返す。
// pruefer_encode(n,edges): 単純無向木の辺n-1本から、最小の葉を順に除くPrüfer列を返す。
// pruefer_decode(n,code): 長さmax(0,n-2)の列を辺リストへ戻す。各要素は0..n-1。
// pruefer_decode(code): n=code.size()+2として復元する。空列なら2頂点。
// pruefer_degrees(n,code): 復元される木の次数列を返す。
// pruefer_from_degrees(degrees): 指定次数を持つ木のPrüfer列。存在しなければnullopt。
// tree_from_degrees(degrees): 指定次数を持つ木の辺。存在しなければnullopt。
// complete_tree_with_degrees(degrees,edges): 指定次数の木にする追加辺。存在しなければnullopt。
// 既存辺は森が前提。循環・次数不足はnullopt。O((n+m)α(n))時間、O(n+m)領域。
// n=1では辺/符号とも空、次数は{0}。n=2では符号は空、辺は{0,1}、次数は{1,1}。
// n=0の次数列は木を持たずnullopt。符号だけでは1頂点と2頂点を区別できない。
// SKIP_RUNTIME_CHECKSでn/入力長/辺端点/符号値/除去中に分かる非木の検査を除く。
// encodeの検査は次数構築・葉除去中に行う。未参照の入力を検査できるとは保証しない。
// 次数列の実現可能性と件数の表現上限は検査省略時も判定する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PRUEFER_DEGREES_DEFINED
#define SIRKIT_PRUEFER_DEGREES_DEFINED
inline std::vector<int> pruefer_degrees(int n, std::span<const int> code) {
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 1 || code.size() != (n < 2 ? 0 : static_cast<std::size_t>(n - 2)))
        throw std::invalid_argument("Pruefer code length");
#endif
    std::vector<int> degrees(static_cast<std::size_t>(n), n == 1 ? 0 : 1);
    for (const int vertex : code) {
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n) throw std::out_of_range("Pruefer vertex");
#endif
        ++degrees[static_cast<std::size_t>(vertex)];
    }
    return degrees;
}
#endif

#ifndef SIRKIT_PRUEFER_DECODE_DEFINED
#define SIRKIT_PRUEFER_DECODE_DEFINED
inline std::vector<std::pair<int, int>> pruefer_decode(int n, std::span<const int> code) {
    auto degrees = pruefer_degrees(n, code);
    std::vector<std::pair<int, int>> edges;
    if (n == 1) return edges;
    edges.reserve(static_cast<std::size_t>(n - 1));
    int scan = 0;
    while (degrees[static_cast<std::size_t>(scan)] != 1) ++scan;
    int leaf = scan;
    for (const int vertex : code) {
        edges.emplace_back(leaf, vertex);
        degrees[static_cast<std::size_t>(leaf)] = 0;
        if (--degrees[static_cast<std::size_t>(vertex)] == 1 && vertex < scan) leaf = vertex;
        else {
            do { ++scan; } while (degrees[static_cast<std::size_t>(scan)] != 1);
            leaf = scan;
        }
    }
    edges.emplace_back(leaf, n - 1);
    return edges;
}
inline std::vector<std::pair<int, int>> pruefer_decode(std::span<const int> code) {
    if (code.size() > static_cast<std::size_t>(std::numeric_limits<int>::max() - 2))
        throw std::length_error("Pruefer vertex count");
    return pruefer_decode(static_cast<int>(code.size()) + 2, code);
}
#endif

#ifndef SIRKIT_PRUEFER_ENCODE_DEFINED
#define SIRKIT_PRUEFER_ENCODE_DEFINED
inline std::vector<int> pruefer_encode(int n, std::span<const std::pair<int, int>> edges) {
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 1 || edges.size() != static_cast<std::size_t>(n - 1))
        throw std::invalid_argument("Pruefer tree edge count");
#endif
    std::vector<int> degrees(static_cast<std::size_t>(n)), neighbors(static_cast<std::size_t>(n));
    for (const auto& [u, v] : edges) {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n || u == v)
            throw std::invalid_argument("Pruefer tree edge");
#endif
        ++degrees[static_cast<std::size_t>(u)]; ++degrees[static_cast<std::size_t>(v)];
        neighbors[static_cast<std::size_t>(u)] ^= v;
        neighbors[static_cast<std::size_t>(v)] ^= u;
    }
    std::vector<int> code;
    if (n <= 2) return code;
    code.reserve(static_cast<std::size_t>(n - 2));
    int scan = 0;
    while (scan < n && degrees[static_cast<std::size_t>(scan)] != 1) ++scan;
    int leaf = scan;
    for (int step = 0; step < n - 2; ++step) {
#ifndef SKIP_RUNTIME_CHECKS
        if (leaf == n || degrees[static_cast<std::size_t>(leaf)] != 1)
            throw std::invalid_argument("Pruefer input is not a tree");
#endif
        const int vertex = neighbors[static_cast<std::size_t>(leaf)];
#ifndef SKIP_RUNTIME_CHECKS
        if (vertex < 0 || vertex >= n || degrees[static_cast<std::size_t>(vertex)] <= 0)
            throw std::invalid_argument("Pruefer input is not a tree");
#endif
        code.push_back(vertex);
        degrees[static_cast<std::size_t>(leaf)] = 0;
        neighbors[static_cast<std::size_t>(vertex)] ^= leaf;
        if (--degrees[static_cast<std::size_t>(vertex)] == 1 && vertex < scan) leaf = vertex;
        else {
            do { ++scan; } while (scan < n && degrees[static_cast<std::size_t>(scan)] != 1);
            leaf = scan;
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    if (leaf == n || leaf == n - 1 || degrees[static_cast<std::size_t>(leaf)] != 1 ||
        degrees[static_cast<std::size_t>(n - 1)] != 1 ||
        neighbors[static_cast<std::size_t>(leaf)] != n - 1 ||
        neighbors[static_cast<std::size_t>(n - 1)] != leaf)
        throw std::invalid_argument("Pruefer input is not a tree");
#endif
    return code;
}
#endif

#ifndef SIRKIT_PRUEFER_FROM_DEGREES_DEFINED
#define SIRKIT_PRUEFER_FROM_DEGREES_DEFINED
inline std::optional<std::vector<int>> pruefer_from_degrees(std::span<const int> degrees) {
    const auto n = degrees.size();
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("Pruefer vertex count");
    if (n == 0) return std::nullopt;
    if (n == 1) {
        if (degrees[0] == 0) return std::vector<int>{};
        return std::nullopt;
    }
    std::size_t remaining = n - 2;
    std::vector<int> code;
    code.reserve(remaining);
    for (std::size_t vertex = 0; vertex < n; ++vertex) {
        if (degrees[vertex] < 1) return std::nullopt;
        const auto copies = static_cast<std::size_t>(degrees[vertex] - 1);
        if (copies > remaining) return std::nullopt;
        remaining -= copies;
        code.insert(code.end(), copies, static_cast<int>(vertex));
    }
    if (remaining != 0) return std::nullopt;
    return code;
}
#endif

#ifndef SIRKIT_TREE_FROM_DEGREES_DEFINED
#define SIRKIT_TREE_FROM_DEGREES_DEFINED
inline std::optional<std::vector<std::pair<int, int>>> tree_from_degrees(std::span<const int> degrees) {
    auto code = pruefer_from_degrees(degrees);
    if (!code) return std::nullopt;
    return pruefer_decode(static_cast<int>(degrees.size()), *code);
}
#endif

#ifndef SIRKIT_COMPLETE_TREE_WITH_DEGREES_DEFINED
#define SIRKIT_COMPLETE_TREE_WITH_DEGREES_DEFINED
inline std::optional<std::vector<std::pair<int, int>>> complete_tree_with_degrees(
        std::span<const int> degrees, std::span<const std::pair<int, int>> edges) {
    const auto n = degrees.size();
    if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("Pruefer vertex count");
    if (n == 0 || edges.size() >= n) return std::nullopt;
    std::vector<int> residual, parent(n, -1);
    residual.reserve(n);
    std::uint64_t budget = std::uint64_t{2} * (n - 1);
    for (const int degree : degrees) {
        if (degree < (n == 1 ? 0 : 1) || static_cast<std::uint64_t>(degree) > budget)
            return std::nullopt;
        budget -= static_cast<std::uint64_t>(degree);
        residual.push_back(degree);
    }
    if (budget != 0) return std::nullopt;
    const auto find = [&parent](int vertex) {
        int root = vertex;
        while (parent[static_cast<std::size_t>(root)] >= 0) root = parent[static_cast<std::size_t>(root)];
        while (vertex != root) {
            const int next = parent[static_cast<std::size_t>(vertex)];
            parent[static_cast<std::size_t>(vertex)] = root;
            vertex = next;
        }
        return root;
    };
    for (const auto& [u, v] : edges) {
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || v < 0 || static_cast<std::size_t>(u) >= n || static_cast<std::size_t>(v) >= n)
            throw std::out_of_range("Pruefer forest vertex");
#endif
        int a = find(u), b = find(v);
        if (a == b || residual[static_cast<std::size_t>(u)] == 0 || residual[static_cast<std::size_t>(v)] == 0)
            return std::nullopt;
        --residual[static_cast<std::size_t>(u)]; --residual[static_cast<std::size_t>(v)];
        if (parent[static_cast<std::size_t>(a)] > parent[static_cast<std::size_t>(b)]) std::swap(a, b);
        parent[static_cast<std::size_t>(a)] += parent[static_cast<std::size_t>(b)];
        parent[static_cast<std::size_t>(b)] = a;
    }
    std::vector<int> component(n, -1), component_degrees;
    std::vector<std::vector<int>> endpoints;
    for (std::size_t vertex = 0; vertex < n; ++vertex) {
        const auto root = static_cast<std::size_t>(find(static_cast<int>(vertex)));
        if (component[root] == -1) {
            component[root] = static_cast<int>(component_degrees.size());
            component_degrees.push_back(0); endpoints.emplace_back();
        }
        const auto index = static_cast<std::size_t>(component[root]);
        // 有効な残余次数の総和は2*(成分数-1)。intへの加算前に上限を確認する。
        if (residual[vertex] > std::numeric_limits<int>::max() - component_degrees[index])
            return std::nullopt;
        component_degrees[index] += residual[vertex];
        endpoints[index].insert(endpoints[index].end(), static_cast<std::size_t>(residual[vertex]),
                                static_cast<int>(vertex));
    }
    auto result = tree_from_degrees(component_degrees);
    if (!result) return std::nullopt;
    for (auto& [u, v] : *result) {
        auto& a = endpoints[static_cast<std::size_t>(u)];
        auto& b = endpoints[static_cast<std::size_t>(v)];
        u = a.back(); a.pop_back(); v = b.back(); b.pop_back();
    }
    return result;
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
