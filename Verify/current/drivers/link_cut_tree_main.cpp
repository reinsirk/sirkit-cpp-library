// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
int main(){
    int n{},q{};verify_in>>n>>q;
    std::vector<std::int64_t> values(n);for(auto& value:values)verify_in>>value;
    LinkCutTree<std::int64_t> tree(values);
    for(int i=1;i<n;++i){int u{},v{};verify_in>>u>>v;tree.link(u,v);}
    while(q--){
        int type{};verify_in>>type;
        if(type==0){int u{},v{},w{},x{};verify_in>>u>>v>>w>>x;tree.cut(u,v);tree.link(w,x);}
        else if(type==1){int vertex{};std::int64_t add{};verify_in>>vertex>>add;tree.transform(vertex,[add](std::int64_t old){return old+add;});}
        else{int u{},v{};verify_in>>u>>v;verify_out<<*tree.path_prod(u,v)<<'\n';}
    }
}
