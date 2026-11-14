// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include "../../../Algorithm/Mo.cpp"
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
int main(){
 std::mt19937_64 random(566087124);std::uint64_t checks=0;
 for(int trial=0;trial<150;++trial){const std::int32_t n=trial%9==0?INT32_MAX:static_cast<std::int32_t>(random()%100);Mo m(n);MoWithUpdates timed(n);if(n)for(int i=0;i<100;++i)timed.add_update(static_cast<std::int32_t>(random()%static_cast<std::uint64_t>(n)));
  for(int i=0;i<220;++i){auto l=static_cast<std::int32_t>(random()%(static_cast<std::uint64_t>(n)+1)),r=static_cast<std::int32_t>(random()%(static_cast<std::uint64_t>(n)+1));if(l>r)std::swap(l,r);m.add_query(l,r);timed.add_query(l,r,static_cast<std::size_t>(random()%(timed.update_count()+1)));if(i%17==0){m.add_query(l,r);timed.add_query(l,r,timed.query(timed.query_count()-1).time);}}
  for(const std::int32_t b:std::array<std::int32_t,5>{0,1,2,17,INT32_MAX}){
   const auto width=m.block_size(b);auto want=m.order(b);std::iota(want.begin(),want.end(),std::size_t{0});auto key=[&](std::size_t id){const auto q=m.query(id);const auto block=static_cast<std::int64_t>(q.left/width),right=static_cast<std::int64_t>(q.right);return std::tuple{block,block%2?-right:right,id};};std::sort(want.begin(),want.end(),[&](auto a,auto c){return key(a)<key(c);});assert(m.order(b)==want);++checks;
   const auto tw=timed.block_size(b);auto expected=timed.order(b);std::iota(expected.begin(),expected.end(),std::size_t{0});auto tk=[&](std::size_t id){const auto q=timed.query(id);const auto lb=static_cast<std::int64_t>(q.left/tw),rb=static_cast<std::int64_t>(q.right/tw),t=static_cast<std::int64_t>(q.time);return std::tuple{lb,lb%2?-rb:rb,(lb+rb)%2?-t:t,id};};std::sort(expected.begin(),expected.end(),[&](auto a,auto c){return tk(a)<tk(c);});assert(timed.order(b)==expected);++checks;
  }
 }
 std::cout<<"Mo normalized-key ordering checks="<<checks<<'\n';
}
