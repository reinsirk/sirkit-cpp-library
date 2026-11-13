// SPDX-License-Identifier: CC0-1.0
#include "wuf_groups_test_model.hpp"
#include <new>
static volatile std::ptrdiff_t countdown=-1;
static volatile std::size_t live=0;
[[gnu::noinline]] void* operator new(std::size_t n){if(countdown==0)throw std::bad_alloc();if(countdown>0)countdown=countdown-1;if(void* p=std::malloc(n?n:1)){live=live+1;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void* operator new(std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
[[gnu::noinline]] void* operator new[](std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new[](n);}catch(...){return nullptr;}}
[[gnu::noinline]] static void release(void* p)noexcept{if(p)live=live-1;std::free(p);}
void operator delete(void* p)noexcept{release(p);}void operator delete[](void* p)noexcept{release(p);}
void operator delete(void* p,std::size_t)noexcept{release(p);}void operator delete[](void* p,std::size_t)noexcept{release(p);}
void operator delete(void* p,const std::nothrow_t&)noexcept{release(p);}
void operator delete[](void* p,const std::nothrow_t&)noexcept{release(p);}
int main(){const auto initial=live;std::size_t failures=0;
    for(int n:{0,1,17,257,513})for(int block:{1,16,1024})for(int order:{2,3,4}){
        bool success=false;
        for(int budget=0;budget<1024;++budget){
            Model model(n,block,order);
            UnionFindW<std::int64_t> uf(n);model.build(uf);const auto expected=model.groups();
            // 確保失敗の各予算で、未圧縮の森から試す。
            const auto before=live;bool failed=false;countdown=budget;
            try{auto got=uf.groups();countdown=-1;require(got==expected);}
            catch(const std::bad_alloc&){countdown=-1;failed=true;++failures;}
            require(live==before);model.observe(uf);require(uf.groups()==expected);
            if(!failed){success=true;break;}
        }
        require(success);
    }
    require(live==initial);
    std::printf("PASS groups allocation checks=%llu failures=%zu live_delta=%td\n",static_cast<unsigned long long>(checks),failures,std::ptrdiff_t(live)-std::ptrdiff_t(initial));
}
