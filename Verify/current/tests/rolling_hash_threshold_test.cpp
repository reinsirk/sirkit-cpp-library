// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RH_SOURCE
#define SIRKIT_RH_SOURCE "../../../String/RollingHash.cpp"
#endif
#include SIRKIT_RH_SOURCE
#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#if defined(__unix__)
#include <sys/mman.h>
#include <unistd.h>
#endif
namespace {
using Big=boost::multiprecision::cpp_int;
constexpr auto p=RollingHash::modulus;
std::size_t checks=0,cases=0;
void require(bool ok){++checks;if(!ok)std::abort();}
std::uint64_t residue(const Big&x){return (x%p).convert_to<std::uint64_t>();}
class Input {
#if defined(__unix__)
 void* mapping_=nullptr;std::size_t bytes_=0;char* data_=nullptr;
#else
 std::string storage_;
#endif
 std::size_t size_=0;
public:
 Input(std::string_view s,bool at_end):size_(s.size()){
#if defined(__unix__)
  const auto page=static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));require(page>0);const auto count=std::max(std::size_t{1},(size_+page-1)/page);bytes_=(count+2)*page;
  mapping_=::mmap(nullptr,bytes_,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);require(mapping_!=MAP_FAILED);auto* begin=static_cast<char*>(mapping_)+page;
  if(::mprotect(begin,count*page,PROT_READ|PROT_WRITE)!=0){::munmap(mapping_,bytes_);std::abort();}
  data_=at_end?begin+count*page-size_:begin;if(size_)std::memcpy(data_,s.data(),size_);
  if(::mprotect(begin,count*page,PROT_READ)!=0){::munmap(mapping_,bytes_);std::abort();}
#else
  static_cast<void>(at_end);storage_=s;
#endif
 }
 Input(const Input&)=delete;Input&operator=(const Input&)=delete;
 ~Input(){
#if defined(__unix__)
  ::munmap(mapping_,bytes_);
#endif
 }
 std::string_view view()const{
#if defined(__unix__)
  return {data_,size_};
#else
  return storage_;
#endif
 }
};
void inspect(const RollingHash&h,const std::string&s,const std::vector<std::uint64_t>&prefix,const std::vector<std::uint64_t>&powers){
 require(h.size()==s.size());require(h.whole()==prefix.back());
 for(std::size_t i=0;i<=s.size();++i){require(h.get(0,i)==prefix[i]);if(i%37==0||i+9>=s.size()){const auto l=i/3;require(h.get(l,i)==residue(Big(prefix[i])+Big(p)*p-Big(prefix[l])*powers[i-l]));}}
 require(h.lcp(0,h,0)==s.size());require(h.compare(0,s.size(),h,0,h.size())==0);require(h.equal(0,s.size(),h,0,h.size()));
}
}
int main(){
 const std::array<std::size_t,15> lengths{0,1,4094,4095,4096,4097,4098,4099,4100,4101,8191,8192,8193,8194,8195};
 for(auto raw_base:{std::uint64_t{0},std::uint64_t{2},std::uint64_t{256},p/2,p-2})for(auto n:lengths)for(int pattern=0;pattern<4;++pattern){
  const auto base=raw_base<2?10:raw_base;std::string s(n,'\0');for(std::size_t i=0;i<n;++i){unsigned c=pattern==0?static_cast<unsigned>((i*137+19)%256):pattern==1?0:pattern==2?255:i<61?1:0;s[i]=static_cast<char>(c);}
  std::vector<std::uint64_t> prefix(n+1),powers(n+1,1);for(std::size_t i=0;i<n;++i){prefix[i+1]=residue(Big(prefix[i])*base+static_cast<unsigned char>(s[i]));powers[i+1]=residue(Big(powers[i])*base);}
  for(bool end:{false,true}){++cases;std::optional<RollingHash> h;{
    Input input(s,end);h.emplace(input.view(),raw_base);inspect(*h,s,prefix,powers);
    h->assign(input.view());inspect(*h,s,prefix,powers);
    h->clear();h->assign(input.view());inspect(*h,s,prefix,powers);
    RollingHash grow("old",raw_base);grow.assign(input.view());inspect(grow,s,prefix,powers);
   }
   inspect(*h,s,prefix,powers);RollingHash copied(*h);h->clear();inspect(copied,s,prefix,powers);
  }
 }
 std::printf("PASS threshold checks=%zu cases=%zu guard_pages=%d\n",checks,cases,
#if defined(__unix__)
 1
#else
 0
#endif
 );
}
