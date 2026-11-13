// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RUF_SOURCE
#define SIRKIT_RUF_SOURCE "../../../DataStructure/rollback-union-find.cpp"
#endif
#include SIRKIT_RUF_SOURCE
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>
#include <algorithm>

static std::size_t checks = 0;
static void require(bool x) { ++checks; if (!x) std::abort(); }
// 各頂点に代表IDを直接記録する独立モデル。UFの親配列や履歴形式を使わない。
struct Reference {
    std::vector<int> representative;
    std::vector<std::vector<int>> versions;
    int snapshot = 0;
    explicit Reference(int n): representative(static_cast<std::size_t>(n)) { std::iota(representative.begin(), representative.end(), 0); }
    bool merge(int a, int b) {
        versions.push_back(representative);
        int x = representative[a], y = representative[b];
        if (x == y) return false;
        const auto sx = std::count(representative.begin(), representative.end(), x);
        const auto sy = std::count(representative.begin(), representative.end(), y);
        if (sx < sy) std::swap(x,y);
        for (auto& r : representative) if (r == y) r = x;
        return true;
    }
    void undo() {
        representative = std::move(versions.back()); versions.pop_back();
        snapshot = std::min(snapshot, static_cast<int>(versions.size()));
    }
    void rollback(int target = -1) {
        if (target == -1) target = snapshot;
        while (static_cast<int>(versions.size()) > target) undo();
        snapshot = std::min(snapshot,target);
    }
};
static void inspect(const RollbackUnionFind& uf, const Reference& m) {
    const int n = static_cast<int>(m.representative.size());
    require(uf.size() == n && uf.get_state() == static_cast<int>(m.versions.size()));
    std::vector<int> sizes(n);
    for (int r : m.representative) ++sizes[r];
    require(uf.component_count() == static_cast<int>(std::count_if(sizes.begin(),sizes.end(),[](int x){return x!=0;})));
    std::vector<std::vector<int>> expected;
    for (int r=0;r<n;++r) if (sizes[r]) {
        std::vector<int> g;
        for (int i=0;i<n;++i) if (m.representative[i]==r) g.push_back(i);
        expected.push_back(std::move(g));
    }
    require(uf.groups() == expected);
    for (int v=0;v<n;++v) {
        const int root=m.representative[v];
        require(uf.find(v)==root && uf.root(v)==root && uf.leader(v)==root && uf.size(v)==sizes[root]);
        const int w=(v*43+17)%n;
        require(uf.same(v,w)==(root==m.representative[w]));
    }
}
static void merge(RollbackUnionFind& uf, Reference& m, int a, int b, bool alias = false) {
    const bool want=m.merge(a,b);
    require((alias?uf.merge(a,b):uf.unite(a,b))==want);
}
static void exercise(int n, std::mt19937_64& rng) {
    RollbackUnionFind uf(n); Reference m(n); inspect(uf,m);
    std::vector<int> permutation(n); std::iota(permutation.begin(),permutation.end(),0);
    std::shuffle(permutation.begin(),permutation.end(),rng);
    // 引数の大小が無関係な平衡木。途中に同じ成分/同じ引数の失敗結合を混ぜる。
    for (int stride=1;stride<n;stride*=2) {
        for (int first=0;first+stride<n;first+=stride*2) {
            int a=permutation[first+stride-1], b=permutation[std::min(n-1,first+2*stride-1)];
            if (rng()&1) std::swap(a,b);
            merge(uf,m,a,b);
            merge(uf,m,a,a);
            merge(uf,m,b,a,true);
            if ((first/stride)%31==0) inspect(uf,m);
        }
        uf.snapshot(); m.snapshot=static_cast<int>(m.versions.size()); inspect(uf,m);
    }
    if (uf.get_state()) {
        uf.undo();m.undo();inspect(uf,m);
        int target=uf.get_state()/3;uf.rollback(target);m.rollback(target);inspect(uf,m);
        uf.rollback();m.rollback();inspect(uf,m);
        uf.rollback(0);m.rollback(0);inspect(uf,m);
    }
    for(int iter=0;iter<2000;++iter) {
        const int operation=static_cast<int>(rng()%10);
        if(n && operation<5) merge(uf,m,static_cast<int>(rng()%n),static_cast<int>(rng()%n),operation==4);
        else if(operation==5 && uf.get_state()) {uf.undo();m.undo();}
        else if(operation==6) {uf.snapshot();m.snapshot=uf.get_state();}
        else if(operation==7) {int target=static_cast<int>(rng()%(uf.get_state()+1));uf.rollback(target);m.rollback(target);}
        else if(operation==8) {uf.rollback();m.rollback();}
        else {uf.clear_history();m.versions.clear();m.snapshot=0;}
        if(iter%31==0) inspect(uf,m);
    }
    inspect(uf,m);uf.rollback(0);m.rollback(0);inspect(uf,m);
}
int main() {
    std::mt19937_64 rng(0x6fa320261030ULL);
    for(int n:{0,1,2,3,17,63,65,257,4097}) exercise(n,rng);
    std::printf("PASS exact representative/history/groups checks=%zu\n",checks);
}
