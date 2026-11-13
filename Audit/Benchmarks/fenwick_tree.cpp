// SPDX-License-Identifier: CC0-1.0
// BENCH_LIBRARYとBENCH_OPERATIONを指定し、操作ごとに独立したバイナリを作る。
#include BENCH_LIBRARY
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <time.h>
struct Query {std::size_t l,r,i; std::int64_t x; unsigned type;};
std::uint64_t stamp(){timespec t{};clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t);return std::uint64_t(t.tv_sec)*1000000000+t.tv_nsec;}
std::uint64_t state;std::uint64_t next(){state^=state<<13;state^=state>>7;return state^=state<<17;}
int main(int argc,char**argv){
 const std::size_t n=std::strtoull(argv[1],0,10),q=std::strtoull(argv[2],0,10);const int op=BENCH_OPERATION;state=std::strtoull(argv[4],0,10);
 std::vector<std::int64_t>a(n);for(auto&v:a)v=next()%1000;
 std::vector<Query>qs(q);for(auto&v:qs){v.l=next()%(n+1);v.r=next()%(n+1);if(v.l>v.r)std::swap(v.l,v.r);v.i=next()%n;v.x=next()%8;v.type=next()%2;}
#ifdef SIRKIT_LEGACY_RANGE_API
 using Tree = FenwickTree<std::int64_t>;
#else
 using Tree = fenwick_tree<std::int64_t>;
#endif
 const auto st=stamp();Tree t(a);const auto built=stamp();std::uint64_t ck=0;
 for(auto x:qs){
 switch(op){
 case 0:if(x.type)t.add(x.i,x.x);else ck+=t.sum(x.l,x.r);break;
 case 1:t.add(x.i,x.x);break;
 case 2:ck+=t.sum(x.l,x.r);break;
 case 3:ck+=t.sum(x.i,std::min(n,x.i+1+(x.r%16)));break;
 case 4:ck+=t.sum(0,x.r);break;
 case 5:t.set(x.i,x.x);break;
 case 6:ck+=t.get(x.i);break;
 case 7:ck+=t.lower_bound(x.i*500);break;
 }
 }
 ck+=t.sum(0,t.size());const auto end=stamp();std::printf("{\"checksum\":%ju,\"build_ns\":%ju,\"op_ns\":%ju}\n",(std::uintmax_t)ck,(std::uintmax_t)(built-st),(std::uintmax_t)(end-built));
}
