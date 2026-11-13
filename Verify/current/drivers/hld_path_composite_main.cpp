// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstdint>
#include <vector>
namespace {
constexpr std::uint64_t modulus=998244353;
struct Affine{std::uint32_t a,b;};
Affine append_affine(Affine first,Affine second){return {
 static_cast<std::uint32_t>(std::uint64_t(second.a)*first.a%modulus),
 static_cast<std::uint32_t>((std::uint64_t(second.a)*first.b+second.b)%modulus)};}
Affine identity_affine(){return {1,0};}
}
int main(){
 int n{},q{};verify_in>>n>>q;std::vector<Affine> value(n);for(auto& f:value)verify_in>>f.a>>f.b;Graph graph(n);
 std::vector<int> ends(2*(n-1)),degree(n);
 for(int i=0;i+1<n;++i){verify_in>>ends[2*i]>>ends[2*i+1];++degree[ends[2*i]];++degree[ends[2*i+1]];}
 for(int v=0;v<n;++v)graph[v].reserve(degree[v]);
 for(int i=0;i+1<n;++i){graph[ends[2*i]].push_back(ends[2*i+1]);graph[ends[2*i+1]].push_back(ends[2*i]);}
 using Range=segtree<Affine,append_affine,identity_affine>;
 auto hld=HeavyLightDecomposition(graph).with_values<Range,true>(value);
 while(q--){int type{};verify_in>>type;if(type==0){int p{};Affine f{};verify_in>>p>>f.a>>f.b;hld.set(p,f);}else{int u{},v{};std::uint32_t x{};verify_in>>u>>v>>x;const auto f=hld.path_prod(u,v);verify_out<<(std::uint64_t(f.a)*x+f.b)%modulus<<'\n';}}
}
