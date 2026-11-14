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

#ifndef WAVELET_SOURCE
#define WAVELET_SOURCE "../../../DataStructure/Wavelet_Matrix.cpp"
#endif
#include WAVELET_SOURCE
#include <cstdint>
#include <iostream>
#include <vector>

using I = std::int64_t;
void check_values(const WaveletMatrix<I>& matrix,const std::vector<I>& values) {
    assert(matrix.size()==values.size());
    for(std::size_t i=0;i<values.size();++i)assert(matrix.access(i)==values[i]);
    for(I probe:{std::numeric_limits<I>::lowest(),I{0},I{123},std::numeric_limits<I>::max()}) {
        std::size_t count=0;for(I value:values)count+=value==probe;
        assert(matrix.count(0,values.size(),probe)==count);
    }
}
void run_checks() {
    std::vector<I> old_values(67),new_values(257);
    for(std::size_t i=0;i<old_values.size();++i)old_values[i]=static_cast<I>(123+i%2);
    for(std::size_t i=0;i<new_values.size();++i)new_values[i]=std::bit_cast<I>(std::uint64_t{0x719ae1bd0173}*(i+1));
    new_values[0]=std::numeric_limits<I>::lowest();new_values[1]=std::numeric_limits<I>::max();
    WaveletMatrix<I> original(new_values);
    std::size_t failures=0;
    for(int operation=0;operation<3;++operation) {
        bool success=false;
        for(std::ptrdiff_t at=0;at<500;++at) {
            WaveletMatrix<I> target(old_values);
            const auto before=live_allocations.load();
            allocation_countdown=at;bool failed=false;
            try {
                if(operation==0)target.build(new_values);
                if(operation==1)target=original;
                if(operation==2){auto copy=original;assert(copy.size()==new_values.size());}
            }catch(const std::bad_alloc&){failed=true;++failures;}
            allocation_countdown=-1;
            check_values(original,new_values);
            if(failed){check_values(target,old_values);assert(live_allocations.load()==before);}
            else {check_values(target,operation==2?old_values:new_values);success=true;break;}
        }
        assert(success);
    }
    auto copy=original;copy.clear();assert(copy.empty());check_values(original,new_values);
    copy.build(old_values);check_values(copy,old_values);
    std::cout<<"wavelet allocation failures="<<failures<<'\n';
}
int main(){const auto before=live_allocations.load();run_checks();assert(live_allocations.load()==before);std::cout<<"wavelet live delta="<<live_allocations.load()-before<<'\n';}
