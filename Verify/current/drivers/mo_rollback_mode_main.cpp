// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
int main(){
    std::int32_t n{},q{};verify_in>>n>>q;
    std::vector<std::int32_t>values(static_cast<std::size_t>(n));for(auto&v:values)verify_in>>v;
    auto keys=values;std::sort(keys.begin(),keys.end());keys.erase(std::unique(keys.begin(),keys.end()),keys.end());
    for(auto&v:values)v=static_cast<std::int32_t>(std::lower_bound(keys.begin(),keys.end(),v)-keys.begin());
    Mo mo(n);mo.reserve_queries(static_cast<std::size_t>(q));
    for(std::int32_t i=0;i<q;++i){std::int32_t l{},r{};verify_in>>l>>r;mo.add_query(l,r);}
    // 出現回数と最頻値は追加だけで更新し、追加した値の記録を逆にたどって巻き戻す。
    std::vector<std::int32_t>count(keys.size()),added,modes(mo.query_count()),times(mo.query_count());added.reserve(static_cast<std::size_t>(n));
    std::int32_t best=0,mode=0,saved_best=0,saved_mode=0;std::size_t mark=0;
    auto undo_to=[&](std::size_t size){while(added.size()>size){--count[static_cast<std::size_t>(added.back())];added.pop_back();}};
    mo.run_rollback([&](std::int32_t i){const auto v=values[static_cast<std::size_t>(i)];added.push_back(v);if(++count[static_cast<std::size_t>(v)]>best){best=count[static_cast<std::size_t>(v)];mode=v;}},
                    [&]{mark=added.size();saved_best=best;saved_mode=mode;},
                    [&]{undo_to(mark);best=saved_best;mode=saved_mode;},
                    [&]{undo_to(0);best=0;},
                    [&](std::size_t id){modes[id]=mode;times[id]=best;});
    for(std::size_t i=0;i<modes.size();++i)verify_out<<keys[static_cast<std::size_t>(modes[i])]<<' '<<times[i]<<'\n';
}
