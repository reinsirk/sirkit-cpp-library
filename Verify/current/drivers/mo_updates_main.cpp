// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
struct Change {std::int32_t position,before,after;};
int main(){
    std::int32_t n{},q{};verify_in>>n>>q;
    std::vector<std::int32_t>values(static_cast<std::size_t>(n));for(auto&v:values)verify_in>>v;
    auto latest=values,keys=values;std::vector<Change>changes;std::vector<std::int32_t>targets;MoWithUpdates mo(n);
    for(std::int32_t i=0;i<q;++i){
        int type{};verify_in>>type;
        if(type==0){std::int32_t position{},value{};verify_in>>position>>value;changes.push_back({position,latest[static_cast<std::size_t>(position)],value});latest[static_cast<std::size_t>(position)]=value;keys.push_back(value);mo.add_update(position);}
        else{std::int32_t l{},r{},value{};verify_in>>l>>r>>value;mo.add_query(l,r);targets.push_back(value);keys.push_back(value);}
    }
    std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());
    auto compress=[&](std::int32_t value){return static_cast<std::int32_t>(std::lower_bound(keys.begin(),keys.end(),value)-keys.begin());};
    for(auto&v:values)v=compress(v);
    for(auto&c:changes){c.before=compress(c.before);c.after=compress(c.after);}
    for(auto&v:targets)v=compress(v);
    std::vector<std::int32_t>frequency(keys.size()),answers(mo.query_count());
    auto add=[&](std::int32_t i){++frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])];};
    auto erase=[&](std::int32_t i){--frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])];};
    auto assign=[&](std::size_t id,std::int32_t l,std::int32_t r,bool forward){const auto&c=changes[id];const bool active=l<=c.position&&c.position<r;if(active)erase(c.position);values[static_cast<std::size_t>(c.position)]=forward?c.after:c.before;if(active)add(c.position);};
    mo.run(add,erase,
           [&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,true);},
           [&](std::size_t id,std::int32_t l,std::int32_t r){assign(id,l,r,false);},
           [&](std::size_t id){answers[id]=frequency[static_cast<std::size_t>(targets[id])];});
    for(const auto answer:answers)verify_out<<answer<<'\n';
}
