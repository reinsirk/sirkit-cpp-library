// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
int main(){
 int cases{};verify_in>>cases;while(cases--){int n{},m{};verify_in>>n>>m;DirectedEulerianTrail graph(n);
 for(int i=0;i<m;++i){int u{},v{};verify_in>>u>>v;graph.add_edge(u,v);}auto path=graph.find();if(!path){verify_out<<"No\n";continue;}
 verify_out<<"Yes\n";for(std::size_t i=0;i<path->vertices.size();++i)verify_out<<(i?" ":"")<<path->vertices[i];verify_out<<'\n';for(std::size_t i=0;i<path->edges.size();++i)verify_out<<(i?" ":"")<<path->edges[i];verify_out<<'\n';}
}
