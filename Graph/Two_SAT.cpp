// SPDX-License-Identifier: CC0-1.0
// 参考（2-SATの問題分類）: https://nyaannyaan.github.io/library/math/two-sat.hpp
// 参考（API仕様）: https://atcoder.github.io/ac-library/production/document_ja/twosat.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/two_sat.md
//
// API一覧
// two_sat() / two_sat(n): 空／n個のbool変数。変数番号[0,n)。O(1)準備。n<=INT_MAX/2。
// add_clause(i,a,j,b): (x_i==a)または(x_j==b)という節を追加。償却O(1)。解を無効化する。
// imply(i,a,j,b): (x_i==a)なら(x_j==b)。force(i,a): x_i==aを強制。償却O(1)。
// add_xor(i,j,different=true): 2変数の値が異なる/等しいという制約。償却O(1)。
// reserve_clauses(m): m節分の領域を確保。size()/clause_count(): 変数数/内部2リテラル節数。O(1)。
// satisfiable(): 充足可能ならtrue。反復SCCでO(n+m)時間・追加領域。複数回実行できる。
// answer(): 最後の判定の解をvector<bool>の値で返す。O(n)時間・領域。未実行/UNSATでも長さn、内容は未規定。
// value(i): 最後のsatisfiable()成功時の変数iのbool値。O(1)。制約追加後は再判定する。
// reset(n=0): 制約を捨てn変数へ。swap(other): O(1)。コピーは独立所有、ムーブ元は空の未解決状態。
// 入力検査はSKIP_RUNTIME_CHECKSで除く。answer()の返り値は独立したコピー。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_TWO_SAT_SCC_DETAIL_DEFINED
#define SIRKIT_TWO_SAT_SCC_DETAIL_DEFINED
namespace reinsirk::detail {
// nは偶数、相補リテラルはvと(v^1)。Indexは総辺数を表現できる符号なし型。
// 矛盾を検出したら{-1,{}}、成功時はトポロジカル順の成分番号を返す。
template<class Index>
inline std::pair<int, std::vector<int>> two_sat_component_ids_index(
        int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<Index> begin(static_cast<std::size_t>(n) + 1);
    for (auto [u, v] : edges) { (void)v; ++begin[static_cast<std::size_t>(u) + 1]; }
    for (int v = 0; v < n; ++v) begin[v + 1] += begin[v];
    std::vector<Index> next(begin.begin() + 1, begin.end());
    // CSRの全要素へ書き込んでから参照するため、先行初期化は不要。
    auto destination = std::make_unique_for_overwrite<int[]>(edges.size());
    // 辺順を保って逆向きに埋め、走査位置の再初期化を省く。
    for (auto it = edges.rbegin(); it != edges.rend(); ++it)
        destination[--next[it->first]] = it->second;
    // idsは中断中のlow-linkの退避先を兼ね、確定時に成分番号を書く。
    std::vector<int> entered(n, -1), ids(n), dfs, pending;
    dfs.reserve(n); pending.reserve(n);
    int clock = 0, count = 0;
    for (int root = 0; root < n; ++root) {
        if (entered[root] != -1) continue;
        int v = root;
        Index cursor = next[v], end = begin[v + 1];
        // 出辺0の頂点は即確定し、DFS番号の割当ても省く。
        if (cursor == end) {
            entered[v] = n; ids[v] = count++;
            continue;
        }
        int low = entered[v] = clock++;
        pending.push_back(v);
        for (;;) {
            if (cursor != end) {
                const int u = destination[cursor++];
                if (entered[u] == -1) {
                    const Index child_cursor = next[u], child_end = begin[u + 1];
                    if (child_cursor == child_end) {
                        entered[u] = n; ids[u] = count++;
                        continue;
                    }
                    // 子へ降りるときだけ、親の走査位置とlow-linkを退避する。
                    next[v] = cursor;
                    ids[v] = low;
                    dfs.push_back(v);
                    v = u;
                    low = entered[v] = clock++;
                    cursor = child_cursor; end = child_end;
                    pending.push_back(v);
                } else low = std::min(low, entered[u]);
                continue;
            }
            // 成分番号への上書き後も親へ渡せるよう、low-linkを退避する。
            const int child_low = low;
            if (child_low == entered[v]) {
                // 1頂点の成分には相補的な2頂点が同居しない。
                if (pending.back() == v) {
                    pending.pop_back();
                    entered[v] = n; ids[v] = count;
                } else {
                    for (;;) {
                        const int u = pending.back(); pending.pop_back();
                        entered[u] = n; ids[u] = count;
                        // 確定済みかつ今回の成分番号なら、同じ成分内の矛盾。
                        if (entered[u ^ 1] == n && ids[u ^ 1] == count)
                            return {-1, {}};
                        if (u == v) break;
                    }
                }
                ++count;
            }
            if (dfs.empty()) break;
            v = dfs.back(); dfs.pop_back();
            low = std::min(ids[v], child_low);
            cursor = next[v]; end = begin[v + 1];
        }
    }
    for (auto& id : ids) id = count - 1 - id;
    return {count, std::move(ids)};
}
inline std::pair<int, std::vector<int>> two_sat_component_ids(
        int n, const std::vector<std::pair<int, int>>& edges) {
    // 無辺では各頂点が1成分。従来と同じ逆番号で返す。
    if (edges.empty()) {
        std::vector<int> ids(n);
        for (int v = 0; v < n; ++v) ids[v] = n - 1 - v;
        return {n, std::move(ids)};
    }
    // 辺数が収まる場合だけCSRの添字を32bitにする。
    if (edges.size() <= std::numeric_limits<std::uint32_t>::max())
        return two_sat_component_ids_index<std::uint32_t>(n, edges);
    return two_sat_component_ids_index<std::size_t>(n, edges);
}
}
#endif

