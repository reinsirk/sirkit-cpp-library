// SPDX-License-Identifier: CC0-1.0
// 多倍長の同期緩和・負閉路からの到達伝播を独立参照とする。
// 通常実行はcustom inf、引数default指定時は既定APIを同じ入力で検証する。
// FWの入力辺は(-inf, inf)にある場合だけ照合する。
#include <boost/multiprecision/cpp_int.hpp>
#include "../../../Graph/Dijkstra.cpp"
#include "../../../Graph/Bellman-Ford.cpp"
#include "../../../Graph/Bellman-Ford_SFPA.cpp"
#include "../../../Graph/Floyd-Warshall.cpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <random>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <tuple>
#include <utility>
#include <vector>
using namespace sirkit;
using I=std::int64_t;
using Big=boost::multiprecision::cpp_int;
struct Ref {std::vector<Big>d;std::vector<bool>reach,neg;};
std::uint64_t digest=UINT64_C(1469598103934665603); std::size_t checks=0;
void require(bool ok,const std::string&msg){if(!ok){std::cerr<<"FAIL "<<msg<<'\n';std::exit(1);}++checks;}
Ref reference(const WGraph& g,int s){
 const int n=g.size();Ref r{std::vector<Big>(n),std::vector<bool>(n),std::vector<bool>(n)};r.reach[s]=true;
 for(int k=0;k<n-1;++k){auto d=r.d;auto reach=r.reach;for(int u=0;u<n;++u)if(r.reach[u])for(auto e:g[u])if(!reach[e.to]||r.d[u]+e.w<d[e.to]){reach[e.to]=true;d[e.to]=r.d[u]+e.w;}r.d.swap(d);r.reach.swap(reach);}
 for(int u=0;u<n;++u)if(r.reach[u])for(auto e:g[u])if(!r.reach[e.to]||r.d[u]+e.w<r.d[e.to])r.neg[e.to]=true;
 for(int k=0;k<n;++k)for(int u=0;u<n;++u)if(r.neg[u])for(auto e:g[u])r.neg[e.to]=true;
 return r;
}
std::pair<std::vector<I>,bool> expected(const Ref&r,I inf,bool&over){std::vector<I>v(r.d.size());bool neg=false;over=false;for(std::size_t i=0;i<v.size();++i){if(r.neg[i]){v[i]=-inf;neg=true;}else if(!r.reach[i])v[i]=inf;else if(r.d[i]<=-Big(inf)||r.d[i]>=Big(inf))over=true;else v[i]=r.d[i].convert_to<I>();}return{v,neg};}
void hash(const std::vector<I>&v,bool neg){digest^=neg;digest*=UINT64_C(1099511628211);for(I x:v){digest^=static_cast<std::uint64_t>(x);digest*=UINT64_C(1099511628211);}}
void pathcheck(const WGraph&g,const Ref&r,const std::vector<int>&p,int s,int t,I expected){
 if(!r.reach[t]||r.neg[t]){require(p.empty(),"path empty");return;}require(!p.empty()&&p.front()==s&&p.back()==t&&p.size()<=g.size(),"path ends");Big sum=0;for(std::size_t i=1;i<p.size();++i){bool found=false;I best=0;for(auto e:g[p[i-1]])if(e.to==p[i]&&(!found||e.w<best)){best=e.w;found=true;}require(found,"path edge");sum+=best;}require(sum==expected,"path weight");}
