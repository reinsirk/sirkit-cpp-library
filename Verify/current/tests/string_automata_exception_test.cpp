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
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#ifndef SUFFIX_AUTOMATON_SOURCE
#define SUFFIX_AUTOMATON_SOURCE "../../../String/Suffix_Automaton.cpp"
#endif
#ifndef PALINDROMIC_TREE_SOURCE
#define PALINDROMIC_TREE_SOURCE "../../../String/Palindromic_Tree.cpp"
#endif
#include SUFFIX_AUTOMATON_SOURCE
#include PALINDROMIC_TREE_SOURCE
using SAM=SuffixAutomaton<>;using PAL=PalindromicTree<>;
void verify(const SAM&s){assert(s.size()==12&&s.contains("abacabadabac")&&s.distinct_substring_count()>0&&s.occurrences("aba")==3);}
void verify(const PAL&p){assert(p.text()=="abacabadabac"&&p.occurrences("aba")==3&&p.longest_suffix_length()==9);}
void allocation_failures(){
 const std::string text="abacabadabac",more(200,'a');SAM s(text);PAL p(text);verify(s);verify(p);int failures=0;
 for(int kind=0;kind<2;++kind)for(int operation=0;operation<9;++operation)for(int allowance=0;allowance<30;++allowance){SAM target=s;PAL pal=p;const auto before=allocations_live;fail_after=allowance;bool failed=false;
  try{if(kind==0){switch(operation){case 0:target.append(static_cast<std::uint8_t>('b'));break;case 1:target.append(more);break;case 2:target.reset(more);break;case 3:target=s;break;case 4:{SAM copy(s);assert(copy.size()==12);break;}case 5:{SAM built(more);assert(built.size()==200);break;}case 6:target.reserve(2000);break;case 7:target.occurrences("aba");break;case 8:target.append("ab");target.occurrences("ab");break;}}
  else{switch(operation){case 0:pal.append(static_cast<std::uint8_t>('b'));break;case 1:pal.append(more);break;case 2:pal.reset(more);break;case 3:pal=p;break;case 4:{PAL copy(p);assert(copy.size()==12);break;}case 5:{PAL built(more);assert(built.size()==200);break;}case 6:pal.reserve(2000);break;case 7:pal.palindromes();break;case 8:pal.append(pal.text());break;}}}
  catch(const std::bad_alloc&){failed=true;++failures;}fail_after=-1;
  if(failed&&operation!=8){verify(target);verify(pal);} // operation8は追加成功後のキャッシュ確保が失敗する場合を含む。
  if(failed&&operation==8){if(kind==0)assert(target.size()==12||target.size()==14);else verify(pal);}
  verify(s);verify(p);if(!failed)break;
 }
 assert(failures>25);std::printf("string allocation injected failures=%d\n",failures);
 // キャッシュ確保だけの失敗を分離し、文字列と構造が保たれることを確認。
 SAM sc("abracadabra");PAL pc("abracadabra");fail_after=0;bool caught=false;try{sc.occurrences("a");}catch(const std::bad_alloc&){caught=true;}fail_after=-1;assert(caught&&sc.contains("abracadabra")&&sc.occurrences("a")==5);
 fail_after=0;caught=false;try{pc.occurrences("a");}catch(const std::bad_alloc&){caught=true;}fail_after=-1;assert(caught&&pc.text()=="abracadabra"&&pc.occurrences("a")==5);
 SAM reserved;PAL pres;reserved.reserve(1000);pres.reserve(1000);fail_after=0;reserved.append("abacabadabacaba");pres.append("abacabadabacaba");for(int i=0;i<50;++i){reserved.append(static_cast<std::uint8_t>('c'));pres.append(static_cast<std::uint8_t>('c'));}fail_after=-1;assert(reserved.size()==65&&pres.size()==65);
 bool length=false;try{reserved.reserve(std::numeric_limits<std::size_t>::max());}catch(const std::length_error&){length=true;}assert(length&&reserved.size()==65);length=false;try{pres.reserve(std::numeric_limits<std::size_t>::max());}catch(const std::length_error&){length=true;}assert(length&&pres.size()==65);
}
int main(){assert(allocations_live==0);{allocation_failures();}assert(allocations_live==0);std::printf("normal_live0=%" PRId64 "\n",allocations_live);}
