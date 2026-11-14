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

#ifndef SIRKIT_ODC_SOURCE
#define SIRKIT_ODC_SOURCE "../../../Graph/Offline_Dynamic_Connectivity.cpp"
#endif
#ifndef SIRKIT_FG_SOURCE
#define SIRKIT_FG_SOURCE "../../../Graph/Functional_Graph.cpp"
#endif
#include SIRKIT_ODC_SOURCE
#include SIRKIT_FG_SOURCE
#include <iostream>
#include <numeric>

void run_checks() {
    std::size_t failures=0;
    const std::vector<std::int64_t> weights{3,5,7,11,13,17,19,23};
    for (int operation=0;operation<11;++operation) {
        bool success=false;
        for(std::ptrdiff_t budget=0;budget<120;++budget){
            OfflineDynamicConnectivity target(weights),source(weights);
            for(int i=0;i<8;++i){target.add_edge(i,(i+1)%8);target.add_vertex(i,i-2);target.query_component_sum(i);}
            source.add_edge(0,7);source.query_same(0,7);
            const auto old=target.solve();const auto e=target.edge_count(),a=target.addition_count(),q=target.query_count();
            const auto before=live_allocations.load();allocation_countdown=budget;bool failed=false;
            try {
                if(operation==0)target.add_edge(0,7);
                if(operation==1)target.add_vertex(7,11);
                if(operation==2)target.query_component_sum(0);
                if(operation==3)target.solve();
                if(operation==4)target.reset(20);
                if(operation==5)target.reset(weights);
                if(operation==6){auto copy=target;assert(copy.size()==8);}
                if(operation==7)target=source;
                if(operation==8)target.reserve_edges(200);
                if(operation==9)target.reserve_queries(200);
                if(operation==10)target.reserve_additions(200);
            }catch(const std::bad_alloc&){failed=true;++failures;}
            allocation_countdown=-1;
            if(failed){assert(target.size()==8&&target.edge_count()==e&&target.addition_count()==a&&target.query_count()==q&&target.solve()==old);assert(live_allocations.load()==before);}
            else{success=true;break;}
        }assert(success);
    }
    const std::vector<int> next{4,2,1,6,3,7,5,6,9,8,10};
    for(int operation=0;operation<4;++operation){bool success=false;
        for(std::ptrdiff_t budget=0;budget<120;++budget){
            FunctionalGraph target(next), source(std::vector<int>{1,2,3,4,0});const auto* ptr=target.successors().data();const auto d=target.distance(0,7);const auto k=target.kth_successor(0,UINT64_MAX);
            const auto before=live_allocations.load();allocation_countdown=budget;bool failed=false;
            try{
                if(operation==0)target.build(source.successors());
                if(operation==1)target.build(target.successors());
                if(operation==2)target=source;
                if(operation==3){auto copy=target;assert(copy.size()==11);}
            }catch(const std::bad_alloc&){failed=true;++failures;}
            allocation_countdown=-1;
            if(failed){assert(target.successors().data()==ptr&&target.distance(0,7)==d&&target.kth_successor(0,UINT64_MAX)==k);assert(std::equal(next.begin(),next.end(),target.successors().begin()));assert(live_allocations.load()==before);}
            else{success=true;break;}
        }assert(success);
    }
    {OfflineDynamicConnectivity a(weights);a.add_edge(0,1);a.query_same(0,1);FunctionalGraph f(next);allocation_countdown=0;auto b=std::move(a);a=std::move(b);a.clear_history();a.clear();auto g=std::move(f);f=std::move(g);f.clear();allocation_countdown=-1;assert(a.empty()&&b.empty()&&f.empty()&&g.empty());}
    assert(failures>0);std::cout<<"connectivity/functional injected failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta="<<live_allocations.load()-before<<'\n';}
