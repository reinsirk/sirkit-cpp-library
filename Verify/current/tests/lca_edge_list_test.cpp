// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_LCA_REQUIRE_EXTRA
#define SIRKIT_LCA_REQUIRE_EXTRA
#endif
#ifndef SIRKIT_LCA_SOURCE
#define SIRKIT_LCA_SOURCE "../../../Graph/LCA.cpp"
#endif
#include SIRKIT_LCA_SOURCE
#include SIRKIT_LCA_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef SIRKIT_LCA_REQUIRE_EXTRA
static_assert(requires(const LCA& tree) { tree.subtree_size(0); tree.subtree_range(0); tree.order_index(0); });
static_assert(std::is_same_v<decltype(std::declval<const LCA&>().subtree_range(0)),std::pair<int,int>>);
#endif
static std::uint64_t checks = 0;
static void require(bool ok) { ++checks; if (!ok) { std::cerr << "LCA regression failed: check=" << checks << '\n'; std::abort(); } }
using Graph = std::vector<std::vector<int>>;
static std::vector<std::pair<int,int>> edge_list(const Graph& g) {
 std::vector<std::pair<int,int>> result;
 for(int v=0;v<static_cast<int>(g.size());++v)for(int u:g[v])if(v<u)result.emplace_back(v,u);
 return result;
}

