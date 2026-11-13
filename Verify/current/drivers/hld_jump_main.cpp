// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>
#include <cstdint>
int main(){
 int n{},q{};verify_in>>n>>q;Graph graph(n);
 std::vector<int> ends(2*(n-1)),degree(n);
 for(int i=0;i+1<n;++i){verify_in>>ends[2*i]>>ends[2*i+1];++degree[ends[2*i]];++degree[ends[2*i+1]];}
 for(int v=0;v<n;++v)graph[v].reserve(degree[v]);
 for(int i=0;i+1<n;++i){graph[ends[2*i]].push_back(ends[2*i+1]);graph[ends[2*i+1]].push_back(ends[2*i]);}
 HeavyLightDecomposition hld(graph);
 while(q--){int u{},v{};std::int64_t k{};verify_in>>u>>v>>k;verify_out<<hld.jump(u,v,k)<<'\n';}
}
