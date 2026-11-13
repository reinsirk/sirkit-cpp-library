// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
inline constexpr std::uint64_t modulus=998244353;
struct AffineAction {std::uint32_t a,b;};
struct AffineMapping {std::uint32_t operator()(const AffineAction&f,std::uint32_t x)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{f.a}*x+f.b)%modulus);}};
struct AffineComposition {AffineAction operator()(const AffineAction&f,const AffineAction&g)const noexcept{return {static_cast<std::uint32_t>(std::uint64_t{f.a}*g.a%modulus),static_cast<std::uint32_t>((std::uint64_t{f.a}*g.b+f.b)%modulus)};}};
int main(){
 int n{},q{};verify_in>>n>>q;std::vector<std::uint32_t> values(n);for(auto&x:values)verify_in>>x;
 DualSegmentTree<std::uint32_t,AffineAction,AffineMapping,AffineComposition> tree(values,{},{});
 while(q--){int type{},l{};verify_in>>type>>l;if(type==0){int r{};AffineAction f{};verify_in>>r>>f.a>>f.b;tree.apply(l,r,f);}else verify_out<<tree.get(l)<<'\n';}
}