static void edge(Graph& g, int u, int v) { g[u].push_back(v); g[v].push_back(u); }
struct Reference {
    std::vector<int> parent, depth, root, component;
    int components = 0;
    Reference(const Graph& g, int requested) : parent(g.size(),-2), depth(g.size()), root(g.size()), component(g.size()) {
        const int n = static_cast<int>(g.size());
        for (int next = -1; next < n; ++next) {
            const int r = next < 0 ? requested : next;
            if (n == 0 || parent[r] != -2) continue;
            std::queue<int> q; q.push(r); parent[r] = -1; root[r] = r; component[r] = components;
            while (!q.empty()) {
                int v = q.front(); q.pop();
                for (int to : g[v]) if (parent[to] == -2) {
                    parent[to] = v; depth[to] = depth[v] + 1; root[to] = r; component[to] = components; q.push(to);
                }
            }
            ++components;
        }
    }
    int lca(int a, int b) const {
        if (root[a] != root[b]) return -1;
        while (depth[a] > depth[b]) a = parent[a];
        while (depth[b] > depth[a]) b = parent[b];
        while (a != b) { a = parent[a]; b = parent[b]; }
        return a;
    }
    int ancestor(int v, std::uint64_t k) const {
        if (k > static_cast<std::uint64_t>(depth[v])) return -1;
        while (k--) v = parent[v];
        return v;
    }
    std::vector<int> path(int u, int v) const {
        int a = lca(u,v); if (a < 0) return {};
        std::vector<int> first, second;
        for (int x = u; x != a; x = parent[x]) first.push_back(x);
        first.push_back(a);
        for (int x = v; x != a; x = parent[x]) second.push_back(x);
        first.insert(first.end(), second.rbegin(), second.rend()); return first;
    }
};
template<class Tree> static void inspect_extra(const Tree& t, const Reference& r) {
    int n = static_cast<int>(r.parent.size());
    if constexpr (requires { t.subtree_size(0); }) {
        for (int u = 0; u < n; ++u) { int count = 0; for (int v = 0; v < n; ++v) count += r.lca(u,v) == u; require(t.subtree_size(u) == count); }
    }
    if constexpr (requires { t.subtree_range(0); t.order_index(0); }) {
        std::vector<int> used(n);
        for (int u = 0; u < n; ++u) {
            const auto [first,last] = t.subtree_range(u);
            require(0 <= first && first < last && last <= n);
            require(first == t.order_index(u)); ++used[first];
            for (int v = 0; v < n; ++v) require((first <= t.order_index(v) && t.order_index(v) < last) == (r.lca(u,v) == u));
            if constexpr (requires { t.subtree_size(0); }) require(last-first == t.subtree_size(u));
        }
        for (int x : used) require(x == 1);
    }
}
static void inspect(const LCA& t, const Graph& g, int requested, bool all_roots) {
    Reference r(g, requested); const int n = static_cast<int>(g.size());
    require(t.size() == g.size()); require(t.empty() == (n == 0)); require(t.root() == (n ? requested : -1)); require(t.component_count() == static_cast<std::size_t>(r.components));
    std::vector<Reference> reroot; for (int root = 0; root < n; ++root) reroot.emplace_back(g,root);
    inspect_extra(t,r);
    for (int u = 0; u < n; ++u) {
        require(t.parent(u) == r.parent[u]); require(t.depth_of(u) == r.depth[u]); require(t.root_of(u) == r.root[u]); require(t.component_of(u) == r.component[u]);
        for (int k = 0; k <= n+1; ++k) require(t.kth_ancestor(u,k) == r.ancestor(u,k));
        require(t.kth_ancestor(u, std::numeric_limits<std::uint64_t>::max()) == -1);
        for (int v = 0; v < n; ++v) {
            const int a = r.lca(u,v); const auto path = r.path(u,v);
            require(t.get(u,v) == a); require(t(u,v) == a); require(t.same_component(u,v) == (a >= 0)); require(t.is_ancestor(u,v) == (a == u));
            require(t.get_distance(u,v) == (path.empty() ? -1 : static_cast<int>(path.size())-1));
            for (std::size_t k = 0; k <= path.size(); ++k) require(t.kth_on_path(u,v,k) == (k < path.size() ? path[k] : -1));
            require(t.kth_on_path(u,v,std::numeric_limits<std::uint64_t>::max()) == -1);
            if (all_roots) for (int root = 0; root < n; ++root) {
                const int expected = r.root[u] == r.root[v] && r.root[u] == r.root[root] ? reroot[root].lca(u,v) : -1;
                require(t.get(u,v,root) == expected);
            } else if (n) {
                const int root = (u+v)%n;
                const int expected = r.root[u] == r.root[v] && r.root[u] == r.root[root] ? reroot[root].lca(u,v) : -1;
                require(t.get(u,v,root) == expected);
            }
        }
    }
}
static void ownership(const Graph& g, int root) {
    LCA t(static_cast<int>(g.size()),edge_list(g),root); LCA copy(t); t.clear(); require(t.empty() && t.component_count()==0 && t.root()==-1); inspect(copy,g,root,false);
    LCA moved(std::move(copy)); require(copy.empty() && copy.root()==-1 && copy.component_count()==0); inspect(moved,g,root,false);
    copy = moved; inspect(copy,g,root,false); LCA& alias = copy; copy = alias; inspect(copy,g,root,false);
    copy = std::move(alias); inspect(copy,g,root,false); copy.swap(copy); inspect(copy,g,root,false);
    LCA other; other = std::move(copy); require(copy.empty()); inspect(other,g,root,false); swap(other,t); require(other.empty()); inspect(t,g,root,false);
    t.build(0,std::span<const std::pair<int,int>>{}); require(t.empty() && t.root()==-1); t.build(static_cast<int>(g.size()),edge_list(g),root); inspect(t,g,root,false);
    for (int i = 0; i < 3; ++i) { t.clear(); t.clear(); t.build(static_cast<int>(g.size()),edge_list(g),root); }
}
template<class Exception, class F> static void throws(F f) { bool caught = false; try { f(); } catch (const Exception&) { caught = true; } require(caught); }
template<class Tree> static void invalid_extra(const Tree& t) {
    if constexpr (requires { t.subtree_size(0); }) { throws<std::out_of_range>([&]{t.subtree_size(-1);}); throws<std::out_of_range>([&]{t.subtree_size(2);}); }
    if constexpr (requires { t.subtree_range(0); }) { throws<std::out_of_range>([&]{t.subtree_range(-1);}); throws<std::out_of_range>([&]{t.subtree_range(2);}); }
    if constexpr (requires { t.order_index(0); }) { throws<std::out_of_range>([&]{t.order_index(-1);}); throws<std::out_of_range>([&]{t.order_index(2);}); }
}
static void invalid_inputs() {
#ifndef SKIP_RUNTIME_CHECKS
    const Graph valid{{1},{0}}; LCA t(valid);
    throws<std::out_of_range>([&]{ LCA x(Graph{},1); }); throws<std::out_of_range>([&]{LCA x(valid,-1);}); throws<std::out_of_range>([&]{LCA x(valid,2);});
    for (int v : {-1,2,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}) {
        throws<std::out_of_range>([&]{t.parent(v);}); throws<std::out_of_range>([&]{t.depth_of(v);}); throws<std::out_of_range>([&]{t.component_of(v);}); throws<std::out_of_range>([&]{t.root_of(v);});
        throws<std::out_of_range>([&]{t.get(v,0);}); throws<std::out_of_range>([&]{t.get(0,v);}); throws<std::out_of_range>([&]{t(0,v);}); throws<std::out_of_range>([&]{t.get(0,1,v);});
        throws<std::out_of_range>([&]{t.same_component(0,v);}); throws<std::out_of_range>([&]{t.is_ancestor(v,0);}); throws<std::out_of_range>([&]{t.get_distance(0,v);}); throws<std::out_of_range>([&]{t.kth_ancestor(v,0);}); throws<std::out_of_range>([&]{t.kth_on_path(0,v,0);});
    }
    invalid_extra(t);
    for (const Graph& bad : {Graph{{-1}},Graph{{1}},Graph{{0}},Graph{{1,1},{0,0}},Graph{{1,2},{0,2},{0,1}}}) {
        bool caught = false; try { t.build(bad); } catch (const std::out_of_range&) { caught=true; } catch(const std::invalid_argument&) { caught=true; }
        require(caught); require(t.size()==2 && t.get(0,1)==0);
    }
    LCA empty; throws<std::out_of_range>([&]{empty.get(0,0);});
#endif
}
static void csr_invalid() {
 const std::vector<std::pair<int,int>> good{{0,1}};
 LCA t(2,good); LCA copied(t);
 throws<std::length_error>([&]{t.build(-1,{});});require(t.size()==2&&t.get(0,1)==0);
#ifndef SKIP_RUNTIME_CHECKS
 throws<std::out_of_range>([&]{LCA z(0,{},1);});
 throws<std::out_of_range>([&]{LCA z(2,good,-1);});
 throws<std::out_of_range>([&]{LCA z(2,good,2);});
 for(const auto& bad:std::vector<std::vector<std::pair<int,int>>>{{{-1,0}},{{0,2}},{{0,0}},{{0,1},{1,0}}}) {
  bool caught=false;try{t.build(2,bad);}catch(const std::out_of_range&){caught=true;}catch(const std::invalid_argument&){caught=true;}
  require(caught);require(t.size()==2&&t.get(0,1)==0);require(copied.size()==2&&copied.get(0,1)==0);
 }
 const std::vector<std::pair<int,int>> cyclic{{0,1},{1,2},{2,0}};
 throws<std::invalid_argument>([&]{t.build(3,cyclic);});require(t.size()==2&&t.get(0,1)==0);
 const std::vector<std::pair<int,int>> unseen_bad{{0,1},{4,99}};
 throws<std::out_of_range>([&]{t.build(5,unseen_bad);});require(t.size()==2&&t.get(0,1)==0);
#endif
 LCA e(0,{});require(e.empty());e.build(1,{});require(e.parent(0)==-1);e.build(0,{});require(e.empty());
 LCA brace_empty({},0);require(brace_empty.empty());
}
// 大きい構築経路は全組合せを回さず、独立な親配列で少数の照会を確認する。
static void sampled_construction_paths(std::mt19937& rng) {
 for(int kind=0;kind<5;++kind) {
  const int n=kind==0?257:kind==1?258:kind==3?1025:513;
  Graph graph(n);
  for(int v=1;v<n;++v) {
   if(kind==4 && v%11==0)continue;
   const int parent=kind==2?0:kind==3&&v==(n+1)/2?0:v-1;
   edge(graph,parent,v);
  }
  auto edges=edge_list(graph);std::shuffle(edges.begin(),edges.end(),rng);
  const Reference expected(graph,0), rerooted(graph,n-1);
  const LCA old(graph,0);
  LCA bulk(n,edges,0), retained(bulk);bulk.clear();edges.clear();
  for(int v=0;v<n;++v) {
   require(retained.parent(v)==expected.parent[v]);
   require(retained.depth_of(v)==expected.depth[v]);
   require(retained.component_of(v)==expected.component[v]);
  }
  for(int query=0;query<96;++query) {
   const int u=rng()%n,v=rng()%n;
   const int ancestor=expected.lca(u,v);
   const auto path=expected.path(u,v);
   require(old.get(u,v)==ancestor);require(retained.get(u,v)==ancestor);
   require(retained.get_distance(u,v)==(path.empty()?-1:static_cast<int>(path.size())-1));
   const std::uint64_t k=rng()%(n+2);
   require(retained.kth_ancestor(u,k)==expected.ancestor(u,k));
   require(retained.kth_on_path(u,v,k)==(k<path.size()?path[k]:-1));
   const int rerooted_lca=expected.root[u]==expected.root[v] && expected.root[u]==expected.root[n-1]
       ?rerooted.lca(u,v):-1;
   require(retained.get(u,v,n-1)==rerooted_lca);
   const auto [first,last]=retained.subtree_range(u);
   require((first<=retained.order_index(v) && retained.order_index(v)<last)==(ancestor==u));
  }
 }
}
int main() {
 std::mt19937 rng(819251);
 for(int trial=0;trial<48;++trial) {
  const int n=trial%19;Graph g(n);
  for(int v=1;v<n;++v)if(trial%4!=0 && rng()%4)edge(g,rng()%v,v);
  for(auto& row:g)std::shuffle(row.begin(),row.end(),rng);
  const int root=n?static_cast<int>(rng()%n):0;
  auto edges=edge_list(g);std::shuffle(edges.begin(),edges.end(),rng);
  for(auto& e:edges)if(rng()%2)std::swap(e.first,e.second);
  LCA old(g,root),bulk(n,edges,root);inspect(old,g,root,n<=5);inspect(bulk,g,root,n<=5);
  if(trial%12==0 || n<=2)ownership(g,root);
  edges.clear();inspect(bulk,g,root,false);
 }
 sampled_construction_paths(rng);
 invalid_inputs();csr_invalid();
 std::cout << "PASS " << checks << " lightweight old/edge-list LCA API checks\n";
}
