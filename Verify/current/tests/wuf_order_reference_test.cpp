// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_WUF_SOURCE
#define SIRKIT_WUF_SOURCE "../../../DataStructure/union_find_w.cpp"
#endif
#include SIRKIT_WUF_SOURCE
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
using I=std::int64_t;
static std::uint64_t checks=0;
static void require(bool x){++checks;if(!x)std::abort();}
static I add(I a,I b){constexpr I lo=std::numeric_limits<I>::min(),hi=std::numeric_limits<I>::max();require(b>=0?a<=hi-b:a>=lo-b);return a+b;}
static I sub(I a,I b){constexpr I lo=std::numeric_limits<I>::min(),hi=std::numeric_limits<I>::max();require(b>=0?a>=lo+b:a<=hi+b);return a-b;}
struct Event{I a,b;friend bool operator==(const Event&,const Event&)=default;};
static std::array<Event,131072> events;
static std::size_t event_count=0;
struct Value{
    I x=0;
    Value()=default;explicit Value(I a)noexcept:x(a){}
    Value& operator+=(const Value& b)noexcept{require(event_count<events.size());events[event_count++]={x,b.x};x=add(x,b.x);return *this;}
    friend Value operator+(const Value& a,const Value& b)noexcept{return Value(add(a.x,b.x));}
    friend Value operator-(const Value& a,const Value& b)noexcept{return Value(sub(a.x,b.x));}
    friend Value operator-(const Value& a)noexcept{return Value(sub(0,a.x));}
    friend bool operator==(const Value&,const Value&)=default;
};
struct Model{
    std::vector<int> parent;std::vector<I> delta;std::vector<Event> events;
    explicit Model(int n):parent(n,-1),delta(n){}
    int root(int a){const int p=parent[a];if(p<0)return a;if(parent[p]<0)return p;const int r=root(p);events.push_back({delta[a],delta[p]});delta[a]=add(delta[a],delta[p]);parent[a]=r;return r;}
    bool merge(int a,int b,I w){const int ra=root(a),rb=root(b);if(ra==rb)return false;w=sub(add(w,delta[a]),delta[b]);a=ra;b=rb;if(parent[a]>parent[b]){std::swap(a,b);w=sub(0,w);}parent[a]+=parent[b];parent[b]=a;delta[b]=w;return true;}
    std::vector<std::vector<int>> groups(){std::vector<int> leaders(parent.size());for(int i=0;i<int(parent.size());++i)leaders[i]=root(i);std::vector<std::vector<int>> out;for(int r=0;r<int(parent.size());++r)if(parent[r]<0){out.emplace_back();for(int v=0;v<int(parent.size());++v)if(leaders[v]==r)out.back().push_back(v);}return out;}
};
static void trace_equal(const Model& m){require(event_count==m.events.size());for(std::size_t i=0;i<event_count;++i)require(events[i]==m.events[i]);}
static std::vector<int> permutation(int n,int block,int order){
    std::vector<int> v(n);std::iota(v.begin(),v.end(),0);
    if(order==1)std::reverse(v.begin(),v.end());
    if(order==2){std::mt19937_64 gen(7139+n);std::shuffle(v.begin(),v.end(),gen);}
    if(order>=3){std::stable_sort(v.begin(),v.end(),[&](int a,int b){a%=block;b%=block;return order==3?(a%2>b%2):(std::popcount(unsigned(a))>std::popcount(unsigned(b)));});std::vector<int> inverse(n);for(int i=0;i<n;++i)inverse[v[i]]=i;v.swap(inverse);}return v;
}
static void cold_order(){
    for(int n:{1,8,17,255,256,257,1025,4097})for(int block:{8,128,2048,8192})for(int order=0;order<5;++order){
        UnionFindW<Value> uf(n);Model model(n);const auto ids=permutation(n,block,order);std::vector<I> potential(n);for(int i=0;i<n;++i)potential[i]=(I(i)*9239%997)-498;
        for(int s=0;s<n;s+=block)for(int step=1;step<block;step*=2)for(int i=s;i+step<std::min(s+block,n);i+=2*step){const int a=ids[i],b=ids[i+step];const I w=sub(potential[b],potential[a]);require(uf.merge(a,b,Value(w))==model.merge(a,b,w));}
        event_count=0;model.events.clear();
        const int leaf=ids[std::min(block,n)-1];require(uf.root(leaf)==model.root(leaf));trace_equal(model);
        require(uf.groups()==model.groups());trace_equal(model);
        for(int i=0;i<n;++i){const int r=model.root(i);require(uf.root(i)==r);require(uf.weight(i).x==model.delta[i]);require(uf.size(i)==-model.parent[r]);require(model.delta[i]==sub(potential[i],potential[r]));}
        trace_equal(model);
        if(n>1){const int a=ids[0],b=ids[n-1];const I w=sub(potential[b],potential[a]);require(uf.merge(a,b,Value(w))==model.merge(a,b,w));require(uf.groups()==model.groups());trace_equal(model);}
    }
}
template<class T> T make(I x){return T(x);}
template<class T> static void signed_order(bool lower,int order,int operation){
    constexpr I lo=std::numeric_limits<I>::min(),hi=std::numeric_limits<I>::max();
    const I a=lower?lo+5:hi-5,b=lower?-10:10,c=lower?10:-10;
    const auto ids=permutation(8,8,order);UnionFindW<T> uf(8);Model m(8);
    auto join=[&](int x,int y,I w){require(uf.merge(ids[x],ids[y],make<T>(w))==m.merge(ids[x],ids[y],w));};
    join(6,7,a);join(4,5,0);join(4,6,b);join(0,1,0);join(2,3,0);join(0,2,0);join(0,4,c);
    event_count=0;m.events.clear();
    if(operation==0){require(uf.root(ids[7])==m.root(ids[7]));}
    if(operation==1){require(uf.groups()==m.groups());}
    if(operation==2){auto got=uf.try_diff(ids[0],ids[7]);m.root(ids[0]);m.root(ids[7]);require(bool(got));if constexpr(std::is_same_v<T,Value>)require(got->x==a);else require(*got==a);}
    if constexpr(std::is_same_v<T,Value>)trace_equal(m);
    for(int v=0;v<8;++v){m.root(ids[v]);const auto got=uf.weight(ids[v]);if constexpr(std::is_same_v<T,Value>)require(got.x==m.delta[ids[v]]);else require(got==m.delta[ids[v]]);}
    if constexpr(std::is_same_v<T,Value>)trace_equal(m);
}
int main(){cold_order();for(bool lower:{false,true})for(int order=0;order<5;++order)for(int op=0;op<3;++op){signed_order<I>(lower,order,op);signed_order<Value>(lower,order,op);}std::printf("PASS iterative order checks=%llu\n",static_cast<unsigned long long>(checks));}
