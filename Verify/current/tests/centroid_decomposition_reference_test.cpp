// SPDX-License-Identifier: CC0-1.0
#ifndef CENTROID_DECOMPOSITION_SOURCE
#define CENTROID_DECOMPOSITION_SOURCE "../../../Graph/Centroid_Decomposition.cpp"
#endif
#include CENTROID_DECOMPOSITION_SOURCE
#include CENTROID_DECOMPOSITION_SOURCE
#include <cassert>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <type_traits>

std::mt19937_64 generator(0xc314d17bd1);
int random_int(int n) { return static_cast<int>(generator() % static_cast<unsigned>(n)); }
template<class F> void must_throw(F f) {
    bool caught = false; try { f(); } catch(const std::exception&) { caught = true; } assert(caught);
}
std::vector<std::vector<int>> distances(const Graph& graph) {
    const auto n = graph.size(); std::vector<std::vector<int>> result(n,std::vector<int>(n,-1));
    for(std::size_t start=0;start<n;++start){std::vector<int>queue{static_cast<int>(start)};result[start][start]=0;for(std::size_t i=0;i<queue.size();++i){int v=queue[i];for(int to:graph[static_cast<std::size_t>(v)])if(result[start][static_cast<std::size_t>(to)]==-1){result[start][static_cast<std::size_t>(to)]=result[start][static_cast<std::size_t>(v)]+1;queue.push_back(to);}}}return result;
}
void validate(const Graph& graph) {
    auto saved = graph; const auto n=graph.size(); auto d=distances(graph);
    CentroidDecomposition cd(graph); assert(graph==saved);assert(cd.size()==n&&cd.empty()==(n==0)&&cd.stores_distances());
    CentroidDecomposition light(graph,false);assert(light.parents()==cd.parents()&&light.depths()==cd.depths()&&light.order()==cd.order());
    if(n==0){assert(cd.root()==-1&&cd.centroid_tree().empty());return;}
    int root=cd.root();assert(root>=0&&static_cast<std::size_t>(root)<n);assert(cd.parent(root)==-1&&cd.depth(root)==0&&cd.component_size(root)==static_cast<int>(n));
    std::vector<int>position(n),subtree(n,1);assert(cd.order().size()==n);
    for(std::size_t i=0;i<n;++i){int v=cd.order()[i];assert(v>=0&&static_cast<std::size_t>(v)<n);position[static_cast<std::size_t>(v)]=static_cast<int>(i);}
    auto sorted=cd.order();std::sort(sorted.begin(),sorted.end());for(std::size_t i=0;i<n;++i)assert(sorted[i]==static_cast<int>(i));
    for(auto it=cd.order().rbegin();it!=cd.order().rend();++it){int v=*it,p=cd.parent(v);if(p!=-1){assert(position[static_cast<std::size_t>(p)]<position[static_cast<std::size_t>(v)]);assert(cd.depth(v)==cd.depth(p)+1);assert(cd.component_size(v)<=cd.component_size(p)/2);subtree[static_cast<std::size_t>(p)]+=subtree[static_cast<std::size_t>(v)];}assert(cd.component_size(v)==subtree[static_cast<std::size_t>(v)]);}
    auto tree=cd.centroid_tree();std::size_t entries=0;for(std::size_t v=0;v<n;++v){entries+=tree[v].size();for(int to:tree[v])assert(cd.parent(static_cast<int>(v))==to||cd.parent(to)==static_cast<int>(v));}assert(entries==2*(n-1));
    for(std::size_t v=0;v<n;++v){std::vector<int>path;for(int u=static_cast<int>(v);u!=-1;u=cd.parent(u))path.push_back(u);std::reverse(path.begin(),path.end());const auto& chain=cd.ancestors(static_cast<int>(v));assert(chain.size()==path.size()&&chain.size()==static_cast<std::size_t>(cd.depth(static_cast<int>(v))+1));for(std::size_t k=0;k<path.size();++k){assert(chain[k].vertex==path[k]);assert(chain[k].distance==d[v][static_cast<std::size_t>(path[k])]);}assert(chain.back().distance==0);}
    for(std::size_t u=0;u<n;++u)for(std::size_t v=0;v<n;++v){assert(cd.distance(static_cast<int>(u),static_cast<int>(v))==d[u][v]);int a=static_cast<int>(u),b=static_cast<int>(v);while(cd.depth(a)>cd.depth(b))a=cd.parent(a);while(cd.depth(b)>cd.depth(a))b=cd.parent(b);while(a!=b){a=cd.parent(a);b=cd.parent(b);}assert(d[u][static_cast<std::size_t>(a)]+d[static_cast<std::size_t>(a)][v]==d[u][v]);}
    std::vector<std::int64_t>best(n,1000000000);std::vector<int>marked;
    for(int repeat=0;repeat<15;++repeat){int v=random_int(static_cast<int>(n));marked.push_back(v);for(auto a:cd.ancestors(v))best[static_cast<std::size_t>(a.vertex)]=std::min(best[static_cast<std::size_t>(a.vertex)],std::int64_t{a.distance});for(std::size_t u=0;u<n;++u){int want=1000000;for(int m:marked)want=std::min(want,d[u][static_cast<std::size_t>(m)]);std::int64_t actual=1000000000;for(auto a:cd.ancestors(static_cast<int>(u)))actual=std::min(actual,best[static_cast<std::size_t>(a.vertex)]+a.distance);assert(actual==want);}}
    auto copied=cd;auto parents=cd.parents();copied.clear();assert(cd.parents()==parents&&copied.empty());copied=cd;assert(copied.parents()==parents);auto moved=std::move(copied);assert(copied.empty()&&copied.root()==-1&&moved.parents()==parents);copied.build(graph);assert(copied.parents()==parents);moved=std::move(copied);assert(copied.empty()&&moved.parents()==parents);swap(moved,copied);assert(moved.empty()&&copied.parents()==parents);
    Graph changed=graph;{CentroidDecomposition owned(changed);changed.clear();assert(owned.parents()==parents);}
#ifndef SKIP_RUNTIME_CHECKS
    must_throw([&]{cd.parent(-1);});must_throw([&]{cd.ancestors(static_cast<int>(n));});must_throw([&]{light.distance(0,0);});
    Graph invalid{{1},{0,2},{1,0}};must_throw([&]{cd.build(invalid);});assert(cd.parents()==parents);
#endif
}
Graph heap_decode(int n,std::vector<int> code){Graph graph(static_cast<std::size_t>(n));if(n==1)return graph;std::vector<int>deg(static_cast<std::size_t>(n),1);for(int x:code)++deg[static_cast<std::size_t>(x)];std::priority_queue<int,std::vector<int>,std::greater<int>>q;for(int i=0;i<n;++i)if(deg[static_cast<std::size_t>(i)]==1)q.push(i);for(int x:code){int leaf=q.top();q.pop();graph[static_cast<std::size_t>(leaf)].push_back(x);graph[static_cast<std::size_t>(x)].push_back(leaf);if(--deg[static_cast<std::size_t>(x)]==1)q.push(x);}int a=q.top();q.pop();int b=q.top();graph[static_cast<std::size_t>(a)].push_back(b);graph[static_cast<std::size_t>(b)].push_back(a);return graph;}
int main(){
    static_assert(std::is_nothrow_move_constructible_v<CentroidDecomposition>);
    static_assert(std::is_nothrow_move_assignable_v<CentroidDecomposition>);
    validate({});
    for(int n=1;n<=6;++n){std::size_t count=1;for(int i=0;i<n-2;++i)count*=static_cast<std::size_t>(n);for(std::size_t mask=0;mask<count;++mask){auto x=mask;std::vector<int>code(static_cast<std::size_t>(std::max(0,n-2)));for(auto&v:code){v=static_cast<int>(x%static_cast<std::size_t>(n));x/=static_cast<std::size_t>(n);}validate(heap_decode(n,code));}}
    for(int repeat=0;repeat<350;++repeat){int n=random_int(100)+1;Graph graph(static_cast<std::size_t>(n));std::vector<int>label(static_cast<std::size_t>(n));std::iota(label.begin(),label.end(),0);std::shuffle(label.begin(),label.end(),generator);for(int v=1;v<n;++v){int p=random_int(v);graph[static_cast<std::size_t>(label[static_cast<std::size_t>(v)])].push_back(label[static_cast<std::size_t>(p)]);graph[static_cast<std::size_t>(label[static_cast<std::size_t>(p)])].push_back(label[static_cast<std::size_t>(v)]);}for(auto&adj:graph)std::shuffle(adj.begin(),adj.end(),generator);validate(graph);}
#ifndef SKIP_RUNTIME_CHECKS
    for(const Graph& invalid:{Graph{{0}},Graph{{1},{}},Graph{{1,1},{0,0}},Graph{{1},{0},{}},Graph{{1,2},{0,2},{0,1}},Graph{{-1}}})must_throw([&]{CentroidDecomposition cd(invalid);});
#endif
    for(bool chain:{false,true})for(bool store:{false,true}){int n=store?50000:250000;Graph graph(static_cast<std::size_t>(n));for(int v=1;v<n;++v){int p=chain?v-1:0;graph[static_cast<std::size_t>(v)].push_back(p);graph[static_cast<std::size_t>(p)].push_back(v);}CentroidDecomposition cd(graph,store);assert(cd.order().size()==static_cast<std::size_t>(n));if(store){assert(cd.distance(0,n-1)==(chain?n-1:1));assert(cd.distance(n-1,n-2)==(chain?1:2));}}
    std::cout<<"all centroid checks passed\n";
}
