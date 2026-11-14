// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_MO_SOURCE
#define SIRKIT_MO_SOURCE "../../../Algorithm/Mo.cpp"
#endif
#include SIRKIT_MO_SOURCE
#include <array>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <set>
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
void checks_all(){
 for(std::int32_t n=0;n<=8;++n){Mo mo(n);mo.reserve_queries(100);std::vector<int>a(static_cast<std::size_t>(n));std::iota(a.begin(),a.end(),-4);for(std::int32_t l=0;l<=n;++l)for(std::int32_t r=l;r<=n;++r){const auto id=mo.add_query(l,r);require(mo.query(id)==Mo::Query{l,r});mo.add_query(l,r);}for(std::int32_t b=0;b<=n+2;++b)static_case(mo,a,b);auto copy=mo;copy.add_query(0,n);require(mo.query_count()+1==copy.query_count());auto* alias=&copy;copy=*alias;copy=std::move(*alias);auto moved=std::move(copy);require(copy.empty()&&copy.length()==0);copy=std::move(moved);require(moved.empty()&&moved.length()==0);copy.clear_requests();require(copy.length()==n&&copy.empty());copy.reset(4);copy.clear();require(copy.length()==0);}
 for(int trial=0;trial<400;++trial){const auto n=static_cast<std::int32_t>(rng(26));MoWithUpdates mo(n);mo.reserve_queries(8);mo.reserve_updates(8);std::vector<int>initial(static_cast<std::size_t>(n));for(auto&v:initial)v=static_cast<int>(rng(201))-100;std::vector<std::vector<int>>versions{initial};std::vector<Change>changes;
  for(int step=0;step<140;++step){if(n&&rng(5)<2){const auto pos=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)));const auto value=static_cast<int>(rng(201))-100;require(mo.add_update(pos)==changes.size());changes.push_back({pos,versions.back()[static_cast<std::size_t>(pos)],value});auto next=versions.back();next[static_cast<std::size_t>(pos)]=value;versions.push_back(std::move(next));}else{auto l=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1)),r=static_cast<std::int32_t>(rng(static_cast<std::size_t>(n)+1));if(l>r)std::swap(l,r);const auto time=rng(changes.size()+1);const auto id=mo.add_query(l,r,time);require(mo.query(id)==MoWithUpdates::Query{l,r,time});if(step%19==0)mo.add_query(l,r,time);}}
  const auto latest_id=mo.add_query(0,n);require(mo.query(latest_id).time==changes.size());
  require(mo.update_count()==changes.size());for(std::size_t i=0;i<changes.size();++i)require(mo.updates()[i]==changes[i].pos);for(const auto b:std::array<std::int32_t,5>{0,1,2,n+1,INT32_MAX})timed_case(mo,initial,changes,versions,b);
  auto copy=mo;copy.add_query(0,n);require(copy.query_count()==mo.query_count()+1);require(copy.update_count()==mo.update_count());auto* alias=&copy;copy=*alias;copy=std::move(*alias);auto moved=std::move(copy);require(copy.empty()&&copy.length()==0&&copy.update_count()==0);copy=std::move(moved);require(moved.empty()&&moved.update_count()==0);copy.clear_requests();require(copy.length()==n&&copy.empty()&&copy.update_count()==0);copy.reset(3);swap(copy,moved);require(moved.length()==3);moved.clear();
 }
 require(applied&&undone&&inside&&outside);
 for(const auto x:std::array<std::size_t,10>{0,1,2,3,63,64,65,SIZE_MAX/2,SIZE_MAX-1,SIZE_MAX}){const auto s=::sirkit::detail::mo_integer_sqrt(x);require(s==0||s<=x/s);require(s+1>x/(s+1));}
 for(const std::int32_t n:{0,1,2,3,64,65,INT32_MAX}){MoWithUpdates mo(n);const auto b=mo.block_size();require(b>=1);if(n){const auto square=static_cast<std::uint64_t>(n)*static_cast<std::uint64_t>(n);const auto value=static_cast<std::uint64_t>(b);require(value*value*value<=square);require((value+1)*(value+1)*(value+1)>square);}}
 Mo huge(INT32_MAX);huge.add_query(INT32_MAX,INT32_MAX);int touched=0;const auto state=huge.run([&](std::int32_t){++touched;},[&](std::int32_t){++touched;},[](std::size_t){});require(touched==0&&state==Mo::Window{INT32_MAX,INT32_MAX});MoWithUpdates timed(INT32_MAX);timed.add_update(0);timed.add_query(INT32_MAX,INT32_MAX);std::size_t seen=0;const auto ts=timed.run([&](std::int32_t){++touched;},[&](std::int32_t){++touched;},[&](std::size_t,std::int32_t l,std::int32_t r){require(l==INT32_MAX&&l==r);++seen;},[](std::size_t,std::int32_t,std::int32_t){},[](std::size_t){});require(seen==1&&touched==0&&ts.time==1);
 Mo move_only(3);move_only.add_query(0,3);int move_sum=0;auto owned_add=[owner=std::make_unique<int>(0),&move_sum](std::int32_t i){++*owner;move_sum+=i+1;};move_only.run(std::move(owned_add),[&](std::int32_t i){move_sum-=i+1;},[&](std::size_t){require(move_sum==6);});
 Mo empty;require(empty.order().empty());require(empty.run([](std::int32_t){},[](std::int32_t){},[](std::size_t){})==Mo::Window{0,0});MoWithUpdates no_query(1);no_query.add_update(0);require(no_query.empty());require(no_query.run([](std::int32_t){},[](std::int32_t){},[](std::size_t,std::int32_t,std::int32_t){std::abort();},[](std::size_t,std::int32_t,std::int32_t){std::abort();},[](std::size_t){})==MoWithUpdates::State{0,0,0});
 rejected([&]{huge.reserve_queries(SIZE_MAX);});rejected([&]{timed.reserve_updates(SIZE_MAX);});
#ifndef SKIP_RUNTIME_CHECKS
 rejected([]{Mo bad(-1);});rejected([]{MoWithUpdates bad(-1);});rejected([&]{huge.add_query(-1,0);});rejected([&]{huge.add_query(2,1);});rejected([&]{empty.add_query(0,1);});rejected([&]{empty.query(0);});rejected([&]{huge.order(-1);});rejected([&]{huge.block_size(-1);});rejected([&]{timed.add_update(-1);});rejected([&]{timed.add_update(INT32_MAX);});rejected([&]{timed.add_query(0,0,2);});rejected([&]{timed.update_position(1);});rejected([&]{timed.query(SIZE_MAX);});rejected([&]{timed.order(-1);});
#endif
}
int main(){checks_all();std::cout<<"Mo independent checks="<<checks<<" apply="<<applied<<" undo="<<undone<<" inside="<<inside<<" outside="<<outside<<'\n';}