void checkgraph(const WGraph&g,int s,I inf,bool nonnegative,bool defaultcall){auto r=reference(g,s);bool over;auto [ex,negative]=expected(r,inf,over);
 for(int kind=0;kind<(nonnegative?6:2);++kind){std::vector<I>d{73};std::vector<int>p{74};bool flag=false,thrown=false;int target=g.size()-1;try{
 if(kind==0)flag=defaultcall?BellmanFord(g,d,s):BellmanFord(g,d,s,inf);else if(kind==1)flag=defaultcall?SPFA(g,d,s):SPFA(g,d,s,inf);else if(kind==2){if(defaultcall)Dijkstra(g,d,s);else Dijkstra(g,d,s,inf);}else if(kind==3){if(defaultcall)Dijkstra(g,d,s,target,p);else Dijkstra(g,d,s,target,p,inf);}else{std::vector<std::tuple<int,int,I>>edges;for(int u=0;u<(int)g.size();++u)for(auto e:g[u])edges.emplace_back(u,e.to,e.w);if(kind==4){if(defaultcall)Dijkstra(g.size(),edges,d,s);else Dijkstra(g.size(),edges,d,s,inf);}else{if(defaultcall)Dijkstra(g.size(),edges,d,s,target,p);else Dijkstra(g.size(),edges,d,s,target,p,inf);}}
 }catch(const std::overflow_error&){thrown=true;}
 require(thrown==over,"graph overflow kind="+std::to_string(kind)+" inf="+std::to_string(inf));if(!over){require(d==ex&&flag==(kind<2?negative:false),"graph result kind="+std::to_string(kind));hash(d,flag);if(kind==3||kind==5)pathcheck(g,r,p,s,target,ex[target]);}
#ifndef SKIP_RUNTIME_CHECKS
 else require(d==std::vector<I>{73},"graph exception preserves distance");
#endif
 }
}
void checkfw(const WGraph&g,I inf,bool defaultcall){for(const auto&row:g)for(auto e:row)if(e.w<=-inf||e.w>=inf)return;const int n=g.size();std::vector<std::vector<I>>a(n,std::vector<I>(n,inf)),ex;std::vector<Ref>refs;bool over=false,negative=false;for(int i=0;i<n;++i){a[i][i]=0;for(auto e:g[i])a[i][e.to]=std::min(a[i][e.to],e.w);refs.push_back(reference(g,i));bool o;auto[v,ng]=expected(refs.back(),inf,o);over|=o;negative|=ng;ex.push_back(v);}for(int restore=0;restore<2;++restore){auto d=a;std::vector<std::vector<int>>next;bool flag=false,thrown=false;try{
 if(defaultcall)flag=restore?FloydWarshall(d,next):FloydWarshall(d);else flag=restore?FloydWarshall(d,next,inf):FloydWarshall(d,inf);
 }catch(const std::overflow_error&){thrown=true;}
 require(thrown==over,"FW overflow inf="+std::to_string(inf));if(!over){require(d==ex&&flag==negative,"FW result inf="+std::to_string(inf));for(auto&row:d)hash(row,flag);if(restore)for(int s=0;s<n;++s)for(int t=0;t<n;++t)pathcheck(g,refs[s],FloydWarshallPath(next,s,t),s,t,ex[s][t]);}}
}
int main(int argc,char**){bool onlydefault=argc>1;std::mt19937_64 rng(20261115);std::vector<I>infs{1,2,7,100,I{1000000000000},kInf-1,kInf,kInf+1,INT64_MAX-1,INT64_MAX};
 for(int iter=0;iter<1500;++iter){int n=1+rng()%13;WGraph g(n);bool nonnegative=iter%3==0;for(int j=0,m=rng()%(n*n+1);j<m;++j){int u=rng()%n,v=rng()%n;I w=rng()%31;if(!nonnegative)w-=15;if(iter%7==0)w*=30000000;g[u].push_back({v,w});}I inf=onlydefault?kInf:infs[iter%infs.size()];int s=rng()%n;checkgraph(g,s,inf,nonnegative,onlydefault);checkfw(g,inf,onlydefault);}
 for(int n:{64,65,256,257,4096,4097})for(int shape=0;shape<4;++shape){WGraph g(n);for(int i=0;i<n-1;++i)g[i].push_back({i+1,shape==0?I{0}:I{2}});if(shape==2)g[n-1].push_back({n/2,-1000000000});if(shape==3){g[0].push_back({n-1,1000000000});g[0].push_back({n-1,1});}checkgraph(g,0,onlydefault?kInf:100,shape!=2,onlydefault);
if(!onlydefault && n>=65)checkgraph(g,0,I{1000000000000},shape!=2,false);
}
 if(!onlydefault){
 for(I inf:{I{0},I{-1},I{INT64_MIN}}){WGraph g(1);std::vector<I>d;std::vector<std::vector<I>>m;for(int type=0;type<4;++type){bool invalid=false;try{if(type==0)Dijkstra(g,d,0,inf);if(type==1)BellmanFord(g,d,0,inf);if(type==2)SPFA(g,d,0,inf);if(type==3)FloydWarshall(m,inf);}catch(const std::invalid_argument&){invalid=true;}require(invalid,"nonpositive inf");}}
 for(I inf:{I{1},I{7},I{kInf+100},I{INT64_MAX}}){WGraph g(4);g[0]={{1,inf-1},{3,inf==1?0:1}};g[1]={{2,-(inf-1)}};checkfw(g,inf,false);}
 for(int shape=0;shape<6;++shape){WGraph g(5);I w=INT64_MAX-3;g[0]={{1,w}};g[1]={{2,shape%2?w:-w}};if(shape>=2)g[2].push_back({1,-w});if(shape>=4)g[2].push_back({3,1});checkfw(g,INT64_MAX,false);}
 }
 std::cout<<"PASS checks="<<checks<<" digest="<<digest<<'\n';}
