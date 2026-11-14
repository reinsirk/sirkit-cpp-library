// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Mo.cpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using namespace sirkit;
struct Change {std::int32_t position;std::int64_t before,after;};
int main(){
    const std::vector<std::int64_t>a{2,7,1,8,2};Mo mo(5);
    mo.add_query(1,4);mo.add_query(0,5);mo.add_query(2,2);mo.add_query(1,4);
    for(const auto id:mo.order()){std::cout<<id<<' ';}std::cout<<'\n';
    std::int64_t sum=0;std::vector<std::int64_t>answers(mo.query_count());
    auto add=[&](std::int32_t i){sum+=a[static_cast<std::size_t>(i)];};
    auto erase=[&](std::int32_t i){sum-=a[static_cast<std::size_t>(i)];};
    const auto window=mo.run(add,erase,[&](std::size_t id){answers[id]=sum;});
    for(const auto answer:answers){std::cout<<answer<<' ';}std::cout<<'\n';
    std::cout<<window.left<<' '<<window.right<<' '<<mo.block_size()<<'\n';

    std::vector<std::int64_t>values{1,2,3,4};const std::vector<Change>changes{{1,2,5},{3,4,0}};
    MoWithUpdates timed(4);timed.reserve_queries(5);timed.reserve_updates(2);
    timed.add_query(0,4);timed.add_update(1);timed.add_query(1,3);timed.add_update(3);timed.add_query(0,4);timed.add_query(0,4,0);timed.add_query(3,3);
    sum=0;answers.assign(timed.query_count(),0);
    auto insert=[&](std::int32_t i){sum+=values[static_cast<std::size_t>(i)];};auto remove=[&](std::int32_t i){sum-=values[static_cast<std::size_t>(i)];};
    auto assign=[&](std::size_t id,std::int32_t l,std::int32_t r,bool forward){const auto c=changes[id];const bool active=l<=c.position&&c.position<r;if(active)remove(c.position);values[static_cast<std::size_t>(c.position)]=forward?c.after:c.before;if(active)insert(c.position);};
    auto apply=[&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,true);};auto undo=[&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,false);};
    const auto state=timed.run(insert,remove,apply,undo,[&](std::size_t id){answers[id]=sum;});
    for(const auto answer:answers){std::cout<<answer<<' ';}std::cout<<'\n';
    std::cout<<state.left<<' '<<state.right<<' '<<state.time<<'\n';
    for(auto i=state.left;i<state.right;++i)remove(i);
    for(auto t=state.time;t;--t)undo(t-1,state.left,state.left);
    std::cout<<values[1]<<' '<<values[3]<<'\n';
    std::cout<<timed.queries().size()<<' '<<timed.updates().size()<<' '<<timed.update_position(0)<<'\n';
    auto copy=timed;copy.clear_requests();auto moved=std::move(copy);
    std::cout<<copy.length()<<' '<<copy.empty()<<' '<<moved.length()<<' '<<moved.empty()<<'\n';
    copy.reset(1);copy.add_update(0);copy.add_query(0,1);copy.clear();
}
