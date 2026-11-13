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

#ifndef SIRKIT_MO_SOURCE
#define SIRKIT_MO_SOURCE "../../../Algorithm/Mo.cpp"
#endif
#include SIRKIT_MO_SOURCE
#include <iostream>
template<class T>std::vector<T>saved(std::span<const T>a){return{a.begin(),a.end()};}
struct CallbackFailure {};
void run_checks(){
 std::size_t failures=0,callback_failures=0;
 for(int operation=0;operation<6;++operation){bool success=false;for(std::ptrdiff_t budget=0;budget<20;++budget){
  Mo target(6),source(10);target.reserve_queries(8);for(int i=0;i<8;++i)target.add_query(i%3,i%3+2);source.add_query(2,8);const auto old=saved(target.queries());const auto* ptr=target.queries().data();const auto before=live_allocations.load();int callbacks=0;allocation_countdown=budget;bool failed=false;
  try{
   if(operation==0)target.add_query(0,6);
   if(operation==1)target.reserve_queries(1000);
   if(operation==2){auto copy=target;assert(copy.query_count()==8);}
   if(operation==3)target=source;
   if(operation==4)target.order();
   if(operation==5)target.run([&](std::int32_t){++callbacks;},[&](std::int32_t){++callbacks;},[&](std::size_t){++callbacks;});
  }catch(const std::bad_alloc&){failed=true;++failures;}
  allocation_countdown=-1;if(failed){assert(target.length()==6&&saved(target.queries())==old&&target.queries().data()==ptr);assert(callbacks==0);assert(live_allocations.load()==before);}else{success=true;break;}
 }assert(success);}
 for(int operation=0;operation<8;++operation){bool success=false;for(std::ptrdiff_t budget=0;budget<20;++budget){
  MoWithUpdates target(6),source(10);target.reserve_queries(8);target.reserve_updates(8);for(int i=0;i<8;++i){target.add_update(i%6);target.add_query(i%3,i%3+2);}source.add_update(9);source.add_query(2,8);const auto old_q=saved(target.queries());const auto old_u=saved(target.updates());const auto* qp=target.queries().data();const auto* up=target.updates().data();const auto before=live_allocations.load();int callbacks=0;allocation_countdown=budget;bool failed=false;
  try{
   if(operation==0)target.add_update(5);
   if(operation==1)target.add_query(0,6);
   if(operation==2)target.reserve_updates(1000);
   if(operation==3)target.reserve_queries(1000);
   if(operation==4){auto copy=target;assert(copy.query_count()==8&&copy.update_count()==8);}
   if(operation==5)target=source;
   if(operation==6)target.order();
   if(operation==7)target.run([&](std::int32_t){++callbacks;},[&](std::int32_t){++callbacks;},[&](std::size_t,std::int32_t,std::int32_t){++callbacks;},[&](std::size_t,std::int32_t,std::int32_t){++callbacks;},[&](std::size_t){++callbacks;});
  }catch(const std::bad_alloc&){failed=true;++failures;}
  allocation_countdown=-1;if(failed){assert(target.length()==6&&saved(target.queries())==old_q&&saved(target.updates())==old_u&&target.queries().data()==qp&&target.updates().data()==up);assert(callbacks==0);assert(live_allocations.load()==before);}else{success=true;break;}
 }assert(success);}
 {Mo m(6);m.add_query(0,6);m.add_query(4,5);m.add_query(1,4);auto old=saved(m.queries());const auto* ptr=m.queries().data();for(int failure=0;failure<200;++failure){int calls=0;const auto before=live_allocations.load();auto tick=[&]{if(calls++==failure)throw CallbackFailure{};};bool threw=false;try{m.run([&](std::int32_t){tick();},[&](std::int32_t){tick();},[&](std::size_t){tick();});}catch(const CallbackFailure&){threw=true;++callback_failures;}assert(saved(m.queries())==old&&m.queries().data()==ptr);assert(live_allocations.load()==before);if(!threw)break;}}
 {MoWithUpdates m(6);m.add_query(0,6);m.add_update(4);m.add_query(1,5);m.add_update(2);m.add_query(4,6);m.add_query(1,3,0);auto old=saved(m.queries());auto updates=saved(m.updates());const auto* ptr=m.queries().data();for(int failure=0;failure<300;++failure){int calls=0;const auto before=live_allocations.load();auto tick=[&]{if(calls++==failure)throw CallbackFailure{};};bool threw=false;try{m.run([&](std::int32_t){tick();},[&](std::int32_t){tick();},[&](std::size_t,std::int32_t,std::int32_t){tick();},[&](std::size_t,std::int32_t,std::int32_t){tick();},[&](std::size_t){tick();});}catch(const CallbackFailure&){threw=true;++callback_failures;}assert(saved(m.queries())==old&&saved(m.updates())==updates&&m.queries().data()==ptr);assert(live_allocations.load()==before);if(!threw)break;}}
 {Mo a(3);a.add_query(0,3);MoWithUpdates b(3);b.add_update(1);b.add_query(0,3);allocation_countdown=0;auto ma=std::move(a);a=std::move(ma);a.clear_requests();a.clear();auto mb=std::move(b);b=std::move(mb);b.clear_requests();b.clear();allocation_countdown=-1;assert(a.length()==0&&a.empty()&&b.length()==0&&b.empty()&&b.update_count()==0);}
 assert(failures>0&&callback_failures>0);std::cout<<"Mo allocation failures="<<failures<<" callback failures="<<callback_failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"live allocation delta="<<live_allocations.load()-before<<'\n';}
