// SPDX-License-Identifier: CC0-1.0
#ifndef TRIE_HEADER
#define TRIE_HEADER "../../../String/Trie.cpp"
#endif
#include TRIE_HEADER
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <map>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
static volatile int budget=-1;
static volatile std::size_t live=0;
[[gnu::noinline]] void* operator new(std::size_t n){if(budget==0)throw std::bad_alloc();if(budget>0)budget=budget-1;if(void*p=std::malloc(n?n:1)){live=live+1;return p;}throw std::bad_alloc();}
[[gnu::noinline]] void* operator new[](std::size_t n){return ::operator new(n);}
[[gnu::noinline]] void* operator new(std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
[[gnu::noinline]] void* operator new[](std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new[](n);}catch(...){return nullptr;}}
static void release(void*p)noexcept{if(p)live=live-1;std::free(p);}
void operator delete(void*p)noexcept{release(p);}void operator delete[](void*p)noexcept{release(p);}
void operator delete(void*p,std::size_t)noexcept{release(p);}void operator delete[](void*p,std::size_t)noexcept{release(p);}
void operator delete(void*p,const std::nothrow_t&)noexcept{release(p);}void operator delete[](void*p,const std::nothrow_t&)noexcept{release(p);}
struct Lex{bool operator()(std::string_view a,std::string_view b)const{const auto n=std::min(a.size(),b.size());for(std::size_t i=0;i<n;++i){const auto x=static_cast<unsigned char>(a[i]),y=static_cast<unsigned char>(b[i]);if(x!=y)return x<y;}return a.size()<b.size();}};
using Ref=std::map<std::string,std::size_t,Lex>;
static std::uint64_t checks=0,failures=0;
static void require(bool x){++checks;assert(x);}
static std::optional<std::string> oracle(const Ref&r,std::string_view q){const auto it=r.lower_bound(std::string(q));return it==r.end()?std::nullopt:std::optional<std::string>(it->first);}
static void check(const Trie&t,const Ref&r,std::string_view q){const auto nodes=t.node_count(),cap=t.capacity();require(t.lower_bound(q)==oracle(r,q));require(t.node_count()==nodes&&t.capacity()==cap);}
static void insert(Trie&t,Ref&r,const std::string&s,std::size_t n=1){require(t.insert(s,n)==r[s]+n);r[s]+=n;}
static void erase(Trie&t,Ref&r,const std::string&s){const auto it=r.find(s);const auto n=it==r.end()?0:it->second;require(t.erase_all(s)==n);r.erase(s);}
static std::vector<std::string> queries(){std::vector<std::string>q{std::string()};for(unsigned x=0;x<256;++x)q.emplace_back(1,static_cast<char>(x));const unsigned bytes[]={0,1,127,128,254,255};for(unsigned x:bytes)for(unsigned y:bytes){q.push_back(std::string(1,static_cast<char>(x))+static_cast<char>(y));for(unsigned z:bytes)q.push_back(std::string(1,static_cast<char>(x))+static_cast<char>(y)+static_cast<char>(z));}q.push_back(std::string(4,static_cast<char>(255)));return q;}
static void small(){const auto q=queries();Trie t;Ref r;for(const auto&s:q)check(t,r,s);
 for(std::size_t i=0;i<q.size();++i)if(i%3==0)insert(t,r,q[i],i%5+1);for(const auto&s:q){check(t,r,s);check(t,r,s+static_cast<char>(255));}
 for(std::size_t i=0;i<q.size();++i)if(i%2==0)erase(t,r,q[i]);for(const auto&s:q)check(t,r,s);
 auto copied=t;Trie moved(std::move(copied));require(copied.empty());for(const auto&s:q){check(moved,r,s);check(copied,Ref{},s);}
 t.clear();r.clear();for(unsigned x=0;x<256;++x)insert(t,r,std::string(1,static_cast<char>(x)));for(unsigned x=0;x<253;++x)erase(t,r,std::string(1,static_cast<char>(x)));for(const auto&s:q)check(t,r,s);
 for(std::size_t i=0;i<q.size();i+=7)insert(t,r,q[i],2);for(const auto&s:q)check(t,r,s);
}
static void ancestors(){Trie t;Ref r;for(const auto&s:{"abxa","abzc","acy","bz"})insert(t,r,s);for(const auto&q:{"","aa","ab","abx","abxa","abxaa","abxb","abz","abzz","azzz","bz","bzz","c"})check(t,r,q);
 for(const auto&s:{"abxa","abzc","acy","bz"}){erase(t,r,s);for(const auto&q:{"abxb","abzz","azzz","c"})check(t,r,q);}
}
static void huge_counts(){Trie t;Ref r;const auto maximum=std::numeric_limits<std::size_t>::max();insert(t,r,"",maximum-3);insert(t,r,std::string(1,'\0'));insert(t,r,std::string(1,static_cast<char>(255)),2);for(const auto&q:queries())check(t,r,q);erase(t,r,"");insert(t,r,std::string(1,static_cast<char>(255)),maximum-3);for(const auto&q:queries())check(t,r,q);}
static void allocation_queries(Trie&t,const Ref&r,const std::vector<std::string>&q){const auto saved=t.items();const auto nodes=t.node_count(),size=t.size();
 for(const auto&word:q){const auto expected=oracle(r,word);bool success=false;for(int allowed=0;allowed<64;++allowed){const auto before=live;bool failed=false;budget=allowed;try{auto got=t.lower_bound(word);budget=-1;require(got==expected);}catch(const std::bad_alloc&){budget=-1;failed=true;++failures;}require(live==before);require(t.node_count()==nodes&&t.size()==size);require(t.items()==saved);if(!failed){success=true;break;}}require(success);}
}
static void long_words(){Trie t;Ref r;std::string p(100000,'\0');for(std::size_t i=0;i<p.size();++i)p[i]=static_cast<char>((i*193)%256);const std::string prefix="a"+p,lo=prefix+std::string(1,'\0'),hi=prefix+static_cast<char>(255);insert(t,r,lo,2);insert(t,r,hi,3);insert(t,r,"b");
 std::vector<std::string>q{"",prefix,prefix.substr(0,50000),lo,hi,lo+'\1',prefix+static_cast<char>(128),hi+static_cast<char>(255),std::string(1,static_cast<char>(255))};for(const auto&word:q)check(t,r,word);allocation_queries(t,r,q);
 erase(t,r,hi);for(const auto&word:q)check(t,r,word);erase(t,r,lo);std::reverse(p.begin(),p.end());insert(t,r,"c"+p,4);for(const auto&word:q)check(t,r,word);check(t,r,"c"+p);check(t,r,"c"+p+static_cast<char>(255));
 t.clear();r.clear();for(const auto&s:{std::string(257,'\0'),std::string(257,static_cast<char>(255))})insert(t,r,s);for(const auto&s:{std::string(),std::string(256,'\0'),std::string(258,'\0'),std::string(256,static_cast<char>(255)),std::string(258,static_cast<char>(255))})check(t,r,s);
}
int main(){const auto before=live;small();ancestors();huge_counts();long_words();require(live==before);std::printf("PASS direct bound checks=%ju allocation_failures=%ju live_delta=%td\n",static_cast<std::uintmax_t>(checks),static_cast<std::uintmax_t>(failures),std::ptrdiff_t(live)-std::ptrdiff_t(before));}
