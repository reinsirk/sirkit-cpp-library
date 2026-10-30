// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_WUF_SOURCE
#define SIRKIT_WUF_SOURCE "../../../DataStructure/union_find_w.cpp"
#endif
#include SIRKIT_WUF_SOURCE
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <type_traits>

inline std::uint64_t checks=0;
inline void require_impl(bool b,const char* text,int line) { ++checks; if(!b) {std::fprintf(stderr,"line %d: %s\n",line,text);std::abort();} }
#define require(b) require_impl(bool(b),#b,__LINE__)
struct Ring {
    static inline std::uint64_t additions=0;
    static constexpr std::int64_t mod=1000000007;
    std::int64_t x=0;
    Ring()=default;
    explicit Ring(std::int64_t y) noexcept :x((y%mod+mod)%mod){}
    Ring& operator+=(const Ring& b) noexcept { ++additions; x=(x+b.x)%mod; return *this; }
    friend Ring operator+(Ring a,const Ring& b) noexcept { return a+=b; }
    friend Ring operator-(const Ring& a,const Ring& b) noexcept { return Ring(a.x-b.x); }
    friend Ring operator-(const Ring& a) noexcept { return Ring(-a.x); }
    friend bool operator==(const Ring&,const Ring&)=default;
};
template<class T> T value(std::int64_t a) { return T(a); }
template<class T> std::int64_t raw(T a) {
    if constexpr(std::is_same_v<T,Ring>)return a.x;
    else return a;
}
struct Model {
    std::vector<int> permutation,labels,sizes;
    std::vector<std::int64_t> potentials;
    int n,block,components;
    Model(int count,int block_size,int order) : permutation(count),labels(count),sizes(count,1),potentials(count),n(count),block(block_size),components(count) {
        std::iota(permutation.begin(),permutation.end(),0);
        if(order==1)std::reverse(permutation.begin(),permutation.end());
        if(order==2){std::mt19937_64 gen(510721+count);std::shuffle(permutation.begin(),permutation.end(),gen);}
        if(order>=3){
            std::stable_sort(permutation.begin(),permutation.end(),[&](int a,int b){
                const int x=a%block,y=b%block;
                if(order==3)return (x%2)!=(y%2) ? x%2>y%2 : false;
                return std::popcount(unsigned(x))>std::popcount(unsigned(y));
            });
            std::vector<int> inverse(count);
            for(int i=0;i<count;++i)inverse[permutation[i]]=i;
            permutation.swap(inverse);
        }
        std::iota(labels.begin(),labels.end(),0);
        for(int i=0;i<count;++i)potentials[i]=(std::int64_t(i)*7919%701)-350;
    }
    template<class T> void build(UnionFindW<T>& uf) {
        for(int start=0;start<n;start+=block){
            const int end=std::min(start+block,n);
            for(int step=1;step<block;step*=2)for(int i=start;i+step<end;i+=2*step){
                const int a=permutation[i],b=permutation[i+step];
                require(uf.merge(a,b,value<T>(potentials[b]-potentials[a])));
                join(a,b);
            }
        }
    }
    void join(int a,int b) {
        int ra=labels[a],rb=labels[b];if(ra==rb)return;
        if(sizes[ra]<sizes[rb])std::swap(ra,rb);
        for(int i=0;i<n;++i)if(labels[i]==rb)labels[i]=ra;
        sizes[ra]+=sizes[rb];sizes[rb]=0;--components;
    }
    std::vector<std::vector<int>> groups() const {
        std::vector<std::vector<int>> result;
        for(int r=0;r<n;++r)if(sizes[r]){result.emplace_back();for(int v=0;v<n;++v)if(labels[v]==r)result.back().push_back(v);}
        return result;
    }
    template<class T> void observe(UnionFindW<T>& uf) const {
        require(uf.size()==n&&uf.component_count()==components);
        for(int v=0;v<n;++v){
            require(uf.root(v)==labels[v]);require(uf.size(v)==sizes[labels[v]]);
            require(raw(uf.weight(v))==raw(value<T>(potentials[v]-potentials[labels[v]])));
            const int w=permutation[n-1-v];const auto d=uf.try_diff(v,w);
            require(bool(d)==(labels[v]==labels[w]));
            if(d)require(raw(*d)==raw(value<T>(potentials[w]-potentials[v])));
        }
    }
};