#ifndef SIRKIT_TWO_SAT_DEFINED
#define SIRKIT_TWO_SAT_DEFINED
class two_sat {
    int n_ = 0;
    std::vector<std::pair<int, int>> implications_;
    std::vector<unsigned char> answer_;
#ifndef SKIP_RUNTIME_CHECKS
    bool solved_ = false, satisfiable_ = false;
    void variable(int i) const {
        if (i < 0 || i >= n_) throw std::out_of_range("two_sat: variable out of range");
    }
    void require_answer() const {
        if (!solved_ || !satisfiable_) throw std::logic_error("two_sat: no current satisfying assignment");
    }
#endif
    void room(std::size_t extra) {
        const auto maximum = implications_.max_size(), current = implications_.size();
        if (extra > maximum - current) throw std::length_error("two_sat: too many clauses");
        if (implications_.capacity() - current >= extra) return;
        const auto capacity = implications_.capacity();
        const auto grown = capacity <= maximum / 2 ? capacity * 2 : maximum;
        implications_.reserve(std::max(current + extra, std::max(std::min(std::size_t{4}, maximum), grown)));
    }
    void invalidate() noexcept {
#ifndef SKIP_RUNTIME_CHECKS
        solved_ = satisfiable_ = false;
#endif
    }
public:
    two_sat() = default;
    explicit two_sat(int n) : n_(n) {
        if (n < 0 || n > std::numeric_limits<int>::max() / 2) throw std::length_error("two_sat: invalid variable count");
    }
    two_sat(const two_sat&) = default;
    two_sat(two_sat&& other) noexcept { swap(other); }
    two_sat& operator=(two_sat other) noexcept { swap(other); return *this; }
    void swap(two_sat& other) noexcept {
        std::swap(n_, other.n_); implications_.swap(other.implications_); answer_.swap(other.answer_);
#ifndef SKIP_RUNTIME_CHECKS
        std::swap(solved_, other.solved_); std::swap(satisfiable_, other.satisfiable_);
#endif
    }
    void reset(int n = 0) { two_sat replacement(n); swap(replacement); }
    int size() const noexcept { return n_; }
    std::size_t clause_count() const noexcept { return implications_.size() / 2; }
    void reserve_clauses(std::size_t count) {
        if (count > implications_.max_size() / 2) throw std::length_error("two_sat: too many clauses");
        implications_.reserve(count * 2);
    }
    void add_clause(int i, bool a, int j, bool b) {
#ifndef SKIP_RUNTIME_CHECKS
        variable(i); variable(j);
#endif
        room(2);
        implications_.emplace_back(2 * i + !a, 2 * j + b);
        implications_.emplace_back(2 * j + !b, 2 * i + a);
        invalidate();
    }
    void imply(int i, bool a, int j, bool b) { add_clause(i, !a, j, b); }
    void force(int i, bool a) { add_clause(i, a, i, a); }
    void add_xor(int i, int j, bool different = true) {
#ifndef SKIP_RUNTIME_CHECKS
        variable(i); variable(j);
#endif
        room(4);
        add_clause(i, true, j, different);
        add_clause(i, false, j, !different);
    }
    bool satisfiable() {
        if (implications_.empty()) {
            // 制約なしで保持済みの解は全false。再判定ではそのまま使える。
            if (answer_.size() != static_cast<std::size_t>(n_)) {
                std::vector<unsigned char> result(n_, 0);
                answer_.swap(result);
            }
#ifndef SKIP_RUNTIME_CHECKS
            solved_ = satisfiable_ = true;
#endif
            return true;
        }
        const auto components = reinsirk::detail::two_sat_component_ids(2 * n_, implications_);
        if (components.first < 0) {
            answer_.clear();
#ifndef SKIP_RUNTIME_CHECKS
            solved_ = true; satisfiable_ = false;
#endif
            return false;
        }
        const auto& ids = components.second;
        std::vector<unsigned char> result(n_);
        for (int i = 0; i < n_; ++i) result[i] = ids[2 * i] < ids[2 * i + 1];
        answer_.swap(result);
#ifndef SKIP_RUNTIME_CHECKS
        solved_ = satisfiable_ = true;
#endif
        return true;
    }
    std::vector<bool> answer() {
#if defined(__GNUC__) && !defined(__clang__)
        if (answer_.size() == static_cast<std::size_t>(n_))
            return std::vector<bool>(answer_.begin(), answer_.end());
        return std::vector<bool>(static_cast<std::size_t>(n_));
#else
        std::vector<bool> result(static_cast<std::size_t>(n_));
        if (answer_.size() == result.size())
            for (int i = 0; i < n_; ++i) result[i] = answer_[i] != 0;
        return result;
#endif
    }
    bool value(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        require_answer(); variable(i);
#endif
        return answer_[i] != 0;
    }
};
#endif
