// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
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
