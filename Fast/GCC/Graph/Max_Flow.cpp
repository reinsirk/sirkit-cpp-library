// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Max_Flow.cpp
// 参考（最大流の理論・ACLのAPI）: https://atcoder.github.io/ac-library/production/document_ja/maxflow.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/max_flow.md
//
// API一覧
// mf_graph<Cap=std::int64_t>()/mf_graph(n): 空/n頂点の有向グラフ。Capはbool以外の整数型。O(1)/O(n)。
// ACLと同名・同じ呼び出し形式。自己辺・多重辺・逆向きの元辺を追加できる。
// add_edge(from,to,cap): 非負容量の元辺を追加し、追加順のint番号を返す。償却O(1)。
// edgeは{int from,to;Cap cap,flow;}。get_edge(int id)/edges(): 元辺のコピー/追加順の一覧。O(1)/O(m)。
// flow(s,t)/flow(s,t,flow_limit): 型の最大値/非負の指定上限まで追加で流し、追加量を返す。O(n²m)、作業領域O(n)。
// 同じ始点・終点で継続可能。総量がCap上限を越す場合は分けて呼び、累積値を十分広い型で保持する。
// min_cut(s): 残余グラフでsから到達可能な頂点のbool配列。O(n+m)時間・O(n)領域。最後まで流した後は最小カット。
// get_edge/edges/min_cutはACL形式の非const版に加えて、constオブジェクトからも同名で呼べる。
// change_edge(int id,new_cap,new_flow): 指定辺を0<=new_flow<=new_capへ変更。O(1)。流量保存は利用側で保証する。
// 追加機能: size()/edge_count()は頂点数int/元辺数size_tをO(1)で返す。reserve_edges(m)は位置表を予約。
// コピー構築/代入: 残余を独立所有しO(n+m)。移動構築はO(1)、移動元は空。代入は自身への参照を返す。
// reset(n=0): O(n+旧領域の破棄)で空のn頂点へ戻す。swap(other)はO(1)。代入にも旧領域の破棄費用が加わる。
// ACLのs!=tという制約外では、s==tを追加仕様として0にする。flow_limit==0も0。
// 反復Dinic法で深い経路も再帰しない。入力検査はSKIP_RUNTIME_CHECKSで除くが、内部添字上限は維持。
// get/changeのidは[0,edge_count())。辺数はINT_MAX以下。継続flowで異なる端点を使う場合も流量保存が前提。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SIRKIT_MAX_FLOW_DEFINED
#define SIRKIT_MAX_FLOW_DEFINED

template<class Cap = std::int64_t>
class mf_graph {
    static_assert(std::is_integral_v<Cap> && !std::is_same_v<Cap, bool>, "capacity must be a non-bool integer");
public:
    struct edge { int from; int to; Cap cap; Cap flow; };
private:
    struct Arc { int to; int reverse; Cap residual; };
    struct Frame { int vertex; Cap limit; Cap sent; };
    std::vector<std::vector<Arc>> graph_;
    std::vector<std::pair<int, int>> positions_;
    std::vector<int> level_, next_, queue_;
    std::vector<Frame> frames_;
    static std::size_t checked_size(int n) {
        if (n < 0) throw std::length_error("mf_graph: negative vertex count");
        return static_cast<std::size_t>(n);
    }
    bool levels(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        int front = 0, end = 0; queue_[end++] = source; level_[source] = 0;
        while (front != end) {
            const int v = queue_[front++];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && level_[edge.to] == -1) {
                level_[edge.to] = level_[v] + 1;
                // sinkの直前層までは発見済みなので、最短路の辺はDFSで列挙できる。
                if (edge.to == sink) return true;
                queue_[end++] = edge.to;
            }
        }
        return level_[sink] != -1;
    }
    Cap blocking_flow(int source, int sink, Cap limit) {
        frames_.clear(); frames_.push_back({source, limit, 0});
        while (!frames_.empty()) {
            auto& frame = frames_.back(); const int v = frame.vertex;
            if (v != sink && frame.sent != frame.limit) {
                auto& cursor = next_[v];
                while (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    if (edge.residual != 0 && level_[edge.to] == level_[v] + 1) break;
                    ++cursor;
                }
                if (cursor != static_cast<int>(graph_[v].size())) {
                    const auto& edge = graph_[v][cursor];
                    frames_.push_back({edge.to, std::min(static_cast<Cap>(frame.limit - frame.sent), edge.residual), 0});
                    continue;
                }
                level_[v] = -1;
            }
            const Cap sent = v == sink ? frame.limit : frame.sent;
            frames_.pop_back();
            if (frames_.empty()) return sent;
            auto& parent = frames_.back(); auto& edge = graph_[parent.vertex][next_[parent.vertex]];
            edge.residual -= sent; graph_[edge.to][edge.reverse].residual += sent;
            parent.sent += sent;
            if (edge.residual == 0 || level_[edge.to] == -1) ++next_[parent.vertex];
        }
        return 0;
    }
