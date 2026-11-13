// SPDX-License-Identifier: CC0-1.0
// 公開APIの所有・再利用・copy失敗を独立参照で確認する。保持形式・確保回数に依存しない。
#ifndef FENWICK_SOURCE
#define FENWICK_SOURCE "../../../DataStructure/dynamic-binary-indexed-tree-2d.cpp"
#endif
#include <cstdlib>
#include <cstdio>
#include <new>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <iterator>
static int failure_after=-1;
void* operator new(std::size_t n){if(failure_after==0)throw std::bad_alloc();if(failure_after>0)--failure_after;if(void*p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline]]
#endif
static void release_allocation(void*p)noexcept{std::free(p);}
void operator delete(void*p)noexcept{release_allocation(p);}
void operator delete[](void*p)noexcept{release_allocation(p);}
void operator delete(void*p,std::size_t)noexcept{release_allocation(p);}
void operator delete[](void*p,std::size_t)noexcept{release_allocation(p);}
#include FENWICK_SOURCE
using I=std::int64_t;using Position=std::pair<int,int>;
static std::uint64_t assertions=0,scenarios=0,caught_failures=0;
static void check(bool ok){++assertions;if(!ok){std::fprintf(stderr,"assertion %ju failed\n",std::uintmax_t(assertions));std::abort();}}
struct Reference{
 int n=0,m=0;std::map<Position,I> points;std::set<std::pair<std::uint64_t,std::uint64_t>> nodes;
 void add(int x,int y,I v){points[{x,y}]+=v;for(std::uint64_t r=std::uint64_t(x)+1;r<=std::uint64_t(n);r+=r&(0-r))for(std::uint64_t c=std::uint64_t(y)+1;c<=std::uint64_t(m);c+=c&(0-c))nodes.emplace(r,c);}
 I sum(int l,int b,int r,int t)const{I s=0;for(auto[xy,v]:points)if(l<=xy.first&&xy.first<r&&b<=xy.second&&xy.second<t)s+=v;return s;}
 void clear(){points.clear();nodes.clear();}
};
struct State{
 DynamicFenwickTree2D<I> bit;Reference ref;
 State(int n,int m):bit(n,m),ref{n,m,{},{}}{}
 void add(int x,int y,I v){bit.add(x,y,v);ref.add(x,y,v);}
};
static State build(int profile){
 const int rows[]={0,3,257,1,1,1000003,1,1},cols[]={4096,5,4096,16381,16381,1000003,4096,16381},counts[]={0,7,23,61,701,173,173,4096};
 State s(rows[profile],cols[profile]);
 for(int j=0;j<counts[profile];++j){int x=s.ref.n==1?0:s.ref.n-1;int y=int(std::uint64_t(j)*17%std::uint64_t(s.ref.m));s.add(x,y,I(j%13)-6);}
 return s;
}
static void verify(const DynamicFenwickTree2D<I>&bit,const Reference&r){
 check(bit.rows()==r.n&&bit.cols()==r.m&&bit.empty()==(!r.n||!r.m));check(bit.stored_nodes()==r.nodes.size());
 check(bit.sum_all()==r.sum(0,0,r.n,r.m));check(bit.sum(r.n,r.m)==r.sum(0,0,r.n,r.m));
 for(int l:{0,r.n/2})for(int b:{0,r.m/2})check(bit.sum(l,b,r.n,r.m)==r.sum(l,b,r.n,r.m));
 if(!r.points.empty())for(auto it:{r.points.begin(),std::next(r.points.begin(),std::ptrdiff_t(r.points.size()/2)),std::prev(r.points.end())})check(bit.get(it->first.first,it->first.second)==it->second);
 check(bit.stored_nodes()==r.nodes.size());
}
static void reuse(State&s){
 s.bit.clear();s.ref.clear();verify(s.bit,s.ref);
 for(auto[n,m]:{Position{1,16381},Position{1,4096},Position{3,5},Position{0,16381}}){s.bit.reset(n,m);s.ref={n,m,{},{}};verify(s.bit,s.ref);if(n&&m){for(int j=0;j<181;++j)s.add(n-1,int(std::uint64_t(j)*19%std::uint64_t(m)),j%2?0:3);verify(s.bit,s.ref);}}
}
int main(){
 for(int a=0;a<8;++a)for(int b=0;b<8;++b){++scenarios;State left=build(a),right=build(b);verify(left.bit,left.ref);verify(right.bit,right.ref);
  left.bit.swap(right.bit);std::swap(left.ref,right.ref);verify(left.bit,left.ref);verify(right.bit,right.ref);
  auto copy=left.bit;Reference expected=left.ref;verify(copy,expected);
  if(expected.n&&expected.m){copy.add(0,0,11);expected.add(0,0,11);verify(copy,expected);verify(left.bit,left.ref);}
  right.bit=left.bit;right.ref=left.ref;verify(right.bit,right.ref);
  auto* same=&right.bit;right.bit=*same;right.bit=std::move(*same);verify(right.bit,right.ref);
  auto moved=std::move(copy);check(copy.empty()&&copy.rows()==0&&copy.cols()==0&&copy.stored_nodes()==0&&copy.sum_all()==0);verify(moved,expected);
  right.bit=std::move(moved);right.ref=expected;check(moved.empty()&&moved.rows()==0&&moved.cols()==0&&moved.sum_all()==0);verify(right.bit,right.ref);reuse(right);
 }
 // 失敗位置を複数選び、copyが成功するかどうかにかかわらず値と所有権を照合する。
 for(int profile:{2,3,4,5,6,7})for(int position=0;position<7;++position){State from=build(profile),to=build(1);failure_after=position;bool failed=false;
  try{to.bit=from.bit;}catch(const std::bad_alloc&){failed=true;}failure_after=-1;
  if(failed)++caught_failures;else to.ref=from.ref;
  verify(from.bit,from.ref);verify(to.bit,to.ref);
  to.bit=from.bit;to.ref=from.ref;to.add(0,0,13);verify(to.bit,to.ref);verify(from.bit,from.ref);reuse(to);
 }
 check(caught_failures>0);
 std::printf("{\"ownership_behavior_scenarios\":%ju,\"ownership_behavior_assertions\":%ju,\"caught_copy_allocation_failures\":%ju,\"status\":\"pass\"}\n",std::uintmax_t(scenarios),std::uintmax_t(assertions),std::uintmax_t(caught_failures));
}
