// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <new>
static volatile std::int64_t allocations_live=0,fail_after=-1;
static void allocation_check(){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;}
[[gnu::noinline]] void* operator new(std::size_t bytes){allocation_check();if(void*p=std::malloc(bytes?bytes:1)){++allocations_live;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void operator delete(void*p)noexcept{if(p){--allocations_live;std::free(p);}}
[[gnu::noinline]] void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void* operator new[](std::size_t bytes){return ::operator new(bytes);}
[[gnu::noinline]] void operator delete[](void*p)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void* operator new(std::size_t bytes,std::align_val_t alignment){allocation_check();void*p=nullptr;if(posix_memalign(&p,static_cast<std::size_t>(alignment),bytes?bytes:1)==0){++allocations_live;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t bytes,std::align_val_t alignment){return ::operator new(bytes,alignment);}
[[gnu::noinline]] void operator delete(void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete(void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
[[gnu::noinline]] void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#ifndef DUAL_SEGMENT_SOURCE
#define DUAL_SEGMENT_SOURCE "../../../DataStructure/Dual_Segment_Tree.cpp"
#endif
#include DUAL_SEGMENT_SOURCE
using I=std::int64_t;
struct Budget{int map=-1,compose=-1;};
void step(int& n){if(n==0)throw std::runtime_error("injected");if(n>0)--n;}
struct Map{std::shared_ptr<Budget>b;I operator()(I f,I x)const{step(b->map);return x+f;}};
struct Comp{std::shared_ptr<Budget>b;I operator()(I f,I g)const{step(b->compose);return f+g;}};
using Tree=DualSegmentTree<I,I,Map,Comp>;
void verify(Tree& t){assert(t.usable());auto v=t.values();for(std::size_t i=0;i<v.size();++i)assert(v[i]==static_cast<I>(i));}
void operation_failures(){
 auto b=std::make_shared<Budget>();std::vector<I>a(37);for(std::size_t i=0;i<a.size();++i)a[i]=i;Tree source(a,Map{b},Comp{b});int failures=0;
 for(int kind=0;kind<2;++kind)for(int allowance=0;allowance<20;++allowance)for(int operation=0;operation<7;++operation){
  Tree target=source;target.apply(0,37,5);target.apply(2,36,7);*b=Budget{};if(kind==0)b->map=allowance;else b->compose=allowance;bool failed=false;
  try{switch(operation){case 0:target.apply(1,35,8);break;case 1:target.get(3);break;case 2:target.set(4,3);break;case 3:target.transform(4,[](I x){return x+1;});break;case 4:target.values();break;case 5:target.apply(4,7);break;case 6:target.apply(0,37,1);target.apply(0,37,2);break;}}
  catch(const std::runtime_error&){failed=true;++failures;}*b=Budget{};
  if(failed){assert(!target.usable());try{target.get(0);assert(false);}catch(const std::logic_error&){}auto copied=target;assert(!copied.usable());auto moved=std::move(target);assert(target.empty()&&target.usable()&&!moved.usable());moved.reset(a);verify(moved);target.clear();assert(target.empty()&&target.usable());}else assert(target.usable());verify(source);
 }
 assert(failures>50);std::printf("dual operation failures=%d\n",failures);
 auto t=source;try{t.transform(0,[](I)->I{throw std::runtime_error("transform");});assert(false);}catch(const std::runtime_error&){}assert(!t.usable());
}
void allocation_failures(){
 auto b=std::make_shared<Budget>();std::vector<I>a(37);for(std::size_t i=0;i<a.size();++i)a[i]=i;Tree source(a,Map{b},Comp{b});std::vector<I>next(200,3);int failures=0;
 for(int operation=0;operation<4;++operation)for(int allowance=0;allowance<20;++allowance){Tree t=source;auto before=allocations_live;fail_after=allowance;bool failed=false;
  try{if(operation==0)t.reset(next);if(operation==1)t=source;if(operation==2){Tree copy(source);assert(copy.size()==37);}if(operation==3){Tree built(next,Map{b},Comp{b});assert(built.size()==200);}}
  catch(const std::bad_alloc&){failed=true;++failures;}fail_after=-1;if(failed){assert(allocations_live==before);verify(t);}verify(source);if(!failed)break;
 }
 auto t=source;fail_after=0;t.apply(0,37,5);t.apply(2,36,3);t.set(4,9);t.transform(3,[](I x){return x+1;});assert(t.get(3)==12);fail_after=-1;
 assert(failures>5);std::printf("dual allocation failures=%d no numeric operation allocation\n",failures);
}
struct Tracked{static inline int live=0,budget=-1;I value;explicit Tracked(I x):value(x){step(budget);++live;}Tracked(const Tracked&x):value(x.value){step(budget);++live;}Tracked(Tracked&&x):value(x.value){step(budget);++live;}Tracked&operator=(const Tracked&x){step(budget);value=x.value;return *this;}Tracked&operator=(Tracked&&x){step(budget);value=x.value;return *this;}~Tracked(){--live;}};
struct TMap{Tracked operator()(const Tracked&f,const Tracked&x)const{return Tracked(f.value+x.value);}};
struct TComp{Tracked operator()(const Tracked&f,const Tracked&g)const{return Tracked(f.value+g.value);}};
void value_failures(){int failures=0;std::vector<Tracked>a;for(int i=0;i<12;++i)a.emplace_back(i);using TT=DualSegmentTree<Tracked,Tracked,TMap,TComp>;TT source(a,{},{});Tracked action(3);
 for(int operation=0;operation<5;++operation)for(int allowance=0;allowance<60;++allowance){TT target=source;target.apply(0,12,action);auto before=Tracked::live;Tracked::budget=allowance;bool failed=false;try{if(operation==0)target.apply(1,11,action);if(operation==1)target.values();if(operation==2)target.get(4);if(operation==3)target.reset(a);if(operation==4)target=source;}catch(const std::runtime_error&){failed=true;++failures;}Tracked::budget=-1;
  if(failed&&operation>=3){assert(Tracked::live==before&&target.usable());}else if(failed)assert(!target.usable());assert(source.get(4).value==4);if(!failed)break;
 }assert(failures>50);std::printf("dual value/action failures=%d\n",failures);
}
struct SMap{std::string operator()(const std::string&f,const std::string&x)const{return x+f;}};
struct SComp{std::string operator()(const std::string&f,const std::string&g)const{return g+f;}};
void string_failures(){using ST=DualSegmentTree<std::string,std::string,SMap,SComp>;ST source(std::vector<std::string>(17,std::string(100,'x')),{},{});std::string f(200,'y');int failures=0;for(int allowance=0;allowance<100;++allowance){auto t=source;fail_after=allowance;bool failed=false;try{t.apply(0,17,f);t.apply(1,16,f);t.values();}catch(const std::bad_alloc&){failed=true;++failures;}fail_after=-1;if(failed)assert(!t.usable());else break;}assert(failures>10);std::printf("dual string allocation failures=%d\n",failures);}
int main(){assert(allocations_live==0);{operation_failures();allocation_failures();value_failures();string_failures();}assert(allocations_live==0&&Tracked::live==0);std::printf("normal_live0=%" PRId64 " tracked_live0=%d\n",allocations_live,Tracked::live);}
