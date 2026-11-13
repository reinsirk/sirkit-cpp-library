// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <vector>
int main(){
 int n{},m{};verify_in>>n>>m;UndirectedLowLink graph(n);
 std::vector<unsigned char> touched(n);
 for(int i=0;i<m;++i){int u{},v{};verify_in>>u>>v;graph.add_edge(u,v);touched[u]=touched[v]=1;}graph.build(true);
 const auto& blocks=graph.biconnected_edge_components();int isolated=0;for(int v=0;v<n;++v)isolated+=!touched[v];
 verify_out<<blocks.size()+isolated<<'\n';
 std::vector<int> last(n,-1),members;members.reserve(n);
 for(std::size_t b=0;b<blocks.size();++b){const int id=static_cast<int>(b);members.clear();
  for(const auto e:blocks[b]){const auto [u,v]=graph.edge(e);if(last[u]!=id){last[u]=id;members.push_back(u);}if(last[v]!=id){last[v]=id;members.push_back(v);}}
  verify_out<<members.size();for(int v:members)verify_out<<' '<<v;verify_out<<'\n';}
 for(int v=0;v<n;++v)if(!touched[v])verify_out<<"1 "<<v<<'\n';
}
