// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
inline constexpr std::uint64_t modulus=998244353;
struct AddMod {std::uint32_t operator()(std::uint32_t a,std::uint32_t b)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{a}+b)%modulus);}};
struct AffineAction {std::uint32_t a,b;};
struct AffineMapping {std::uint32_t operator()(const AffineAction&f,std::uint32_t value,std::size_t n)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{f.a}*value+std::uint64_t{f.b}*(n%modulus))%modulus);}};
struct AffineComposition {AffineAction operator()(const AffineAction&f,const AffineAction&g)const noexcept{return {static_cast<std::uint32_t>(std::uint64_t{f.a}*g.a%modulus),static_cast<std::uint32_t>((std::uint64_t{f.a}*g.b+f.b)%modulus)};}};
int main(){
    int n{},q{};verify_in>>n>>q;
    std::vector<std::uint32_t> values(n);for(auto& value:values)verify_in>>value;
    LinkCutTree<std::uint32_t,AddMod,AffineAction,AffineMapping,AffineComposition> tree(values,0);
    for(int i=1;i<n;++i)tree.link(i-1,i);
    while(q--){
        int type{},left{},right{};verify_in>>type>>left>>right;
        if(type==0){AffineAction f{};verify_in>>f.a>>f.b;tree.apply(left,right-1,f);}
        else verify_out<<*tree.path_prod(left,right-1)<<'\n';
    }
}
