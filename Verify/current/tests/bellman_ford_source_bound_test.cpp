// SPDX-License-Identifier: CC0-1.0
// 出辺あり頂点数の上限・多数sink・負閉路・Warm引継ぎを独立参照で照合する。
#ifndef SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#define SIRKIT_TEST_NEGATIVE_PATH_SOURCE "../../../Graph/Bellman-Ford.cpp"
#endif
#include SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

namespace {
std::uint64_t checks=0,cases=0;
void require(bool value){++checks;if(!value){std::cerr<<"source-bound failure "<<cases<<' '<<checks<<'\n';std::abort();}}
std::vector<std::int64_t> oracle(const WGraph&g,int start){
 int n=int(g.size());std::vector<std::vector<std::int64_t>>d(n,std::vector<std::int64_t>(n,INF));
 for(int v=0;v<n;++v){d[v][v]=0;for(auto e:g[v])d[v][e.to]=std::min(d[v][e.to],e.w);}
 for(int k=0;k<n;++k)for(int i=0;i<n;++i)if(d[i][k]!=INF)for(int j=0;j<n;++j)if(d[k][j]!=INF)
  d[i][j]=std::min(d[i][j],std::max(-INF/4,d[i][k]+d[k][j]));
 auto expected=d[start];
 for(int k=0;k<n;++k)if(d[start][k]!=INF&&d[k][k]<0)for(int j=0;j<n;++j)if(d[k][j]!=INF)expected[j]=-INF;
 return expected;
}
void test(const WGraph&g,int start,const std::vector<std::int64_t>&expected){
 ++cases;bool negative=std::find(expected.begin(),expected.end(),-INF)!=expected.end();
 std::vector<std::int64_t>d{7,8};
 auto check=[&](bool found){require(found==negative);require(d==expected);};
 check(BellmanFord(g,d,start));d={7,8};
 check(reinsirk::detail::BellmanFordImpl<true,true>(g,d,start));d={7,8};
 check(reinsirk::detail::BellmanFordImpl<true,false>(g,d,start));
 if(g.size()<=64){d={7,8};check(reinsirk::detail::BellmanFordImpl<false,false>(g,d,start));}
}
}
int main(){
 std::mt19937 rng(0x55018);
 // 多数sink、源点数0/1/63/64、並列辺、自己辺を含む小例。
 for(int trial=0;trial<4000;++trial){
  int n=1+int(rng()%35);int count=int(rng()%(n+1));WGraph g(n);
  std::vector<int> sources(n);std::iota(sources.begin(),sources.end(),0);std::shuffle(sources.begin(),sources.end(),rng);
  for(int i=0;i<count;++i){int u=sources[i];for(unsigned e=0,len=rng()%15;e<len;++e){
   int v=int(rng()%n);std::int64_t w=int(rng()%11)-5;if(trial%7==0)w*=200000000;
   g[u].emplace_back(v,w);
  }}
  int start=int(rng()%n);test(g,start,oracle(g,start));
 }
 // 64源点境界と多くのsink。潜在関数法で負閉路なしのグラフも混ぜる。
 for(int trial=0;trial<400;++trial){
  int n=65+int(rng()%30);WGraph g(n);int count=std::min(n,61+trial%7);
  std::vector<int> permutation(n);std::iota(permutation.begin(),permutation.end(),0);std::shuffle(permutation.begin(),permutation.end(),rng);
  std::vector<int> potential(n);for(int&x:potential)x=int(rng()%11)-5;
  for(int i=0;i<count;++i){int u=permutation[i];for(int e=0;e<3;++e){int v=int(rng()%n);std::int64_t w=int(rng()%9)-4;
   if(trial%3==0) w=std::int64_t(rng()%5)+potential[v]-potential[u];
   g[u].emplace_back(v,w);
  }}
  int start=permutation[0];test(g,start,oracle(g,start));
 }
 // 厳密にSパス必要な逆順鎖。NはSより十分大きくし、並列辺で疎試行を超える。
 for(int count:{1,2,3,15,16,17,62,63,64,65,127})for(bool cycle:{false,true}){
  const int n=20000;WGraph g(n);std::vector<int> path;for(int i=0;i<count;++i)path.push_back(n-2-i);path.push_back(0);
  for(int i=0;i<count;++i)for(int copy=0;copy<300/count+3;++copy)g[path[i]].emplace_back(path[i+1],-1);
  if(cycle)g[path[count-1]].emplace_back(path[0],-1);
  std::vector<std::int64_t> expected(n,INF);
  for(int i=0;i<=count;++i)expected[path[i]]=cycle?-INF:-i;
  test(g,path[0],expected);
 }
 // 前進試行が未到達の後退先で停止し、距離をWarm経路へ引き継ぐ。
 for(int count:{2,3,15,16,17,62,63,64,65})for(bool cycle:{false,true}){
  const int n=20000;WGraph g(n);std::vector<int> path{0};
  for(int i=1;i<count;++i)path.push_back(n-i);
  path.push_back(1);
  g[0].emplace_back(path[1],-1);
  for(int i=1;i<count;++i)for(int copy=0;copy<300/count+3;++copy)g[path[i]].emplace_back(path[i+1],-1);
  if(cycle)g[path[count-1]].emplace_back(path[0],-1);
  std::vector<std::int64_t> expected(n,INF);
  for(int i=0;i<=count;++i)expected[path[i]]=cycle?-INF:-i;
  test(g,0,expected);
 }
 // 単一源点から多数sinkへ。負自己辺の影響が全sinkへ届く。
 for(bool cycle:{false,true}){
  const int n=20000;WGraph g(n);std::vector<std::int64_t> expected(n,INF);expected[0]=0;
  for(int i=1;i<n;++i){g[0].emplace_back(i,i%3);expected[i]=i%3;}
  if(cycle){g[0].emplace_back(0,-1);std::fill(expected.begin(),expected.end(),-INF);}
  test(g,0,expected);
 }
 // 多数の未到達sourceと、少数の到達頂点に集中する多重辺。
 for(int count:{2,10,63}){
  const int n=2500;WGraph g(n);
  g[0].emplace_back(1,0);g[0].emplace_back(n-1,1);g[1].emplace_back(n-1,0);
  for(int i=1;i<=count;++i)g[i].emplace_back(i%count+1,-1);
  for(int i=count+1;i<n-1;++i)g[i].emplace_back(i,-1);
  for(int i=0;i<2500;++i)g[1+i%count].emplace_back(1+(i+1)%count,-1);
  std::vector<std::int64_t> expected(n,INF);expected[0]=0;expected[n-1]=-INF;
  for(int i=1;i<=count;++i)expected[i]=-INF;
  test(g,0,expected);
 }
#ifndef SKIP_RUNTIME_CHECKS
 // Sパス目に初めて到達するsink/最終源点の入力検査と出力保全。
 for(int count:{1,2,3,63,64,65})for(int kind:{0,1}){
  const int n=1000;WGraph g(n);int start=count;
  for(int i=count;i>0;--i)for(int copy=0;copy<4;++copy)g[i].emplace_back(i-1,-1);
  // 最終の源点1に不正辺を置く。未到達成分の辺も含めて全走査する検査ではない。
  g[1].emplace_back(kind?0:n,kind?1000000001:0);std::vector<std::int64_t>d{77,88};bool threw=false;
  try{(void)BellmanFord(g,d,start);}catch(const std::out_of_range&){threw=true;}
  require(threw&&d==std::vector<std::int64_t>({77,88}));
 }
#endif
 std::cout<<"source-bound cases="<<cases<<" checks="<<checks<<'\n';
}
