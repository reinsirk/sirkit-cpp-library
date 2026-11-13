// SPDX-License-Identifier: CC0-1.0
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

#ifndef SIRKIT_GENERAL_MATCHING_SOURCE
#define SIRKIT_GENERAL_MATCHING_SOURCE "../../../Graph/General_Matching.cpp"
#endif
#include SIRKIT_GENERAL_MATCHING_SOURCE
#include <iostream>
template<class T> std::vector<T> saved(std::span<const T> a) { return {a.begin(),a.end()}; }
GeneralMatching partial() {
    GeneralMatching graph(9); graph.add_edge(1,2);graph.add_edge(3,4);graph.add_edge(5,6);
    assert(graph.solve()==3);graph.add_edge(0,1);graph.add_edge(2,3);graph.add_edge(4,5);graph.add_edge(6,7);return graph;
}
void run_checks() {
    std::size_t failures=0;
    for(int operation=0;operation<7;++operation) {
        bool success=false;
        for(std::ptrdiff_t budget=0;budget<20;++budget) {
            auto target=operation==0?GeneralMatching(9):partial();GeneralMatching source(12);source.add_edge(0,1);source.add_edge(1,2);source.solve();
            const auto partners=saved(target.partners());const auto edges=saved(target.edges());const auto old_size=target.size(),old_count=target.matching_size();const bool old_maximum=target.is_maximum();
            const auto* pp=target.partners().data();const auto* ep=target.edges().data();const auto before=live_allocations.load();bool failed=false;allocation_countdown=budget;
            try {
                if(operation==0)target.add_edge(0,1);
                if(operation==1)target.reserve_edges(1000);
                if(operation==2){auto copy=target;assert(copy.matching_size()==old_count);}
                if(operation==3)target=source;
                if(operation==4)target.reset(11);
                if(operation==5)assert(target.solve()==4);
                if(operation==6){auto pairs=target.pairs();assert(pairs.size()==3);}
            }catch(const std::bad_alloc&){failed=true;++failures;}
            allocation_countdown=-1;
            if(failed) {
                assert(target.size()==old_size&&target.matching_size()==old_count&&target.is_maximum()==old_maximum);
                assert(saved(target.partners())==partners&&saved(target.edges())==edges&&target.partners().data()==pp&&target.edges().data()==ep);
                assert(live_allocations.load()==before);
            } else {success=true;break;}
        }
        assert(success);
    }
    {
        bool failed=false;const auto before=live_allocations.load();allocation_countdown=0;
        try{GeneralMatching graph(9);}catch(const std::bad_alloc&){failed=true;++failures;}
        allocation_countdown=-1;assert(failed&&live_allocations.load()==before);
    }
    {
        auto graph=partial();auto* self=&graph;GeneralMatching empty;graph.solve();
        allocation_countdown=0;
        graph=*self;graph=std::move(*self);assert(graph.solve()==4);
        GeneralMatching moved(std::move(graph));assert(graph.empty()&&graph.is_perfect());graph=std::move(moved);swap(graph,empty);empty.clear_edges();empty.clear();graph.clear();
        GeneralMatching perfect(0);assert(perfect.solve()==0);allocation_countdown=-1;
    }
    {
        GeneralMatching graph(4);graph.add_edge(0,1);graph.add_edge(2,3);assert(graph.solve()==2);graph.add_edge(0,2);assert(!graph.is_maximum());
        allocation_countdown=0;assert(graph.solve()==2&&graph.is_maximum());allocation_countdown=-1;
    }
    std::cout<<"GeneralMatching allocation failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta=0\n";}
