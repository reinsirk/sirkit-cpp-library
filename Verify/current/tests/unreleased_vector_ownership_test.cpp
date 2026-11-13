// SPDX-License-Identifier: CC0-1.0
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <new>
struct AllocationHeader{void*base;std::size_t bytes;};
static volatile std::int64_t live_blocks=0,live_bytes=0;
[[gnu::noinline]] void*tracked_allocate(std::size_t bytes,std::size_t align){if(bytes>std::numeric_limits<std::size_t>::max()-sizeof(AllocationHeader)-align)throw std::bad_alloc();void*base=std::malloc(bytes+sizeof(AllocationHeader)+align);if(!base)throw std::bad_alloc();const auto raw=reinterpret_cast<std::uintptr_t>(base)+sizeof(AllocationHeader),address=(raw+align-1)&~static_cast<std::uintptr_t>(align-1);auto*header=reinterpret_cast<AllocationHeader*>(address-sizeof(AllocationHeader));std::construct_at(header,AllocationHeader{base,bytes});live_blocks=live_blocks+1;live_bytes=live_bytes+static_cast<std::int64_t>(bytes);return reinterpret_cast<void*>(address);}
[[gnu::noinline]] void tracked_delete(void*pointer)noexcept{if(!pointer)return;auto*header=reinterpret_cast<AllocationHeader*>(reinterpret_cast<std::uintptr_t>(pointer)-sizeof(AllocationHeader));live_blocks=live_blocks-1;live_bytes=live_bytes-static_cast<std::int64_t>(header->bytes);std::free(header->base);}
[[gnu::noinline]] void*operator new(std::size_t n){return tracked_allocate(n,alignof(std::max_align_t));}
[[gnu::noinline]] void*operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void*operator new(std::size_t n,std::align_val_t align){return tracked_allocate(n,static_cast<std::size_t>(align));}
[[gnu::noinline]] void*operator new[](std::size_t n,std::align_val_t align){return ::operator new(n,align);}
[[gnu::noinline]] void operator delete(void*p)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete[](void*p)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::align_val_t)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::align_val_t)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t,std::align_val_t)noexcept{tracked_delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{tracked_delete(p);}
#ifndef SIRKIT_UNRELEASED_VECTOR_SOURCE
#define SIRKIT_UNRELEASED_VECTOR_SOURCE "../../../Fast/Source/Internal/Unreleased_Vector.cpp"
#endif
#include SIRKIT_UNRELEASED_VECTOR_SOURCE
struct Tracked{static inline int live=0,destroyed=0;int value=0;explicit Tracked(int n=0):value(n){++live;}Tracked(const Tracked&x):value(x.value){++live;}Tracked(Tracked&&x)noexcept:value(std::exchange(x.value,-1)){++live;}Tracked&operator=(const Tracked&x){value=x.value;return *this;}Tracked&operator=(Tracked&&x)noexcept{value=std::exchange(x.value,-1);return *this;}~Tracked(){--live;++destroyed;}};
int main(){using V=reinsirk::detail::UnreleasedVector<Tracked>;assert(live_blocks==0);{std::vector<Tracked>normal;for(int i=0;i<1000;++i)normal.emplace_back(i);}assert(live_blocks==0&&live_bytes==0&&Tracked::live==0);std::printf("normal_live0=%" PRId64 " normal_bytes0=%" PRId64 "\n",live_blocks,live_bytes);
 std::size_t retained=0;int dtor_before=0;{V v;for(int i=0;i<10000;++i)v.emplace_back(i);assert(live_blocks==1&&live_bytes==static_cast<std::int64_t>(v.capacity()*sizeof(Tracked))&&Tracked::live==10000);v.erase(v.begin(),v.begin()+5000);assert(Tracked::live==5000);v.resize(6000);assert(Tracked::live==6000);v.clear();assert(Tracked::live==0&&live_blocks==1);for(int i=0;i<300;++i)v.emplace_back(i);V dest;dest.emplace_back(900);assert(Tracked::live==301);dest=std::move(v);assert(v.empty()&&dest.size()==300&&live_blocks==1&&Tracked::live==300);dest=std::move(dest);assert(dest.size()==300);retained=dest.capacity()*sizeof(Tracked);dtor_before=Tracked::destroyed;}assert(live_blocks==1&&live_bytes==static_cast<std::int64_t>(retained)&&Tracked::live==300&&Tracked::destroyed==dtor_before);std::printf("fast_retained_blocks=%" PRId64 " fast_retained_bytes=%" PRId64 " final_elements_not_destroyed=%d; old_growth_blocks_reclaimed\n",live_blocks,live_bytes,Tracked::live);
}
