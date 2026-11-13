// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
struct State { std::int64_t sum{},count{}; };
int main() {
    constexpr std::int64_t mod=998244353;
    int n{}; verify_in >> n;
    std::vector<std::int64_t> values(static_cast<std::size_t>(n));
    for (auto& value : values) verify_in >> value;
    std::vector<std::int64_t> slope(static_cast<std::size_t>(n-1)),constant(static_cast<std::size_t>(n-1));
    RerootingDP<State> tree(n);
    for (int i=0;i<n-1;++i) {
        int u{},v{}; verify_in >> u >> v >> slope[static_cast<std::size_t>(i)] >> constant[static_cast<std::size_t>(i)];
        tree.add_edge(u,v);
    }
    tree.build(State{},
        [](const State& a,const State& b){ return State{(a.sum+b.sum)%mod,a.count+b.count}; },
        [&](const State& a,int v){ return State{(a.sum+values[static_cast<std::size_t>(v)])%mod,a.count+1}; },
        [&](const State& a,int,int,std::size_t id){ return State{(a.sum*slope[id]+a.count*constant[id])%mod,a.count}; });
    for (int v=0;v<n;++v) { if (v!=0) verify_out << ' '; verify_out << tree[v].sum; }
    verify_out << '\n';
}
