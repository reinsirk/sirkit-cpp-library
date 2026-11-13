// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
struct Affine {std::uint32_t a,b;};
struct Compose {
    static constexpr std::uint64_t mod=998244353;
    Affine operator()(const Affine&first,const Affine&second)const noexcept{
        return {static_cast<std::uint32_t>(std::uint64_t{first.a}*second.a%mod),static_cast<std::uint32_t>((std::uint64_t{first.b}*second.a+second.b)%mod)};
    }
};
int main(){
    int n{},q{};verify_in>>n>>q;
    std::vector<Affine> values(n);for(auto& value:values)verify_in>>value.a>>value.b;
    LinkCutTree<Affine,Compose> tree(values,Affine{1,0});
    for(int i=1;i<n;++i){int u{},v{};verify_in>>u>>v;tree.link(u,v);}
    while(q--){
        int type{};verify_in>>type;
        if(type==0){int u{},v{},w{},x{};verify_in>>u>>v>>w>>x;tree.cut(u,v);tree.link(w,x);}
        else if(type==1){int vertex{};Affine value{};verify_in>>vertex>>value.a>>value.b;tree.set(vertex,value);}
        else{int u{},v{};std::uint64_t x{};verify_in>>u>>v>>x;const auto result=*tree.path_prod(u,v);verify_out<<(std::uint64_t{result.a}*x+result.b)%Compose::mod<<'\n';}
    }
}
