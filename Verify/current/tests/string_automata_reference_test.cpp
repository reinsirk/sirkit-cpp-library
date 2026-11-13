// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <set>
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
#include SUFFIX_AUTOMATON_SOURCE
#include PALINDROMIC_TREE_SOURCE
bool palindrome(std::string_view s){for(std::size_t i=0;i<s.size()/2;++i)if(s[i]!=s[s.size()-1-i])return false;return true;}
std::uint64_t frequency(std::string_view text,std::string_view pattern){if(pattern.empty())return text.size()+1;std::uint64_t count=0;for(std::size_t i=0;i+pattern.size()<=text.size();++i)if(text.substr(i,pattern.size())==pattern)++count;return count;}
std::map<std::string,std::uint64_t> substrings(const std::string&s,bool pal=false){std::map<std::string,std::uint64_t>answer;for(std::size_t l=0;l<s.size();++l)for(std::size_t n=1;n<=s.size()-l;++n){auto t=s.substr(l,n);if(!pal||palindrome(t))++answer[t];}return answer;}
void lcs(const std::string&first,const std::string&second,const auto&sam){auto got=sam.longest_common_substring(second);std::size_t length=0,end=0;for(std::size_t e=1;e<=second.size();++e)for(std::size_t n=1;n<=e;++n)if(first.find(second.substr(e-n,n))!=std::string::npos&&n>length){length=n;end=e;}assert(got.length==length);if(length)assert(got.second_start==end-length);assert(first.substr(got.first_start,got.length)==second.substr(got.second_start,got.length));}
void verify(const std::string&s,auto&sam,auto&tree,std::vector<std::string>&creation,std::mt19937_64&rng){
 auto all=substrings(s),pals=substrings(s,true);assert(sam.size()==s.size()&&sam.empty()==s.empty()&&sam.distinct_substring_count()==all.size());assert(sam.state_count()<=2*s.size()+1);assert(tree.size()==s.size()&&tree.text()==s&&tree.distinct_palindrome_count()==pals.size());
 for(auto&[t,count]:pals)if(std::find(creation.begin(),creation.end(),t)==creation.end())creation.push_back(t);
 auto infos=tree.palindromes();assert(infos.size()==creation.size());
 std::size_t longest_suffix=0,longest=0,first=0;
 for(auto&[t,count]:pals){if(t.size()>longest||(t.size()==longest&&s.find(t)<first)){longest=t.size();first=s.find(t);}if(s.ends_with(t))longest_suffix=std::max(longest_suffix,t.size());}
 assert(tree.longest_suffix_length()==longest_suffix);const auto best=tree.longest_palindrome();assert(best.length==longest&&best.start==first);
 if(s.empty())assert(tree.last_node()==0);else{const auto suffix=s.substr(s.size()-longest_suffix);assert(tree.find(suffix)==tree.last_node());}
 for(std::size_t i=0;i<creation.size();++i){const auto&t=creation[i];const auto id=static_cast<std::int32_t>(i+1);const auto info=tree.node_info(id);assert(info.id==id&&info.length==static_cast<std::int32_t>(t.size())&&info.start==s.find(t)&&info.occurrences==pals[t]);assert(infos[i].id==id&&infos[i].length==info.length&&infos[i].start==info.start&&infos[i].occurrences==info.occurrences&&infos[i].parent==info.parent&&infos[i].suffix_link==info.suffix_link);assert(tree.find(t)==id&&tree.contains(t)&&tree.occurrences(t)==pals[t]);std::int32_t parent=t.size()==1?-1:t.size()==2?0:static_cast<std::int32_t>(std::find(creation.begin(),creation.end(),t.substr(1,t.size()-2))-creation.begin()+1);std::int32_t suffix=0;for(std::size_t k=t.size()-1;k>0;--k)if(palindrome(std::string_view(t).substr(t.size()-k))){suffix=static_cast<std::int32_t>(std::find(creation.begin(),creation.end(),t.substr(t.size()-k))-creation.begin()+1);break;}assert(info.parent==parent&&info.suffix_link==suffix&&tree.parent(id)==parent&&tree.suffix_link(id)==suffix);assert(tree.length(id)==info.length&&tree.first_position(id)==info.start);}
 assert(tree.node_info(-1).length==-1&&tree.node_info(0).occurrences==s.size()+1);
 for(int q=0;q<24;++q){std::string t;auto n=rng()%8;for(std::size_t i=0;i<n;++i)t+=static_cast<char>('a'+rng()%5);const auto f=frequency(s,t);assert(sam.contains(t)==(f>0)&&sam.occurrences(t)==f);const auto pf=palindrome(t)?f:0;assert(tree.contains(t)==(pf>0)&&tree.occurrences(t)==pf);}
 for(auto&[t,count]:all)assert(sam.occurrences(t)==count);
 std::string other;for(int q=0;q<20;++q)other+=static_cast<char>(rng()%5==0?'X':'a'+rng()%4);lcs(s,other,sam);
}
int main(){std::mt19937_64 rng(7338821);std::uint64_t prefixes=0;
 for(int trial=0;trial<180;++trial){std::string s;SuffixAutomaton<>sam;PalindromicTree<>tree;std::vector<std::string>creation;verify(s,sam,tree,creation,rng);const auto n=rng()%36;for(std::size_t i=0;i<n;++i){auto byte=static_cast<std::uint8_t>('a'+rng()%4);s+=static_cast<char>(byte);sam.append(byte);tree.append(byte);verify(s,sam,tree,creation,rng);++prefixes;}
  auto sc=sam;auto pc=tree;sc.append(static_cast<std::uint8_t>('d'));pc.append(static_cast<std::uint8_t>('d'));assert(sam.size()==s.size()&&tree.text()==s);auto sm=std::move(sc);auto pm=std::move(pc);assert(sc.empty()&&pc.empty()&&sc.occurrences("")==1&&pc.last_node()==0);sc.reset(s);pc.reset(s);assert(sc.distinct_substring_count()==sam.distinct_substring_count()&&pc.text()==s);sm=std::move(sm);pm=std::move(pm);sc=sam;pc=tree;swap(sc,sm);swap(pc,pm);sam.reset(s);tree.reset(tree.text());assert(sam.size()==s.size()&&tree.text()==s);sam.clear();tree.clear();assert(sam.empty()&&tree.empty());
 }
 PalindromicTree<>alias("abac");alias.append(alias.text().substr(1,2));assert(alias.text()=="abacba");
 std::string bytes;for(int i=0;i<70;++i)bytes+=static_cast<char>(rng()%256);bytes+=std::string("\0\x7f\x80\xff",4);SuffixAutomaton<256,0>bs(bytes);PalindromicTree<256,0>bp(bytes);auto all=substrings(bytes),pals=substrings(bytes,true);assert(bs.distinct_substring_count()==all.size()&&bp.distinct_palindrome_count()==pals.size());for(auto&[s,n]:all)assert(bs.contains(s)&&bs.occurrences(s)==n);for(auto&[s,n]:pals)assert(bp.occurrences(s)==n);lcs(bytes,bytes.substr(3),bs);
 for(unsigned value=0;value<256;++value){const std::string one(1,static_cast<char>(value)),other(1,static_cast<char>((value+1)%256));SuffixAutomaton<256,0> all_s(one);PalindromicTree<256,0> all_p(one);assert(all_s.contains(one)&&all_s.occurrences(one)==1&&all_p.contains(one)&&all_p.occurrences(one)==1);assert(!all_s.contains(other)&&!all_p.contains(other));}
 std::string zeros(7,'\0');SuffixAutomaton<1,0> zs(zeros);PalindromicTree<1,0> zp(zeros);assert(zs.distinct_substring_count()==7&&zs.occurrences(zeros.substr(0,3))==5&&zp.occurrences(zeros.substr(0,3))==5);
 const std::string top_bytes("\xfe\xff\xfe",3);SuffixAutomaton<2,254> ts(top_bytes);PalindromicTree<2,254> tp(top_bytes);assert(ts.distinct_substring_count()==5&&tp.longest_suffix_length()==3);
 std::string high(35,static_cast<char>(255));SuffixAutomaton<1,255>hs(high);PalindromicTree<1,255>hp(high);assert(hs.distinct_substring_count()==35&&hs.occurrences(high.substr(0,7))==29&&hp.occurrences(high.substr(0,7))==29);assert(!hs.contains("a")&&!hp.contains("a"));
#ifndef SKIP_RUNTIME_CHECKS
 bool caught=false;SuffixAutomaton<>s("ab");try{s.append(std::string_view("cdXef"));}catch(const std::invalid_argument&){caught=true;}assert(caught&&s.size()==4&&s.contains("abcd"));PalindromicTree<>p("ab");caught=false;try{p.append(std::string_view("cdXef"));}catch(const std::invalid_argument&){caught=true;}assert(caught&&p.text()=="abcd");caught=false;try{p.node_info(100);}catch(const std::out_of_range&){caught=true;}assert(caught);
#endif
 std::cout<<"string automata prefixes="<<prefixes<<" exhaustive substring/frequency/LCS/ownership/byte boundaries OK\n";
}
