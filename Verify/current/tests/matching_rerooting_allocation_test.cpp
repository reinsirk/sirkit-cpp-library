// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <stdexcept>
static std::ptrdiff_t allocation_countdown = -1;
static std::atomic<std::size_t> live_allocations{0};
static void allocation_check() {
    if (allocation_countdown == 0) throw std::bad_alloc();
    if (allocation_countdown > 0) --allocation_countdown;
}
void* operator new(std::size_t size) {
    allocation_check();
    if (void* p = std::malloc(size == 0 ? 1 : size)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { if (p) { --live_allocations; std::free(p); } }
void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }
void* operator new(std::size_t size, std::align_val_t alignment) {
    allocation_check();
    const auto align = static_cast<std::size_t>(alignment);
    const auto bytes = (size / align + 1) * align;
    if (void* p = std::aligned_alloc(align, bytes)) { ++live_allocations; return p; }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size, std::align_val_t alignment) { return ::operator new(size, alignment); }
void operator delete(void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { ::operator delete(p); }

#ifndef MATCHING_SOURCE
#define MATCHING_SOURCE "../../../Graph/Bipartite_Matching.cpp"
#endif
#ifndef REROOTING_SOURCE
#define REROOTING_SOURCE "../../../Graph/Rerooting_DP.cpp"
#endif
#include MATCHING_SOURCE
#include REROOTING_SOURCE
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using State=std::pair<std::int64_t,std::int64_t>;
State combine(const State&a,const State&b){return{a.first+b.first,a.second+b.second};}
State vertex(const State&a,int){return{a.first,a.second+1};}
State edge(const State&a,int,int,std::size_t){return{a.first+a.second,a.second};}

template<class T>std::vector<T> saved(std::span<const T> value){return {value.begin(),value.end()};}
struct Counted {
    static inline int budget=-1;
    int value;
    Counted()=delete;
    explicit Counted(int v):value(v){}
    static void tick(){if(budget==0)throw std::runtime_error("copy injection");if(budget>0)--budget;}
    Counted(const Counted& v):value(v.value){tick();}
    Counted(Counted&& v)noexcept(false):value(v.value){tick();}
    Counted& operator=(const Counted& v){tick();value=v.value;return *this;}
    Counted& operator=(Counted&& v)noexcept(false){tick();value=v.value;return *this;}
};
void run_checks(){
    std::size_t failures=0;
    // 縮約先の構築・solveで確保が失敗しても、元グラフと対応はそのまま。
    {
        bool success = false;
        for (std::ptrdiff_t limit = 0; limit < 100; ++limit) {
            BipartiteMatching target(6, 6);
            for (int u = 0; u < 6; ++u) {
                target.add_edge(u, u);
                target.add_edge(u, (u + 1) % 6);
            }
            target.add_edge(0, 3);
            const auto view = target.left_matches();
            const auto neighbors = target.neighbors(0);
            const auto before = live_allocations.load();
            allocation_countdown = limit;
            bool failed = false;
            try { assert(target.solve() == 6); }
            catch (const std::bad_alloc&) { failed = true; ++failures; }
            allocation_countdown = -1;
            assert(view.data() == target.left_matches().data() && neighbors.data() == target.neighbors(0).data());
            assert(target.edge_count() == 13 && neighbors.size() == 3);
            if (!failed) { assert(target.is_maximum()); success = true; break; }
            assert(target.matching_size() == 0 && !target.is_maximum());
            assert(live_allocations.load() == before);
            for (int v : view) assert(v == -1);
            for (int u : target.right_matches()) assert(u == -1);
        }
        assert(success);
    }
    // 空の対応からの次数削減用確保を含め、更新より先にすべての確保が終わることを調べる。
    for(int kind=0;kind<4;++kind){bool success=false;for(std::ptrdiff_t limit=0;limit<100;++limit){
        const bool transpose=kind>=2;BipartiteMatching target(transpose?11:7,transpose?7:11);
        auto add=[&](int u,int v){if(transpose)std::swap(u,v);target.add_edge(u,v);};
        for(int u=0;u<4;++u){add(u,u);add(u,(u+1)%4);}
        if(kind%2==0){add(4,4);add(4,4);add(5,5);add(6,5);}
        else{for(int u=4;u<7;++u)for(int v=4;v<7;++v)add(u,v);add(6,7);}
        const auto left_view=target.left_matches(),right_view=target.right_matches();
        const auto old_left=saved(left_view),old_right=saved(right_view);const auto old_edges=target.edge_count();
        const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;
        try{assert(target.solve()==(kind%2==0?6:7));}catch(const std::bad_alloc&){failed=true;++failures;}
        allocation_countdown=-1;
        assert(left_view.data()==target.left_matches().data()&&right_view.data()==target.right_matches().data());
        assert(target.edge_count()==old_edges);
        if(failed){assert(saved(left_view)==old_left&&saved(right_view)==old_right);assert(target.matching_size()==0&&!target.is_maximum());assert(live_allocations.load()==before);}
        else{assert(target.is_maximum());success=true;break;}
    }assert(success);}
    for(int kind=0;kind<7;++kind){bool success=false;for(std::ptrdiff_t limit=0;limit<100;++limit){
        BipartiteMatching target(4,4);for(int u=0;u<3;++u)target.add_edge(u,u+1);assert(target.solve()==3);
        for(int u=0;u<4;++u)target.add_edge(u,u);
        if(kind>=2&&kind<=4)assert(target.solve()==4);
        const auto old_left=saved(target.left_matches()),old_right=saved(target.right_matches());
        const auto old_edges=target.edge_count();const auto old_card=target.matching_size();const auto old_max=target.is_maximum();
        const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;
        try{
            if(kind==0)target.add_edge(0,3);
            if(kind==1)target.solve();
            if(kind==2)target.min_vertex_cover();
            if(kind==3)target.max_independent_set();
            if(kind==4)target.pairs();
            if(kind==5)target.reset(8,9);
            if(kind==6){auto copy=target;assert(copy.matching_size()==old_card);}
        }catch(const std::bad_alloc&){failed=true;++failures;}
        allocation_countdown=-1;
        if(failed){assert(saved(target.left_matches())==old_left&&saved(target.right_matches())==old_right);assert(target.edge_count()==old_edges&&target.matching_size()==old_card&&target.is_maximum()==old_max);assert(live_allocations.load()==before);}
        else{success=true;break;}
    }assert(success);}
    {
        // 非対称な次数で反転経路を選び、部分的な対応がある状態の確保失敗を調べる。
        bool success=false;std::size_t orientation_failures=0;
        for(std::ptrdiff_t limit=0;limit<100;++limit){
            BipartiteMatching target(7,11);
            for(int u=0;u<3;++u){target.add_edge(u,0);target.add_edge(u,u+1);}
            assert(target.solve()==3);
            for(int u=3;u<6;++u){target.add_edge(u,0);target.add_edge(u,u+1);}
            target.add_edge(6,0);
            const auto left_view=target.left_matches(),right_view=target.right_matches();
            const auto old_left=saved(left_view),old_right=saved(right_view);
            std::vector<std::span<const int>> views;std::vector<std::vector<int>> neighbors;
            for(int u=0;u<7;++u){views.push_back(target.neighbors(u));neighbors.push_back(saved(views.back()));}
            const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;
            try{assert(target.solve()==7);}catch(const std::bad_alloc&){failed=true;++failures;++orientation_failures;}
            allocation_countdown=-1;
            assert(target.left_size()==7&&target.right_size()==11&&target.edge_count()==13);
            assert(left_view.data()==target.left_matches().data()&&right_view.data()==target.right_matches().data());
            for(int u=0;u<7;++u){const auto now=target.neighbors(u);assert(now.data()==views[static_cast<std::size_t>(u)].data());assert(saved(now)==neighbors[static_cast<std::size_t>(u)]);}
            if(failed){assert(saved(left_view)==old_left&&saved(right_view)==old_right);assert(target.matching_size()==3&&!target.is_maximum());assert(live_allocations.load()==before);}
            else{for(int u=0;u<7;++u){const int v=u==6?0:u+1;assert(target.match_left(u)==v&&target.match_right(v)==u);}assert(target.is_maximum()&&target.min_vertex_cover().size()==7);success=true;break;}
        }
        assert(success&&orientation_failures>0);
    }
    {
        BipartiteMatching source(4,4),target(2,2);source.add_edge(1,2);source.solve();target.add_edge(0,0);target.solve();auto old=saved(target.left_matches());bool success=false;
        for(std::ptrdiff_t limit=0;limit<80;++limit){const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;try{target=source;}catch(const std::bad_alloc&){failed=true;++failures;}allocation_countdown=-1;if(failed){assert(saved(target.left_matches())==old&&target.left_size()==2);assert(live_allocations.load()==before);}else{success=true;break;}}
        assert(success&&source.match_left(1)==2&&target.match_left(1)==2);
        allocation_countdown=0;auto moved=std::move(target);target=std::move(moved);target.clear_matching();target.clear_edges();target.clear();allocation_countdown=-1;assert(target.empty());
    }
    for(int kind=0;kind<4;++kind){bool success=false;for(std::ptrdiff_t limit=0;limit<100;++limit){
        RerootingDP<State> target(6);for(int v=1;v<5;++v)target.add_edge(v-1,v);target.build(State{},combine,vertex,edge);
        auto old_values=saved(target.values()),old_messages=saved(target.messages());auto old_edges=saved(target.edges());
        const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;
        try{
            if(kind==0)target.build(State{},combine,vertex,[](const State&a,int,int,std::size_t){return State{a.first+2*a.second,a.second};});
            if(kind==1)target.add_edge(4,5);
            if(kind==2)target.reset(11);
            if(kind==3){auto copy=target;assert(copy.size()==6&&copy[0]==target[0]);}
        }catch(const std::bad_alloc&){failed=true;++failures;}
        allocation_countdown=-1;
        if(failed){assert(saved(target.values())==old_values&&saved(target.messages())==old_messages&&saved(target.edges())==old_edges);assert(target.has_results());assert(live_allocations.load()==before);}
        else{success=true;break;}
    }assert(success);}
    {
        RerootingDP<State> source(8),target(2);for(int v=1;v<8;++v)source.add_edge(0,v);source.build(State{},combine,vertex,edge);target.add_edge(0,1);target.build(State{},combine,vertex,edge);auto old_values=saved(target.values()),old_messages=saved(target.messages());bool success=false;
        for(std::ptrdiff_t limit=0;limit<100;++limit){const auto before=live_allocations.load();allocation_countdown=limit;bool failed=false;try{target=source;}catch(const std::bad_alloc&){failed=true;++failures;}allocation_countdown=-1;if(failed){assert(target.size()==2&&saved(target.values())==old_values&&saved(target.messages())==old_messages);assert(live_allocations.load()==before);}else{success=true;break;}}
        assert(success&&source[0]==target[0]);allocation_countdown=0;auto moved=std::move(target);target=std::move(moved);target.clear();allocation_countdown=-1;assert(target.empty());
    }
    // 値のcopy/move/代入と各callbackの失敗位置を全てずらして確認。
    for(int at=0;at<180;++at){Counted::budget=-1;RerootingDP<Counted> dp(5);for(int v=1;v<5;++v)dp.add_edge(0,v);auto merge=[](const Counted&a,const Counted&b){return Counted(a.value+b.value);};auto tv=[](const Counted&a,int){return Counted(a.value+1);};auto te=[](const Counted&a,int,int,std::size_t){return Counted(a.value);};dp.build(Counted(0),merge,tv,te);std::vector<int>values,messages;for(const auto&a:dp.values())values.push_back(a.value);for(const auto&a:dp.messages())messages.push_back(a.value);Counted::budget=at;bool failed=false;try{dp.build(Counted(0),merge,tv,te);}catch(const std::runtime_error&){failed=true;++failures;}Counted::budget=-1;for(std::size_t i=0;i<values.size();++i)assert(dp.values()[i].value==values[i]);for(std::size_t i=0;i<messages.size();++i)assert(dp.messages()[i].value==messages[i]);if(!failed)break;}
    for(int kind=0;kind<3;++kind)for(int at=0;at<30;++at){RerootingDP<State> dp(5);for(int v=1;v<5;++v)dp.add_edge(v-1,v);dp.build(State{},combine,vertex,edge);auto values=saved(dp.values()),messages=saved(dp.messages());int calls=0;auto merge=[&](const State&a,const State&b){if(kind==0&&calls++==at)throw std::runtime_error("merge");return combine(a,b);};auto tv=[&](const State&a,int v){if(kind==1&&calls++==at)throw std::runtime_error("vertex");return vertex(a,v);};auto te=[&](const State&a,int from,int to,std::size_t id){if(kind==2&&calls++==at)throw std::runtime_error("edge");return edge(a,from,to,id);};try{dp.build(State{},merge,tv,te);}catch(const std::runtime_error&){++failures;assert(saved(dp.values())==values&&saved(dp.messages())==messages);} }
    assert(failures>0);std::cout<<"matching/rerooting injected failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta="<<live_allocations.load()-before<<'\n';}
