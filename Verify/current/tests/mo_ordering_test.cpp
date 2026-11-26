// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include "../../../Algorithm/Mo.cpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
#include <vector>
int main(){
 std::mt19937_64 random(566087124);std::uint64_t checks=0;
 // 独立に正規化した鍵で並べた順と一致するか確かめる。
 auto check_static=[&](const Mo& m,std::int32_t b){const auto width=m.block_size(b);std::vector<std::size_t>want(m.query_count());std::iota(want.begin(),want.end(),std::size_t{0});auto key=[&](std::size_t id){const auto q=m.query(id);const auto block=static_cast<std::int64_t>(q.left/width),right=static_cast<std::int64_t>(q.right);return std::tuple{block,block%2?-right:right,id};};std::sort(want.begin(),want.end(),[&](auto a,auto c){return key(a)<key(c);});assert(m.order(b)==want);++checks;};
 auto check_timed=[&](const MoWithUpdates& timed,std::int32_t b){const auto tw=timed.block_size(b);std::vector<std::size_t>expected(timed.query_count());std::iota(expected.begin(),expected.end(),std::size_t{0});auto tk=[&](std::size_t id){const auto q=timed.query(id);const auto lb=static_cast<std::int64_t>(q.left/tw),rb=static_cast<std::int64_t>(q.right/tw),t=static_cast<std::int64_t>(q.time);return std::tuple{lb,lb%2?-rb:rb,(lb+rb)%2?-t:t,id};};std::sort(expected.begin(),expected.end(),[&](auto a,auto c){return tk(a)<tk(c);});assert(timed.order(b)==expected);++checks;};
 auto random_range=[&](std::int32_t n){auto l=static_cast<std::int32_t>(random()%(static_cast<std::uint64_t>(n)+1)),r=static_cast<std::int32_t>(random()%(static_cast<std::uint64_t>(n)+1));if(l>r)std::swap(l,r);return std::pair{l,r};};
 for(int trial=0;trial<150;++trial){const std::int32_t n=trial%9==0?INT32_MAX:static_cast<std::int32_t>(random()%100);Mo m(n);MoWithUpdates timed(n);if(n)for(int i=0;i<100;++i)timed.add_update(static_cast<std::int32_t>(random()%static_cast<std::uint64_t>(n)));
  for(int i=0;i<220;++i){const auto [l,r]=random_range(n);m.add_query(l,r);timed.add_query(l,r,static_cast<std::size_t>(random()%(timed.update_count()+1)));if(i%17==0){m.add_query(l,r);timed.add_query(l,r,timed.query(timed.query_count()-1).time);}}
  for(const std::int32_t b:std::array<std::int32_t,5>{0,1,2,17,INT32_MAX}){check_static(m,b);check_timed(timed,b);}
 }
 // 基数ソートへ切り替える要求数の前後と、桁が一定の入力。
 for(const std::size_t count:{std::size_t{1023},std::size_t{1024},std::size_t{1025},std::size_t{6000}})for(int shape=0;shape<4;++shape){
  const std::int32_t n=shape==3?0:100000;Mo m(n);MoWithUpdates timed(n);if(n)for(int i=0;i<3000;++i)timed.add_update(static_cast<std::int32_t>(random()%static_cast<std::uint64_t>(n)));
  for(std::size_t i=0;i<count;++i){auto [l,r]=random_range(n);if(shape==1)r=n;if(shape==2)l=5;if(l>r)std::swap(l,r);m.add_query(l,r);timed.add_query(l,r,static_cast<std::size_t>(random()%(timed.update_count()+1)));if(i%29==0){m.add_query(l,r);timed.add_query(l,r,timed.query(timed.query_count()-1).time);}}
  for(const std::int32_t b:std::array<std::int32_t,6>{0,1,2,17,1000,INT32_MAX}){check_static(m,b);check_timed(timed,b);}
 }
 // 鍵の幅が64bitちょうどなら基数ソート、超えれば比較ソートを使う。両側で同じ順か確かめる。
 {Mo m(INT32_MAX);for(int i=0;i<2048;++i){const auto [l,r]=random_range(INT32_MAX);m.add_query(l,r);}
  const auto bits=[&](std::int32_t b){return std::bit_width(static_cast<std::uint32_t>(INT32_MAX/b))+std::bit_width(static_cast<std::uint32_t>(INT32_MAX))+std::bit_width(m.query_count()-1);};
  assert(bits(512)==64&&bits(256)==65);for(const std::int32_t b:{0,256,512})check_static(m,b);}
 {MoWithUpdates timed(INT32_MAX);for(int i=0;i<(1<<20)-1;++i)timed.add_update(static_cast<std::int32_t>(random()%INT32_MAX));for(int i=0;i<1024;++i){const auto [l,r]=random_range(INT32_MAX);timed.add_query(l,r,static_cast<std::size_t>(random()%(timed.update_count()+1)));}
  const auto bits=[&](std::int32_t b){return 2*std::bit_width(static_cast<std::uint32_t>(INT32_MAX/b))+std::bit_width(timed.update_count())+std::bit_width(timed.query_count()-1);};
  assert(bits(16384)==64&&bits(8192)==66);for(const std::int32_t b:{0,8192,16384})check_timed(timed,b);}
 // 長さと幅で64bitを超える鍵を作り、同じ左ブロック内で右端の比較を確かめる。
 {
  Mo m(INT32_MAX);
  for (const auto [left, right] : std::array<Mo::Query, 8>{{
       {1, 8}, {1, 3}, {2, 9}, {2, 5}, {1, 6}, {2, 9}, {2, 7}, {1, 8}}})
  {
   m.add_query(left, right);
  }
  check_static(m, 1);
  std::vector<std::size_t> actual;
  m.run_rollback([](std::int32_t) {}, [] {}, [] {}, [] {},
                 [&](std::size_t id) { actual.push_back(id); }, 1);
  const std::vector<std::size_t> expected{1, 4, 0, 7, 3, 6, 2, 5};
  assert(actual == expected);
 }
 // 左右のブロックが同じ照会を並べ、時刻が昇順・降順になる組を両方確かめる。
 {
  MoWithUpdates timed(INT32_MAX);
  for (std::int32_t pos : {0, 1, 2})
  {
   timed.add_update(pos);
  }
  for (std::size_t time : {2u, 0u, 3u, 1u})
  {
   timed.add_query(1, 3, time);
   timed.add_query(1, 4, time);
  }
  check_timed(timed, 1);
 }
 std::cout<<"Mo normalized-key ordering checks="<<checks<<'\n';
}
