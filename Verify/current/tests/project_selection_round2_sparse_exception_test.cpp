// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_PROJECT_TEST_HEADER
#define SIRKIT_PROJECT_TEST_HEADER "../../../Graph/Project_Selection.cpp"
#endif
#ifndef SIRKIT_PROJECT_REQUIRE_EXTRA
#define SIRKIT_PROJECT_REQUIRE_EXTRA 1
#endif
#ifndef SIRKIT_PROJECT_REQUIRE_STRONG
#define SIRKIT_PROJECT_REQUIRE_STRONG 1
#endif
#include SIRKIT_PROJECT_TEST_HEADER
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <optional>
#include <utility>
std::int64_t allocation_budget=-1;
std::size_t live_allocations=0,live_bytes=0,peak_bytes=0,allocation_calls=0;
struct AllocationHeader{void* raw;std::size_t bytes;};
void* allocate_test(std::size_t bytes,std::size_t alignment){if(allocation_budget==0)throw std::bad_alloc();if(allocation_budget>0)--allocation_budget;alignment=std::max(alignment,alignof(std::max_align_t));const std::size_t extra=sizeof(AllocationHeader)+alignment-1;if(bytes>std::numeric_limits<std::size_t>::max()-extra)throw std::bad_alloc();void*raw=std::malloc((bytes==0?1:bytes)+extra);if(!raw)throw std::bad_alloc();const auto address=(reinterpret_cast<std::uintptr_t>(raw)+sizeof(AllocationHeader)+alignment-1)&~(std::uintptr_t(alignment)-1);auto*header=reinterpret_cast<AllocationHeader*>(address)-1;header->raw=raw;header->bytes=bytes;++live_allocations;++allocation_calls;live_bytes+=bytes;peak_bytes=std::max(peak_bytes,live_bytes);return reinterpret_cast<void*>(address);}
void free_test(void* pointer)noexcept{if(!pointer)return;auto*header=static_cast<AllocationHeader*>(pointer)-1;--live_allocations;live_bytes-=header->bytes;std::free(header->raw);}
void*operator new(std::size_t n){return allocate_test(n,alignof(std::max_align_t));}void*operator new[](std::size_t n){return allocate_test(n,alignof(std::max_align_t));}void*operator new(std::size_t n,std::align_val_t a){return allocate_test(n,std::size_t(a));}void*operator new[](std::size_t n,std::align_val_t a){return allocate_test(n,std::size_t(a));}
void operator delete(void*p)noexcept{free_test(p);}void operator delete[](void*p)noexcept{free_test(p);}void operator delete(void*p,std::size_t)noexcept{free_test(p);}void operator delete[](void*p,std::size_t)noexcept{free_test(p);}void operator delete(void*p,std::align_val_t)noexcept{free_test(p);}void operator delete[](void*p,std::align_val_t)noexcept{free_test(p);}void operator delete(void*p,std::size_t,std::align_val_t)noexcept{free_test(p);}void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{free_test(p);}
template<class Cost>void test(){for(int n:{65533,65534,1048576})for(int mode=0;mode<2;++mode)for(int budget=0;budget<6;++budget){TwoVariableSubmodularOpt<Cost> model(n);model.add_single_cost(0,Cost{0},Cost{-5});model.add_single_cost(1,Cost{0},Cost{3});model.add_psp_constraint(0,1);if(mode){assert(model.solve()==-2);model.add_all_true_profit({0,1,2},Cost{7});}const auto count=model.edge_count();const auto nodes=model.node_count();const Cost expected=mode?Cost{-9}:Cost{-2};allocation_budget=budget;bool failed=false;try{assert(model.solve()==expected);}catch(const std::bad_alloc&){failed=true;}allocation_budget=-1;assert(model.variable_count()==n&&model.node_count()==nodes&&model.edge_count()==count);assert(model.solve()==expected);assert(model.solve_with_assignment().first==expected);allocation_budget=0;model.clear();allocation_budget=-1;assert(model.solve()==0);(void)failed;}}
int main(){const auto before=live_allocations;test<std::int8_t>();test<std::int16_t>();test<std::int32_t>();test<std::int64_t>();assert(live_allocations==before&&live_bytes==0);std::cout<<"PASS large solve allocation retry\n";}
