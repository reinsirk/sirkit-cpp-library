// SPDX-License-Identifier: CC0-1.0
#include "wuf_groups_test_model.hpp"
template<class T> void run() {
    for(int n:{0,1,7,17,255,256,257,263,511,512,513,4097})for(int b:{1,8,128,8192})for(int order=0;order<5;++order){
        UnionFindW<T> uf(n);Model m(n,b,order);m.build(uf);
        // 最初のgroupsより前には、照会で経路を圧縮しない。
        const auto expected=m.groups();
        require(uf.groups()==expected);
        if constexpr(std::is_same_v<T,Ring>)Ring::additions=0;
        m.observe(uf);
        if constexpr(std::is_same_v<T,Ring>)require(Ring::additions==0);
        require(uf.groups()==expected);
        if(n>1&&m.components>1){const int a=m.permutation[0],c=m.permutation[n-1];
            require(uf.merge(a,c,value<T>(m.potentials[c]-m.potentials[a])));m.join(a,c);
            require(uf.groups()==m.groups());m.observe(uf);
        }
        auto copy=uf;require(copy.groups()==m.groups());m.observe(copy);
        uf.reset();require(uf.component_count()==n);for(int i=0;i<n;++i)require(uf.weight(i)==T{});
    }
}
int main(){run<std::int64_t>();run<std::uint8_t>();run<Ring>();std::printf("PASS cold groups checks=%llu\n",static_cast<unsigned long long>(checks));}
