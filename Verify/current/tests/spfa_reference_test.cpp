// SPDX-License-Identifier: CC0-1.0
// 全距離・有限枝・未到達・負閉路を独立Floyd参照と照合する。
#ifndef SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#define SIRKIT_TEST_NEGATIVE_PATH_SOURCE "../../../Graph/Bellman-Ford_SFPA.cpp"
#endif
#include SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>
static std::uint64_t cases=0,checks=0;
static void require(bool b){++checks;if(!b)throw std::runtime_error("SPFA supplemental reference mismatch");}
static std::vector<std::int64_t> reference(const WGraph&g,int start){
    const int n=int(g.size());std::vector<std::vector<std::int64_t>> d(n,std::vector<std::int64_t>(n,INF));
    for(int i=0;i<n;++i){d[i][i]=0;for(auto e:g[i])d[i][e.to]=std::min(d[i][e.to],e.w);}
    for(int k=0;k<n;++k)for(int i=0;i<n;++i)if(d[i][k]!=INF)for(int j=0;j<n;++j)if(d[k][j]!=INF)
        d[i][j]=std::min(d[i][j],std::max(-INF/4,d[i][k]+d[k][j]));
    auto out=d[start];for(int k=0;k<n;++k)if(d[start][k]!=INF&&d[k][k]<0)
        for(int j=0;j<n;++j)if(d[k][j]!=INF)out[j]=-INF;
    return out;
}
static void test(const WGraph&g,int start,const std::vector<std::int64_t>*known=nullptr){
    ++cases;auto expected=known?*known:reference(g,start);const bool negative=std::find(expected.begin(),expected.end(),-INF)!=expected.end();
    std::vector<std::int64_t> actual{333,444};require(SPFA(g,actual,start)==negative);require(actual==expected);
    actual={555};require((reinsirk::detail::SPFARun<false,false>(g,actual,start))==negative);require(actual==expected);
}
int main(){
    std::mt19937 rng(20261026);
    for(int trial=0;trial<160;++trial){
        const int n=65+int(rng()%64);WGraph g(n);std::vector<int> p(n),label(n);std::iota(label.begin(),label.end(),0);
        std::shuffle(label.begin(),label.end(),rng);for(auto&x:p)x=int(rng()%2001)-1000;
        for(int u=0;u<n;++u)for(int e=0;e<8;++e){int v=int(rng()%n);if(v==u)v=(v+1)%n;
            std::int64_t w=int(rng()%7)-3;
            if(trial%4==0)w=int(rng()%2)+p[v]-p[u];
            if(trial%4==1){if(v<u)continue;w=p[v]-p[u];}
            if(trial%4==2)w=1+p[v]-p[u];
            g[label[u]].emplace_back(label[v],w);
        }
        test(g,label[0]);
    }
    // 完全なDAGで、長い有限経路による多数の更新を起こす。
    for(int n:{65,96,128})for(bool permute:{false,true}){
        WGraph g(n);std::vector<int> label(n);std::iota(label.begin(),label.end(),0);
        if(permute)std::shuffle(label.begin(),label.end(),rng);
        for(int u=0;u<n;++u)for(int v=n-1;v>u;--v)g[label[u]].emplace_back(label[v],-1);
        test(g,label[0]);
    }
    // 初期DFSで見えにくい負閉路、有限枝、未到達の別成分を同時に持たせる。
    for(int count:{32,64,128,1247})for(bool shifted:{false,true}){
        const int n=2*count+6,finite=n-3,sink=n-2,unreachable=n-1;WGraph g(n);
        std::vector<std::int64_t> potential(n);if(shifted)for(int i=0;i<n;++i)potential[i]=i%13-6;
        const auto add=[&](int u,int v,std::int64_t w){g[u].emplace_back(v,w+potential[v]-potential[u]);};
        add(0,1,0);for(int i=0;i<count;++i)add(1,2+i,1);
        for(int i=0;i<count;++i)add(1,2+count+i,0);
        for(int i=0;i<count;++i){add(2+i,1,0);add(2+count+i,2+i,-1);}
        add(0,finite,7);add(finite,sink,5);add(unreachable,unreachable,-1);
        std::vector<std::int64_t> expected(n,-INF);expected[0]=0;expected[2*count+2]=INF;
        expected[finite]=7+potential[finite]-potential[0];expected[sink]=12+potential[sink]-potential[0];expected[unreachable]=INF;
        if(count<=32)require(reference(g,0)==expected);
        test(g,0,&expected);
    }
#ifndef SKIP_RUNTIME_CHECKS
    // 大量の再処理後に初めて参照する辺も検査し、旧出力を保つ。
    for(int kind=0;kind<4;++kind){
        constexpr int n=1600;WGraph g(n);
        for(int i=1;i<=64;i+=2){g[0].emplace_back(i,0);g[i].emplace_back(i+1,-1);g[i+1].emplace_back(i,0);}
        g[0].emplace_back(65,0);for(int i=65;i<n-1;++i)g[i].emplace_back(i+1,0);
        g[n-1].emplace_back(kind==0?-1:kind==1?n:0,kind==2?-1'000'000'001:kind==3?1'000'000'001:0);
        std::vector<std::int64_t> actual{111,222};bool threw=false;
        try{SPFA(g,actual,0);}catch(const std::out_of_range&){threw=true;}
        require(threw);require(actual==std::vector<std::int64_t>({111,222}));
    }
#endif
    std::cout<<"{\"cases\":"<<cases<<",\"checks\":"<<checks<<"}\n";
}