public:
    mf_graph() = default;
    explicit mf_graph(int n) : graph_(checked_size(n)), level_(graph_.size()), next_(graph_.size()), queue_(graph_.size()) {}
    mf_graph(const mf_graph&) = default;
    mf_graph(mf_graph&& other) noexcept { swap(other); }
    mf_graph& operator=(mf_graph other) noexcept { swap(other); return *this; }
    void swap(mf_graph& other) noexcept {
        graph_.swap(other.graph_); positions_.swap(other.positions_); level_.swap(other.level_);
        next_.swap(other.next_); queue_.swap(other.queue_); frames_.swap(other.frames_);
    }
    void reset(int n = 0) { mf_graph replacement(n); swap(replacement); }
    int size() const noexcept { return static_cast<int>(graph_.size()); }
    std::size_t edge_count() const noexcept { return positions_.size(); }
    void reserve_edges(std::size_t count) {
        if (count > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("mf_graph: too many edges");
        positions_.reserve(count);
    }
    int add_edge(int from, int to, Cap capacity) {
        constexpr auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
        if (positions_.size() >= maximum) throw std::length_error("mf_graph: too many edges");
        if (graph_[from].size() >= maximum || graph_[to].size() >= maximum ||
            (from == to && graph_[from].size() >= maximum - 1)) throw std::length_error("mf_graph: too many incident edges");
        const int forward = static_cast<int>(graph_[from].size());
        const int backward = static_cast<int>(graph_[to].size()) + static_cast<int>(from == to);
        graph_[from].push_back({to, backward, capacity});
        try { graph_[to].push_back({from, forward, 0}); }
        catch (...) { graph_[from].pop_back(); throw; }
        try { positions_.emplace_back(from, forward); }
        catch (...) { graph_[to].pop_back(); graph_[from].pop_back(); throw; }
        return static_cast<int>(positions_.size()) - 1;
    }
    edge get_edge(int id) { return std::as_const(*this).get_edge(id); }
    edge get_edge(int id) const {
        const auto [from, position] = positions_[id]; const auto& edge = graph_[from][position];
        const Cap sent = graph_[edge.to][edge.reverse].residual;
        return {from, edge.to, static_cast<Cap>(edge.residual + sent), sent};
    }
    std::vector<edge> edges() { return std::as_const(*this).edges(); }
    std::vector<edge> edges() const {
        std::vector<edge> result; result.reserve(positions_.size());
        for (int id = 0; id < static_cast<int>(positions_.size()); ++id) result.push_back(get_edge(id));
        return result;
    }
    void change_edge(int id, Cap capacity, Cap sent) {
        const auto [from, position] = positions_[id]; auto& edge = graph_[from][position];
        edge.residual = capacity - sent; graph_[edge.to][edge.reverse].residual = sent;
    }
    Cap flow(int source, int sink) { return flow(source, sink, std::numeric_limits<Cap>::max()); }
    Cap flow(int source, int sink, Cap limit) {
        if (source == sink || limit == 0) return 0;
        frames_.reserve(graph_.size());
        Cap result = 0;
        while (result != limit && levels(source, sink)) {
            std::fill(next_.begin(), next_.end(), 0);
            result += blocking_flow(source, sink, limit - result);
        }
        return result;
    }
    std::vector<bool> min_cut(int source) { return std::as_const(*this).min_cut(source); }
    std::vector<bool> min_cut(int source) const {
        std::vector<bool> reachable(graph_.size()); std::vector<int> queue;
        queue.reserve(graph_.size()); queue.push_back(source); reachable[source] = true;
        for (std::size_t front = 0; front < queue.size(); ++front) {
            const int v = queue[front];
            for (const auto& edge : graph_[v]) if (edge.residual != 0 && !reachable[edge.to]) {
                reachable[edge.to] = true; queue.push_back(edge.to);
            }
        }
        return reachable;
    }
};
#endif
