// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <utility>
#include <vector>
int main(){
    int n,q;verify_in>>n>>q;
    std::vector<std::pair<int,int>> edges;edges.reserve(n-1);
    for(int v=1;v<n;++v){int p;verify_in>>p;edges.emplace_back(p,v);}
    LCA lca(n,edges);
    while(q--){int u,v;verify_in>>u>>v;verify_out<<lca.get(u,v)<<'\n';}
}
