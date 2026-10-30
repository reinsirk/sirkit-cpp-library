// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RWUF_SOURCE
#define SIRKIT_RWUF_SOURCE "../../../DataStructure/rollback-union-findw.cpp"
#endif
#include SIRKIT_RWUF_SOURCE
#include <algorithm>
#include <cstddef>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <cstdint>

static std::size_t checks = 0;
static void check(bool x) { ++checks; if (!x) std::abort(); }
using UF = RollbackWeightedUnionFind<std::int64_t>;
static std::int64_t potential(int v) { return static_cast<std::int64_t>(v) * 12345 - 6789; }
static void inspect(const UF& uf, const std::vector<int>& labels) {
    const int n = static_cast<int>(labels.size());
    std::vector<std::vector<int>> wanted(n);
    for (int v=0;v<n;++v) wanted[labels[v]].push_back(v);
    std::vector<std::vector<int>> groups;
    for (auto& g : wanted) if (!g.empty()) groups.push_back(std::move(g));
    check(uf.groups() == groups); check(uf.component_count() == static_cast<int>(groups.size()));
    for (int v=0;v<n;++v) {
        check(uf.find(v)==labels[v]);
        check(uf.get_weight(v)==potential(v)-potential(labels[v]));
        check(uf.diff(labels[v],v)==potential(v)-potential(labels[v]));
    }
}
static void groups_case(int n, int block, std::mt19937& rng) {
    UF uf(n); std::vector<int> order(n), labels(n);
    std::iota(order.begin(),order.end(),0); std::shuffle(order.begin(),order.end(),rng);
    std::iota(labels.begin(),labels.end(),0);
    for (int start=0;start<n;start+=block) {
        const int end=std::min(n,start+block);
        for (int step=1;step<end-start;step*=2)
            for (int j=start;j+step<end;j+=step*2)
                check(uf.unite(order[j],order[j+step],potential(order[j+step])-potential(order[j])));
        for (int j=start;j<end;++j) labels[order[j]]=order[start];
    }
    inspect(uf,labels); auto original=labels; uf.snapshot(); const auto original_state=uf.get_state();
    std::vector<std::vector<int>> versions;
    for (int start=block;start<n && versions.size()<17;start+=block) {
        versions.push_back(labels); const int x=order[0], y=order[start];
        check(uf.unite(x,y,potential(y)-potential(x)));
        const int old=labels[y]; for(auto& l:labels) if(l==old) l=labels[x];
        inspect(uf,labels);
        // 冗長・矛盾した制約も履歴を1件ずつ残す。
        check(uf.unite_checked(x,y,potential(y)-potential(x))==UF::MergeResult::AlreadyConsistent);
        check(uf.unite_checked(x,y,potential(y)-potential(x)+1)==UF::MergeResult::Contradiction);
        uf.undo(); uf.undo(); inspect(uf,labels);
    }
    if (!versions.empty()) { uf.undo(); labels=versions.back(); versions.pop_back(); inspect(uf,labels); }
    uf.rollback(original_state); labels=original; inspect(uf,labels);
    uf.rollback(); inspect(uf,labels);
    const UF copy=uf; uf.clear_history(); inspect(uf,labels); inspect(copy,labels);
}
struct ExplicitZero {
    std::int64_t value;
    ExplicitZero() noexcept : value(999) {}
    explicit ExplicitZero(int v) noexcept : value(v) {}
    ExplicitZero(const ExplicitZero&) noexcept = default;
    ExplicitZero(ExplicitZero&&) noexcept = default;
    ExplicitZero& operator=(const ExplicitZero&) noexcept = default;
    ExplicitZero& operator=(ExplicitZero&&) noexcept = default;
    friend ExplicitZero operator+(ExplicitZero a, ExplicitZero b) noexcept { a.value+=b.value;return a; }
    friend ExplicitZero operator-(ExplicitZero a, ExplicitZero b) noexcept { a.value-=b.value;return a; }
    friend bool operator==(ExplicitZero,ExplicitZero)=default;
};
static void reset_case() {
    RollbackWeightedUnionFind<ExplicitZero> uf(8193);
    for (int n : {4097,63,4097,0,8193,1,0,65,4097,8193}) {
        uf.reset(n); check(uf.size()==n && uf.get_state()==0 && uf.component_count()==n);
        check(uf.groups().size()==static_cast<std::size_t>(n));
        for(int v=0;v<n;++v) {check(uf.find(v)==v);check(uf.get_weight(v).value==0);check(uf.size(v)==1);}
        for(int v=1;v<n;++v) check(uf.unite(v-1,v,ExplicitZero(3)));
        if(n) { check(uf.diff(0,n-1).value==static_cast<std::int64_t>(n-1)*3);uf.snapshot(); }
    }
    uf.clear(); uf.reset(8193); check(uf.get_weight(8192).value==0);
    for(int v=0;v<8193;++v) check(uf.find(v)==v);
}
int main() {
    std::mt19937 rng(20261030);
    for (int n : {0,1,17,257,4097})
        for(int block : {2,4,16,128,8192}) groups_case(n,block,rng);
    reset_case(); std::printf("PASS groups/reset reference checks=%zu\n",checks);
}
