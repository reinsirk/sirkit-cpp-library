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
    Mo mo(n);
    for(std::int32_t i=0;i<q;++i){std::int32_t l{},r{};verify_in>>l>>r;mo.add_query(l,r);}
    std::vector<std::int32_t>frequency(keys.size()),answers(mo.query_count());std::int32_t distinct=0;
    auto add=[&](std::int32_t i){if(frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])]++==0)++distinct;};
    auto erase=[&](std::int32_t i){if(--frequency[static_cast<std::size_t>(values[static_cast<std::size_t>(i)])]==0)--distinct;};
    mo.run(add,erase,[&](std::size_t id){answers[id]=distinct;});
    for(const auto answer:answers)verify_out<<answer<<'\n';
}
