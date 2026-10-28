// SPDX-License-Identifier: CC0-1.0
// SCC処理・warm距離・DFS祖先判定を独立Floyd参照と照合する。
#ifndef SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#define SIRKIT_TEST_NEGATIVE_PATH_SOURCE "../../../Graph/Bellman-Ford.cpp"
#endif
#include SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

static std::uint64_t checks=0,cases=0;
static void require(bool condition){++checks;if(!condition)throw std::runtime_error("SCC independent review mismatch");}
static std::vector<std::int64_t> reference(const WGraph& g,int source){
    int n=int(g.size());std::vector<std::vector<std::int64_t>> d(n,std::vector<std::int64_t>(n,INF));
    for(int i=0;i<n;++i){d[i][i]=0;for(const auto&e:g[i])d[i][e.to]=std::min(d[i][e.to],e.w);}
    for(int k=0;k<n;++k)for(int i=0;i<n;++i)if(d[i][k]!=INF)for(int j=0;j<n;++j)if(d[k][j]!=INF)
        d[i][j]=std::min(d[i][j],std::max(-INF/4,d[i][k]+d[k][j]));
    auto result=d[source];
    for(int k=0;k<n;++k)if(d[source][k]!=INF&&d[k][k]<0)for(int j=0;j<n;++j)if(d[k][j]!=INF)result[j]=-INF;
    return result;
}
static void test(const WGraph&g,int source,int warm,const std::vector<std::int64_t>*known=nullptr){
    ++cases;const int n=int(g.size());auto expected=known?*known:reference(g,source);
    const std::int64_t lower=-std::int64_t(n)*1'000'000'000;
    std::vector<std::int64_t> distance(n,INF);distance[source]=0;
    for(int pass=0;pass<warm;++pass)for(int u=0;u<n;++u)if(distance[u]!=INF)
        for(const auto&e:g[u])distance[e.to]=std::min(distance[e.to],distance[u]+e.w);
    std::vector<int> sources,negative;
    for(int i=0;i<n;++i)if(!g[i].empty())sources.push_back(i);
#ifndef SKIP_RUNTIME_CHECKS
    std::vector<std::uint8_t> checked(n,0);
#endif
    reinsirk::detail::BellmanFordComponents(g,sources,distance,INF,lower,negative
#ifndef SKIP_RUNTIME_CHECKS
        ,checked
#endif
    );
    for(auto&d:distance)if(d==lower)d=-INF;
    require(distance==expected);
    std::vector<std::int64_t> actual{777,888};bool found=BellmanFord(g,actual,source);
    require(actual==expected);require(found==(std::find(expected.begin(),expected.end(),-INF)!=expected.end()));
}
int main(){
    std::mt19937 rng(20261025);
    for(int trial=0;trial<1500;++trial){
        int n=1+int(rng()%85);WGraph g(n);std::vector<int> potential(n);
        for(auto&x:potential)x=int(rng()%101)-50;
        for(int e=0,m=int(rng()%(5*n+1));e<m;++e){int u=int(rng()%n),v=int(rng()%n);
            std::int64_t w=int(rng()%21)-10;
            if(trial%3==0)w=int(rng()%11)+potential[v]-potential[u];
            if(trial%3==1&&u>=v)continue;
            g[u].emplace_back(v,w);
        }
        test(g,int(rng()%n),int(rng()%20));
    }
    // A ring with arbitrary warm entry points: two rounds must settle all entries.
    for(int size:{1,2,3,16,65,127})for(int kind:{0,1,2,3}){
        WGraph g(size+2);const int start=size,sink=size+1;
        for(int i=0;i<size;++i){const std::int64_t w=i==0?-std::int64_t(size-1)*7:7;
            g[i].emplace_back((i+1)%size,w+(kind==0&&i==0?-1:kind==1&&i==0?1:0));
            if(kind==3)g[i].emplace_back(sink,100);
            g[start].emplace_back(i,std::int64_t(rng()%1000));
        }
        test(g,start,0);test(g,start,17);
    }
    // Stack-safe Tarjan, negative terminal SCC, and backward topological numbering.
    for(int n:{65,20000}){
        WGraph g(n);for(int i=1;i<n;++i)g[i].emplace_back(i-1,-1);
        std::vector<std::int64_t> expected(n);for(int i=0;i<n;++i)expected[i]=-std::int64_t(n-1-i);
        test(g,n-1,0,&expected);
        g[n/2].emplace_back(n/2,-1);for(int i=0;i<=n/2;++i)expected[i]=-INF;
        test(g,n-1,0,&expected);
    }
    // Aを先にDFSで完了させ、B→Aの交差辺で負閉路を作る。木の祖先辺だけでは見つからない。
    for(int k:{1,2,3,8,32,64,128,1249})for(bool nonnegative:{false,true}){
        const int n=2*k+2;WGraph g(n);g[0].emplace_back(1,0);
        for(int i=0;i<k;++i)g[1].emplace_back(i+2,1);
        for(int i=0;i<k;++i)g[1].emplace_back(i+k+2,0);
        for(int i=0;i<k;++i){g[i+2].emplace_back(1,nonnegative?1:0);g[i+k+2].emplace_back(i+2,-1);}
        // 中心→B→A→中心は-1/0。始点へ戻る辺はない。非負版のAの最短値は-1。
        std::vector<std::int64_t> expected(n,-INF);expected[0]=0;
        if(nonnegative){expected.assign(n,0);for(int i=0;i<k;++i)expected[i+2]=-1;}
        if(k<=64)require(reference(g,0)==expected);
        test(g,0,0,&expected);test(g,0,17,&expected);
    }
    // 成分内の有限距離が7/8/9/10パスで確定する。全閉路の換算重みは0。
    for(int gap:{7,8,9,10})for(bool shifted:{false,true}){
        constexpr int size=20;WGraph g(size+1);std::vector<std::int64_t> expected(size+1);
        if(shifted)for(int i=1;i<=size;++i)expected[i]=i%7-3;
        const auto add=[&](int from,int to){g[from].emplace_back(to,expected[to]-expected[from]);};
        add(0,1);for(int i=gap+1;i<=size;++i)add(0,i);
        for(int i=1;i<=size;++i)add(i,i==size?1:i+1);
        add(1,2); // 多重辺で単一閉路専用経路を避ける。
        require(reference(g,0)==expected);
        test(g,0,0,&expected);test(g,0,17,&expected);
    }
    // 負閉路なしでも、多数の厳密更新と前駆検査を要する逆向きの距離改善波。
    for(int size:{8,9,16,64,128,2499})for(bool shifted:{false,true}){
        WGraph g(size+1);std::vector<std::int64_t> potential(size+1),expected(size+1,1);
        if(shifted)for(int i=1;i<=size;++i)potential[i]=i%7-3;
        expected[0]=0;for(int i=1;i<=size;++i)expected[i]+=potential[i];
        for(int i=size;i>=1;--i)g[0].emplace_back(i,size+1-i+potential[i]);
        for(int i=1;i<=size;++i){const int to=i==1?size:i-1;g[i].emplace_back(to,potential[to]-potential[i]);}
        g[size].emplace_back(size-1,potential[size-1]-potential[size]);
        // 始点からsizeへ費用1で入り、換算重み0の閉路で全頂点へ到達する。
        if(size<=64)require(reference(g,0)==expected);
        test(g,0,0,&expected);test(g,0,17,&expected);
    }
    // 独立な負閉路群でfrontierを増やし、遅い非負成分も公開APIのSCC切替へ到達させる。
    for(int size:{64,128,2368}){
        constexpr int count=64;const int center=size+1,entry=size+2*count+2,delay=entry+1;
        WGraph g(delay+1);
        for(int i=size;i>=1;--i)g[0].emplace_back(i,size+1-i);
        for(int i=1;i<=size;++i)g[i].emplace_back(i==1?size:i-1,0);
        g[size].emplace_back(size-1,0);g[0].emplace_back(delay,0);
        g[delay].emplace_back(entry,0);g[entry].emplace_back(center,0);
        for(int i=0;i<count;++i)g[center].emplace_back(size+2+i,1);
        for(int i=0;i<count;++i)g[center].emplace_back(size+count+2+i,0);
        for(int i=0;i<count;++i){g[size+2+i].emplace_back(center,0);g[size+count+2+i].emplace_back(size+2+i,-1);}
        std::vector<std::int64_t> expected(g.size(),-INF);expected[0]=expected[entry]=expected[delay]=0;
        for(int i=1;i<=size;++i)expected[i]=1;
        if(size<=128)require(reference(g,0)==expected);
        test(g,0,0,&expected);test(g,0,17,&expected);
    }
    // 初期activeの逆順を常用すると遅くなる非負成分。元の順序なら1パスで確定する。
    for(int count:{8,9,64,128,1248})for(bool shifted:{false,true}){
        WGraph g(count+2);std::vector<std::int64_t> potential(g.size()),expected(g.size(),1);
        if(shifted)for(int i=1;i<int(g.size());++i)potential[i]=i%7-3;
        expected[0]=0;expected[1]=potential[1];
        for(int i=2;i<int(g.size());++i)expected[i]+=potential[i];
        const auto add=[&](int from,int to,std::int64_t cost){g[from].emplace_back(to,cost+potential[to]-potential[from]);};
        add(0,1,0);for(int i=1;i<=count;++i)add(0,i+1,count+1-i);
        for(int i=1;i<=count;++i)add(1,i+1,count);
        for(int i=1;i<=count;++i){add(i+1,1,0);if(i>=2)add(i+1,i,0);}
        if(count<=64)require(reference(g,0)==expected);
        test(g,0,0,&expected);test(g,0,17,&expected);
    }
    // 多数の負成分の下流を長鎖・終点まで伝播し、有限枝と未到達成分を保つ。
    for(int size:{160,1024})for(int permutation:{0,1,2}){
        WGraph g(size);std::vector<int> label(size);std::iota(label.begin(),label.end(),0);
        if(permutation==1)std::reverse(label.begin(),label.end());
        if(permutation==2)std::shuffle(label.begin(),label.end(),rng);
        const auto add=[&](int from,int to,std::int64_t weight){g[label[from]].emplace_back(label[to],weight);};
        std::vector<std::int64_t> expected(size,-INF);
        expected[label[0]]=0;expected[label[size-3]]=7;expected[label[size-2]]=12;expected[label[size-1]]=INF;
        for(int i=1;i<=64;++i){add(0,i,i%3);add(i,i,-1);add(i,size-4,0);}
        for(int i=size-4;i>65;--i)add(i,i-1,0);
        add(0,size-3,7);add(size-3,size-2,5);add(size-1,size-1,-1);
        if(size==160)require(reference(g,label[0])==expected);
        test(g,label[0],0,&expected);test(g,label[0],17,&expected);
    }
    // Tarjanのactive内に残っていても、DFS祖先でない頂点への辺は閉路証拠にしない。
    {
        WGraph g(3);g[0].emplace_back(1,100);g[1].emplace_back(0,0);
        g[0].emplace_back(2,0);g[2].emplace_back(1,0);
        test(g,0,0);test(g,0,17);
    }
#ifndef SKIP_RUNTIME_CHECKS
    // Existing lower-bound seeds can precede SCC dispatch; their unseen descendants still need validation.
    for(int kind=0;kind<4;++kind){
        const int n=80;WGraph g(n);for(int j=0;j<=n;++j)g[0].emplace_back(0,-1'000'000'000);
        g[0].emplace_back(n-2,0);for(int i=n-2;i>1;--i)g[i].emplace_back(i-1,0);
        g[1].emplace_back(kind==0?-1:kind==1?n:0,kind==2?-1'000'000'001:kind==3?1'000'000'001:0);
        // An unreachable invalid edge must not be inspected.
        g[n-1].emplace_back(n+10,1'000'000'001);
        std::vector<std::int64_t> d{123,456};bool threw=false;
        try{BellmanFord(g,d,0);}catch(const std::out_of_range&){threw=true;}
        require(threw);require(d==std::vector<std::int64_t>({123,456}));
        // SCC切替前から負影響が判明していても、下流の検査を省かない。
        std::vector<int> sources,negative{0};
        for(int i=0;i<n;++i)if(!g[i].empty())sources.push_back(i);
        const auto lower=-std::int64_t(n)*1'000'000'000;
        d.assign(n,INF);d[0]=lower;
        std::vector<std::uint8_t> checked(n,0);threw=false;
        try{reinsirk::detail::BellmanFordComponents(g,sources,d,INF,lower,negative,checked);}
        catch(const std::out_of_range&){threw=true;}
        require(threw);
    }
#endif
    {
        const int n=80;WGraph g(n);
        for(int j=0;j<=n;++j)g[0].emplace_back(0,-1'000'000'000);
        g[0].emplace_back(n-2,0);for(int i=n-2;i>1;--i)g[i].emplace_back(i-1,0);
        g[n-1].emplace_back(n+10,1'000'000'001);
        std::vector<std::int64_t> expected(n,-INF);expected[n-1]=INF;
        test(g,0,0,&expected);
    }
    std::cout<<"{\"cases\":"<<cases<<",\"checks\":"<<checks<<"}\n";
}
