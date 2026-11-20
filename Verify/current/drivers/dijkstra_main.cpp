// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
int main() {
    int n{},m{},s{},t{}; if (!(verify_in>>n>>m>>s>>t)) return 0;
    std::vector<std::tuple<int,int,std::int64_t>> edges(m);
    for(auto& [u,v,w]:edges)verify_in>>u>>v>>w;
    std::vector<std::int64_t> distance; std::vector<int> path;
    Dijkstra(n,edges,distance,s,t,path);
    if (distance[t]==kInf) { verify_out<<"-1\n"; return 0; }
    verify_out<<distance[t]<<' '<<path.size()-1<<'\n';
    for (std::size_t i=1;i<path.size();++i) verify_out<<path[i-1]<<' '<<path[i]<<'\n';
}
