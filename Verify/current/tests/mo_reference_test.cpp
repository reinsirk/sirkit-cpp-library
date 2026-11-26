// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_MO_SOURCE
#define SIRKIT_MO_SOURCE "../../../Algorithm/Mo.cpp"
#endif
#include SIRKIT_MO_SOURCE
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <tuple>
#include <cstdlib>
std::uint64_t checks=0,applied=0,undone=0,inside=0,outside=0;std::mt19937_64 random_engine(843560901005ULL);
void require(bool value){++checks;if(!value){std::cerr<<"failed check "<<checks<<'\n';std::abort();}}
template<class F>void rejected(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught);}
std::size_t rng(std::size_t n){return static_cast<std::size_t>(random_engine()%n);}
template<class T>std::vector<T> saved(std::span<const T>a){return{a.begin(),a.end()};}
void static_case(const Mo& mo,const std::vector<int>&a,std::int32_t block){
 std::deque<std::pair<std::int32_t,int>>active;std::vector<std::size_t>seen;const auto expected_order=mo.order(block);
 auto add_left=[&](std::int32_t i){require(i>=0&&static_cast<std::size_t>(i)<a.size());require(active.empty()||active.front().first==i+1);active.emplace_front(i,a[static_cast<std::size_t>(i)]);};
 auto add_right=[&](std::int32_t i){require(i>=0&&static_cast<std::size_t>(i)<a.size());require(active.empty()||active.back().first+1==i);active.emplace_back(i,a[static_cast<std::size_t>(i)]);};
 auto erase_left=[&](std::int32_t i){require(!active.empty()&&active.front()==std::pair{i,a[static_cast<std::size_t>(i)]});active.pop_front();};
 auto erase_right=[&](std::int32_t i){require(!active.empty()&&active.back()==std::pair{i,a[static_cast<std::size_t>(i)]});active.pop_back();};
 auto answer=[&](std::size_t id){seen.push_back(id);const auto q=mo.query(id);require(active.size()==static_cast<std::size_t>(q.right-q.left));for(auto i=q.left;i<q.right;++i)require(active[static_cast<std::size_t>(i-q.left)]==std::pair{i,a[static_cast<std::size_t>(i)]});};
 const auto end=mo.run(add_left,add_right,erase_left,erase_right,answer,block);require(seen==expected_order);
 if(seen.empty())require(end==Mo::Window{0,0});else{const auto q=mo.query(seen.back());require(end==Mo::Window{q.left,q.right});}
 std::set<std::int32_t>indices;std::int64_t sum=0;std::size_t answers=0;
 auto add=[&](std::int32_t i){require(indices.insert(i).second);sum+=a[static_cast<std::size_t>(i)];};auto erase=[&](std::int32_t i){require(indices.erase(i)==1);sum-=a[static_cast<std::size_t>(i)];};
 auto symmetric_answer=[&](std::size_t id){++answers;const auto q=mo.query(id);require(sum==std::accumulate(a.begin()+q.left,a.begin()+q.right,std::int64_t{0}));};
 require(mo.run(std::ref(add),std::ref(erase),std::ref(symmetric_answer),block)==end);require(answers==mo.query_count());
}
// 巻き戻し版: 追加は区間の両端に隣接し、保存は同時に1つ、resetは空でない右側だけを消す。
std::uint64_t rollback_left_adds=0,rollback_resets=0;
void rollback_case(const Mo& mo,const std::vector<int>&a,std::int32_t block){
 using Item=std::pair<std::int32_t,int>;std::deque<Item>current;std::optional<std::deque<Item>>saved;std::vector<std::size_t>seen;std::size_t snapshots=0,resets=0;
 const auto at=[&](std::int32_t i){require(i>=0&&static_cast<std::size_t>(i)<a.size());return a[static_cast<std::size_t>(i)];};
 auto add_left=[&](std::int32_t i){require(saved.has_value());require(current.empty()||current.front().first==i+1);current.emplace_front(i,at(i));++rollback_left_adds;};
 auto add_right=[&](std::int32_t i){require(current.empty()||current.back().first+1==i);current.emplace_back(i,at(i));};
 auto snapshot=[&]{require(!saved.has_value());saved=current;++snapshots;};
 auto rollback=[&]{require(saved.has_value());current=std::move(*saved);saved.reset();};
 auto reset=[&]{require(!saved.has_value()&&!current.empty());current.clear();++resets;};
 auto answer=[&](std::size_t id){require(saved.has_value());seen.push_back(id);const auto q=mo.query(id);require(current.size()==static_cast<std::size_t>(q.right-q.left));for(auto i=q.left;i<q.right;++i)require(current[static_cast<std::size_t>(i-q.left)]==Item{i,at(i)});};
 mo.run_rollback(add_left,add_right,snapshot,rollback,reset,answer,block);
 require(current.empty()&&!saved.has_value()&&snapshots==mo.query_count());
 const auto width=mo.block_size(block);std::vector<std::size_t>expected(mo.query_count());std::iota(expected.begin(),expected.end(),std::size_t{0});
 std::sort(expected.begin(),expected.end(),[&](std::size_t x,std::size_t y){const auto p=mo.query(x),q=mo.query(y);return std::tuple{p.left/width,p.right,x}<std::tuple{q.left/width,q.right,y};});
 require(seen==expected);std::set<std::int32_t>blocks;for(const auto q:mo.queries())blocks.insert(q.left/width);require(resets<=blocks.size());rollback_resets+=resets;
 std::int64_t sum=0,saved_sum=0;std::size_t answers=0;
 auto add=[&](std::int32_t i){sum+=at(i);};auto save=[&]{saved_sum=sum;};auto restore=[&]{sum=saved_sum;};auto clear=[&]{sum=0;};
 auto check=[&](std::size_t id){++answers;const auto q=mo.query(id);require(sum==std::accumulate(a.begin()+q.left,a.begin()+q.right,std::int64_t{0}));};
 mo.run_rollback(std::ref(add),std::ref(save),std::ref(restore),std::ref(clear),std::ref(check),block);require(answers==mo.query_count()&&sum==0);
}
struct Change{std::int32_t pos;int before,after;};
void timed_case(const MoWithUpdates&mo,const std::vector<int>&initial,const std::vector<Change>&changes,const std::vector<std::vector<int>>&versions,std::int32_t block){
 auto current=initial;std::deque<std::pair<std::int32_t,int>>active;std::vector<std::size_t>seen;const auto expected_order=mo.order(block);
 auto add_left=[&](std::int32_t i){require(i>=0&&static_cast<std::size_t>(i)<current.size());require(active.empty()||active.front().first==i+1);active.emplace_front(i,current[static_cast<std::size_t>(i)]);};
 auto add_right=[&](std::int32_t i){require(i>=0&&static_cast<std::size_t>(i)<current.size());require(active.empty()||active.back().first+1==i);active.emplace_back(i,current[static_cast<std::size_t>(i)]);};
 auto erase_left=[&](std::int32_t i){require(!active.empty()&&active.front()==std::pair{i,current[static_cast<std::size_t>(i)]});active.pop_front();};
 auto erase_right=[&](std::int32_t i){require(!active.empty()&&active.back()==std::pair{i,current[static_cast<std::size_t>(i)]});active.pop_back();};
 auto change=[&](std::size_t id,std::int32_t left,std::int32_t right,bool forward){const auto c=changes[id];require(c.pos==mo.update_position(id));require(active.size()==static_cast<std::size_t>(right-left));require(current[static_cast<std::size_t>(c.pos)]==(forward?c.before:c.after));const int value=forward?c.after:c.before;if(left<=c.pos&&c.pos<right){++inside;require(active[static_cast<std::size_t>(c.pos-left)].first==c.pos);active[static_cast<std::size_t>(c.pos-left)].second=value;}else ++outside;current[static_cast<std::size_t>(c.pos)]=value;};
 auto apply=[&](std::size_t id,std::int32_t l,std::int32_t r){++applied;change(id,l,r,true);};auto undo=[&](std::size_t id,std::int32_t l,std::int32_t r){++undone;change(id,l,r,false);};
 auto answer=[&](std::size_t id){seen.push_back(id);const auto q=mo.query(id);require(current==versions[q.time]);require(active.size()==static_cast<std::size_t>(q.right-q.left));for(auto i=q.left;i<q.right;++i)require(active[static_cast<std::size_t>(i-q.left)]==std::pair{i,versions[q.time][static_cast<std::size_t>(i)]});};
 const auto end=mo.run(add_left,add_right,erase_left,erase_right,apply,undo,answer,block);require(seen==expected_order);
 if(seen.empty())require(end==MoWithUpdates::State{0,0,0});else{const auto q=mo.query(seen.back());require(end==MoWithUpdates::State{q.left,q.right,q.time});}
 current=initial;std::set<std::int32_t>indices;std::int64_t sum=0;std::size_t answers=0;
 auto add=[&](std::int32_t i){require(indices.insert(i).second);sum+=current[static_cast<std::size_t>(i)];};auto erase=[&](std::int32_t i){require(indices.erase(i)==1);sum-=current[static_cast<std::size_t>(i)];};
 auto assign=[&](std::size_t id,std::int32_t l,std::int32_t r,bool forward){const auto c=changes[id];const auto pos=static_cast<std::size_t>(c.pos);const auto expected=forward?c.before:c.after;require(current[pos]==expected);if(l<=c.pos&&c.pos<r)sum-=current[pos];current[pos]=forward?c.after:c.before;if(l<=c.pos&&c.pos<r)sum+=current[pos];};
 auto ap=[&](std::size_t i,std::int32_t l,std::int32_t r){assign(i,l,r,true);};auto un=[&](std::size_t i,std::int32_t l,std::int32_t r){assign(i,l,r,false);};auto ans=[&](std::size_t id){++answers;const auto q=mo.query(id);require(sum==std::accumulate(versions[q.time].begin()+q.left,versions[q.time].begin()+q.right,std::int64_t{0}));};
 require(mo.run(std::ref(add),std::ref(erase),std::ref(ap),std::ref(un),std::ref(ans),block)==end);require(answers==mo.query_count());
}
// 基数ソートを使う規模で、素朴な集計に一致し、照会の追加順を入れ替えても各照会の答えが変わらないことを確かめる。
std::uint64_t large_answers=0;
void large_cases(){
 for(int trial=0;trial<6;++trial){
  const auto n=static_cast<std::int32_t>(500+rng(2500));const auto count=static_cast<std::size_t>(1023+rng(3000));constexpr int kKinds=30;
  std::vector<int>a(static_cast<std::size_t>(n));for(auto&v:a)v=static_cast<int>(rng(kKinds));
  std::vector<Mo::Query>qs(count);for(auto&q:qs){auto l=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1)),r=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1));if(l>r)std::swap(l,r);if(trial%3==1)r=std::min(n,l+static_cast<std::int32_t>(rng(40)));if(trial%3==2)r=n;q={l,r};}
  std::vector<int>expected(count);for(std::size_t i=0;i<count;++i){std::array<bool,kKinds>seen{};for(auto j=qs[i].left;j<qs[i].right;++j)if(!seen[static_cast<std::size_t>(a[static_cast<std::size_t>(j)])]){seen[static_cast<std::size_t>(a[static_cast<std::size_t>(j)])]=true;++expected[i];}}
  std::vector<std::size_t>perm(count);std::iota(perm.begin(),perm.end(),std::size_t{0});std::shuffle(perm.begin(),perm.end(),random_engine);
  for(const bool shuffled:{false,true}){
   Mo mo(n);std::vector<std::size_t>original(count);for(std::size_t k=0;k<count;++k){const auto index=shuffled?perm[k]:k;original[mo.add_query(qs[index].left,qs[index].right)]=index;}
   for(const std::int32_t b:{0,1,37}){
    std::vector<int>freq(kKinds),got(count,-1);int distinct=0;const auto value=[&](std::int32_t i){return static_cast<std::size_t>(a[static_cast<std::size_t>(i)]);};
    mo.run([&](std::int32_t i){if(freq[value(i)]++==0)++distinct;},[&](std::int32_t i){if(--freq[value(i)]==0)--distinct;},[&](std::size_t id){got[original[id]]=distinct;},b);require(got==expected);
    std::fill(freq.begin(),freq.end(),0);distinct=0;got.assign(count,-1);std::vector<std::size_t>log;std::size_t mark=0;int saved=0;
    const auto undo=[&](std::size_t size){while(log.size()>size){if(--freq[log.back()]==0)--distinct;log.pop_back();}};
    mo.run_rollback([&](std::int32_t i){log.push_back(value(i));if(freq[value(i)]++==0)++distinct;},[&]{mark=log.size();saved=distinct;},[&]{undo(mark);require(distinct==saved);},[&]{undo(0);},[&](std::size_t id){got[original[id]]=distinct;},b);
    require(got==expected&&log.empty()&&distinct==0);large_answers+=2*count;
   }
  }
 }
 for(int trial=0;trial<4;++trial){
  const auto n=static_cast<std::int32_t>(300+rng(500));const auto changes=200+rng(600),count=static_cast<std::size_t>(1024+rng(1500));
  std::vector<int>initial(static_cast<std::size_t>(n));for(auto&v:initial)v=static_cast<int>(rng(201))-100;
  std::vector<Change>log;auto latest=initial;for(std::size_t t=0;t<changes;++t){const auto pos=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)));const int value=static_cast<int>(rng(201))-100;log.push_back({pos,latest[static_cast<std::size_t>(pos)],value});latest[static_cast<std::size_t>(pos)]=value;}
  std::vector<MoWithUpdates::Query>qs(count);for(auto&q:qs){auto l=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1)),r=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1));if(l>r)std::swap(l,r);q={l,r,rng(changes+1)};}
  std::vector<std::size_t>by_time(count);std::iota(by_time.begin(),by_time.end(),std::size_t{0});std::stable_sort(by_time.begin(),by_time.end(),[&](std::size_t x,std::size_t y){return qs[x].time<qs[y].time;});
  std::vector<std::int64_t>expected(count);auto version=initial;std::size_t time=0;
  for(const auto i:by_time){while(time<qs[i].time){version[static_cast<std::size_t>(log[time].pos)]=log[time].after;++time;}expected[i]=std::accumulate(version.begin()+qs[i].left,version.begin()+qs[i].right,std::int64_t{0});}
  std::vector<std::size_t>perm(count);std::iota(perm.begin(),perm.end(),std::size_t{0});std::shuffle(perm.begin(),perm.end(),random_engine);
  for(const bool shuffled:{false,true}){
   MoWithUpdates mo(n);for(const auto& c:log)mo.add_update(c.pos);std::vector<std::size_t>original(count);
   for(std::size_t k=0;k<count;++k){const auto index=shuffled?perm[k]:k;original[mo.add_query(qs[index].left,qs[index].right,qs[index].time)]=index;}
   for(const std::int32_t b:{0,1,53}){
    auto current=initial;std::int64_t sum=0;std::vector<std::int64_t>got(count);
    auto add=[&](std::int32_t i){sum+=current[static_cast<std::size_t>(i)];};auto erase=[&](std::int32_t i){sum-=current[static_cast<std::size_t>(i)];};
    auto assign=[&](std::size_t id,std::int32_t l,std::int32_t r,bool forward){const auto& c=log[id];const bool active=l<=c.pos&&c.pos<r;if(active)erase(c.pos);current[static_cast<std::size_t>(c.pos)]=forward?c.after:c.before;if(active)add(c.pos);};
    mo.run(add,erase,[&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,true);},[&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,false);},[&](std::size_t id){got[original[id]]=sum;},b);
    require(got==expected);large_answers+=count;
   }
  }
 }
}
void checks_all(){
 large_cases();
 for(std::int32_t n=0;n<=8;++n){Mo mo(n);mo.reserve_queries(100);std::vector<int>a(static_cast<std::size_t>(n));std::iota(a.begin(),a.end(),-4);for(std::int32_t l=0;l<=n;++l)for(std::int32_t r=l;r<=n;++r){const auto id=mo.add_query(l,r);require(mo.query(id)==Mo::Query{l,r});mo.add_query(l,r);}for(std::int32_t b=0;b<=n+2;++b){static_case(mo,a,b);rollback_case(mo,a,b);}auto copy=mo;copy.add_query(0,n);require(mo.query_count()+1==copy.query_count());auto* alias=&copy;copy=*alias;copy=std::move(*alias);auto moved=std::move(copy);require(copy.empty()&&copy.length()==0);copy=std::move(moved);require(moved.empty()&&moved.length()==0);copy.clear_requests();require(copy.length()==n&&copy.empty());copy.reset(4);copy.clear();require(copy.length()==0);}
 for(int trial=0;trial<400;++trial){const auto n=static_cast<std::int32_t>(rng(26));MoWithUpdates mo(n);mo.reserve_queries(8);mo.reserve_updates(8);std::vector<int>initial(static_cast<std::size_t>(n));for(auto&v:initial)v=static_cast<int>(rng(201))-100;std::vector<std::vector<int>>versions{initial};std::vector<Change>changes;
  for(int step=0;step<140;++step){if(n&&rng(5)<2){const auto pos=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)));const auto value=static_cast<int>(rng(201))-100;require(mo.add_update(pos)==changes.size());changes.push_back({pos,versions.back()[static_cast<std::size_t>(pos)],value});auto next=versions.back();next[static_cast<std::size_t>(pos)]=value;versions.push_back(std::move(next));}else{auto l=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1)),r=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1));if(l>r)std::swap(l,r);const auto time=rng(changes.size()+1);const auto id=mo.add_query(l,r,time);require(mo.query(id)==MoWithUpdates::Query{l,r,time});if(step%19==0)mo.add_query(l,r,time);}}
  const auto latest_id=mo.add_query(0,n);require(mo.query(latest_id).time==changes.size());
  require(mo.update_count()==changes.size());for(std::size_t i=0;i<changes.size();++i)require(mo.updates()[i]==changes[i].pos);for(const auto b:std::array<std::int32_t,5>{0,1,2,n+1,INT32_MAX})timed_case(mo,initial,changes,versions,b);
  auto copy=mo;copy.add_query(0,n);require(copy.query_count()==mo.query_count()+1);require(copy.update_count()==mo.update_count());auto* alias=&copy;copy=*alias;copy=std::move(*alias);auto moved=std::move(copy);require(copy.empty()&&copy.length()==0&&copy.update_count()==0);copy=std::move(moved);require(moved.empty()&&moved.update_count()==0);copy.clear_requests();require(copy.length()==n&&copy.empty()&&copy.update_count()==0);copy.reset(3);swap(copy,moved);require(moved.length()==3);moved.clear();
 }
 require(applied&&undone&&inside&&outside&&rollback_left_adds&&rollback_resets&&large_answers);
 for(const auto x:std::array<std::size_t,10>{0,1,2,3,63,64,65,SIZE_MAX/2,SIZE_MAX-1,SIZE_MAX}){const auto s=::sirkit::detail::mo_integer_sqrt(x);require(s==0||s<=x/s);require(s+1>x/(s+1));}
 // 自動幅: 静的は整数のfloor(sqrt(2n^2/Q))を[1,n]へ収め、更新版は4QB^3>=3n^2B+12Un^2となる最小のB。
 const auto root=[](std::uint64_t x){auto s=static_cast<std::uint64_t>(std::sqrt(static_cast<long double>(x)));while(s*s>x)--s;while((s+1)*(s+1)<=x)++s;return s;};
 for(const std::int32_t n:{0,1,2,3,64,65,1000,200000,INT32_MAX})for(const std::size_t q:{0,1,2,3,10,1000,100148}){
  Mo mo(n);MoWithUpdates timed(n);mo.reserve_queries(q);timed.reserve_queries(q);for(std::size_t i=0;i<q;++i){mo.add_query(0,0);timed.add_query(0,0,0);}
  const auto square=static_cast<std::uint64_t>(n)*static_cast<std::uint64_t>(n);
  require(static_cast<std::uint64_t>(mo.block_size())==(n==0?std::uint64_t{1}:std::clamp<std::uint64_t>(root(2*square/std::max<std::uint64_t>(1,q)),1,static_cast<std::uint64_t>(n))));
  for(const std::size_t u:{std::size_t{0},std::size_t{1},std::size_t{1000},std::size_t{99852}}){
   while(n&&timed.update_count()<u)timed.add_update(0);
   const auto b=timed.block_size();require(b>=1&&b<=std::max(1,n));if(!n){require(b==1);continue;}
   const long double count=static_cast<long double>(std::max<std::size_t>(1,q)),changes=static_cast<long double>(u),s=static_cast<long double>(square);
   const auto slope=[&](long double x){return 4*count*x*x*x-3*s*x-12*changes*s;};const auto tolerance=1e-9L*(3*s*b+12*changes*s);
   require(b==n||slope(b)>=-tolerance);require(b==1||slope(b-1)<=tolerance);
  }
 }
 {MoWithUpdates sample(200000);for(int i=0;i<100148;++i)sample.add_query(0,0,0);for(int i=0;i<99852;++i)sample.add_update(0);require(sample.block_size()==4948);}
 Mo huge(INT32_MAX);huge.add_query(INT32_MAX,INT32_MAX);int touched=0;const auto state=huge.run([&](std::int32_t){++touched;},[&](std::int32_t){++touched;},[](std::size_t){});require(touched==0&&state==Mo::Window{INT32_MAX,INT32_MAX});MoWithUpdates timed(INT32_MAX);timed.add_update(0);timed.add_query(INT32_MAX,INT32_MAX);std::size_t seen=0;const auto ts=timed.run([&](std::int32_t){++touched;},[&](std::int32_t){++touched;},[&](std::size_t,std::int32_t l,std::int32_t r){require(l==INT32_MAX&&l==r);++seen;},[](std::size_t,std::int32_t,std::int32_t){},[](std::size_t){});require(seen==1&&touched==0&&ts.time==1);
 Mo move_only(3);move_only.add_query(0,3);int move_sum=0;auto owned_add=[owner=std::make_unique<int>(0),&move_sum](std::int32_t i){++*owner;move_sum+=i+1;};move_only.run(std::move(owned_add),[&](std::int32_t i){move_sum-=i+1;},[&](std::size_t){require(move_sum==6);});
 Mo empty;require(empty.order().empty());require(empty.run([](std::int32_t){},[](std::int32_t){},[](std::size_t){})==Mo::Window{0,0});MoWithUpdates no_query(1);no_query.add_update(0);require(no_query.empty());require(no_query.run([](std::int32_t){},[](std::int32_t){},[](std::size_t,std::int32_t,std::int32_t){std::abort();},[](std::size_t,std::int32_t,std::int32_t){std::abort();},[](std::size_t){})==MoWithUpdates::State{0,0,0});
 {std::size_t touched_rollback=0,answered=0;huge.run_rollback([&](std::int32_t){++touched_rollback;},[]{},[]{},[&]{++touched_rollback;},[&](std::size_t){++answered;});huge.run_rollback([&](std::int32_t){++touched_rollback;},[]{},[]{},[&]{++touched_rollback;},[&](std::size_t){++answered;},1);require(touched_rollback==0&&answered==2);}
 {Mo owned(4);owned.add_query(0,4);owned.add_query(1,2);int total=0,saved_total=0;auto owned_push=[owner=std::make_unique<int>(0),&total](std::int32_t i){++*owner;total+=i+1;};std::vector<int>seen_totals;owned.run_rollback(std::move(owned_push),[&]{saved_total=total;},[&]{total=saved_total;},[&]{total=0;},[&](std::size_t id){seen_totals.push_back(static_cast<int>(id)*100+total);},1);require((seen_totals==std::vector<int>{10,102}));}
 {bool called=false;empty.run_rollback([&](std::int32_t){called=true;},[&](std::int32_t){called=true;},[&]{called=true;},[&]{called=true;},[&]{called=true;},[&](std::size_t){called=true;});require(!called);}
 Mo first(3), second(5);
 first.add_query(0,3); second.add_query(1,5);
 using std::swap;
 swap(first,second);
 require(first.length()==5&&first.query(0)==Mo::Query{1,5});
 require(second.length()==3&&second.query(0)==Mo::Query{0,3});
 rejected([&]{huge.reserve_queries(SIZE_MAX);});rejected([&]{timed.reserve_updates(SIZE_MAX);});
#ifndef SKIP_RUNTIME_CHECKS
 rejected([]{Mo bad(-1);});rejected([]{MoWithUpdates bad(-1);});rejected([&]{huge.add_query(-1,0);});rejected([&]{huge.add_query(2,1);});rejected([&]{empty.add_query(0,1);});rejected([&]{empty.query(0);});rejected([&]{huge.order(-1);});rejected([&]{huge.run_rollback([](std::int32_t){},[]{},[]{},[]{},[](std::size_t){},-1);});rejected([&]{huge.block_size(-1);});rejected([&]{timed.add_update(-1);});rejected([&]{timed.add_update(INT32_MAX);});rejected([&]{timed.add_query(0,0,2);});rejected([&]{timed.update_position(1);});rejected([&]{timed.query(SIZE_MAX);});rejected([&]{timed.order(-1);});
#endif
}
int main(){checks_all();std::cout<<"Mo independent checks="<<checks<<" apply="<<applied<<" undo="<<undone<<" inside="<<inside<<" outside="<<outside<<'\n';}
