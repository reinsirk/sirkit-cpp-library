// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
int main(){
 int n{},q{};verify_in>>n>>q;std::vector<std::int64_t> value(n);for(auto& x:value)verify_in>>x;Graph graph(n);
 std::vector<int> ends(2*(n-1)),degree(n);
 for(int i=0;i+1<n;++i){verify_in>>ends[2*i]>>ends[2*i+1];++degree[ends[2*i]];++degree[ends[2*i+1]];}
 for(int v=0;v<n;++v)graph[v].reserve(degree[v]);
 for(int i=0;i+1<n;++i){graph[ends[2*i]].push_back(ends[2*i+1]);graph[ends[2*i+1]].push_back(ends[2*i]);}
 auto hld=HeavyLightDecomposition(graph).with_values<fenwick_tree<std::int64_t>>(value);
 while(q--){int type{};verify_in>>type;if(type==0){int v{};std::int64_t delta{};verify_in>>v>>delta;hld.add(v,delta);}else{int u{},v{};verify_in>>u>>v;verify_out<<hld.path_prod(u,v)<<'\n';}}
}
