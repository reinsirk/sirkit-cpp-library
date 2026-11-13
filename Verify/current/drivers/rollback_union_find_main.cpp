// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <array>
#include <vector>
int main() {
    int n,q;verify_in>>n>>q;
    std::vector<std::array<int,3>> queries(q+1);
    std::vector<int> head(q+1,-1), next(q+1);
    for(int i=1;i<=q;++i){int k;auto& a=queries[i];verify_in>>a[0]>>k>>a[1]>>a[2];next[i]=head[k+1];head[k+1]=i;}
    RollbackUnionFind uf(n);
    std::vector<int> answers(q+1,-1);
    struct Frame{int vertex;bool exit;};std::vector<Frame> stack{{0,false}};
    while(!stack.empty()){
        auto [i,exit]=stack.back();stack.pop_back();
        if(exit){uf.undo();continue;}
        if(i!=0){const auto& a=queries[i];
            if(a[0]!=0)answers[i]=uf.same(a[1],a[2]);
            else if(head[i]!=-1){uf.unite(a[1],a[2]);stack.push_back({i,true});}
            else continue;}
        for(int child=head[i];child!=-1;child=next[child])stack.push_back({child,false});
    }
    for(int i=1;i<=q;++i)if(answers[i]!=-1)verify_out<<answers[i]<<'\n';
}
