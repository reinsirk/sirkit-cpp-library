// SPDX-License-Identifier: CC0-1.0
#ifndef AHO_HEADER
#define AHO_HEADER "../../../String/Aho_Corasick.cpp"
#endif
#include AHO_HEADER
#include AHO_HEADER
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>
#include <string>
#include <tuple>
namespace {
std::uint64_t checks=0;void check(bool v){++checks;if(!v)std::abort();}
template<class E,class F>void rejects(F f){bool yes=false;try{f();}catch(const E&){yes=true;}check(yes);}
void test(const std::vector<std::string>&patterns,const std::string&text){AhoCorasick<4,'a'>a;std::map<std::string,int>ids{{"",0}};std::vector<std::string>labels(1);std::vector<int>ends;for(const auto&p:patterns){for(std::size_t j=1;j<=p.size();++j){const auto prefix=p.substr(0,j);if(!ids.contains(prefix)){const int id=static_cast<int>(labels.size());ids.emplace(prefix,id);labels.push_back(prefix);}}const int id=a.add(p);check(id==static_cast<int>(ends.size()));ends.push_back(ids[p]);check(a.pattern_length(id)==p.size());check(a.pattern_state(id)==ends.back());}a.build();a.build();check(a.state_count()==static_cast<int>(labels.size()));for(std::size_t i=0;i<labels.size();++i){const auto&s=labels[i];check(a.parent(static_cast<int>(i))==(s.empty()?0:ids.at(s.substr(0,s.size()-1))));int failure=0;for(std::size_t offset=1;offset<s.size();++offset)if(ids.contains(s.substr(offset))){failure=ids.at(s.substr(offset));break;}check(a.failure(static_cast<int>(i))==failure);std::uint64_t outputs=0;for(const auto&p:patterns)outputs+=s.size()>=p.size()&&s.compare(s.size()-p.size(),p.size(),p)==0;check(a.output_count(static_cast<int>(i))==outputs);for(char c='a';c<'e';++c){auto next=s+c;while(!ids.contains(next))next.erase(next.begin());check(a.step(static_cast<int>(i),static_cast<unsigned char>(c))==ids.at(next));}}
std::vector<std::uint64_t>counts(patterns.size());std::vector<AhoMatch>matches;for(std::size_t id=0;id<patterns.size();++id)for(std::size_t start=0;start<=text.size();++start)if(patterns[id].size()<=text.size()-start&&text.compare(start,patterns[id].size(),patterns[id])==0){++counts[id];matches.push_back({static_cast<int>(id),start,start+patterns[id].size()});}check(a.match_counts(text)==counts);check(a.count_total(text)==matches.size());auto actual=a.find_all(text);std::size_t previous=0;for(const auto&m:actual){check(m.end>=previous);previous=m.end;}const auto less=[](const AhoMatch&x,const AhoMatch&y){return std::tie(x.end,x.pattern,x.begin)<std::tie(y.end,y.pattern,y.begin);};std::sort(actual.begin(),actual.end(),less);std::sort(matches.begin(),matches.end(),less);check(actual==matches);std::vector<AhoMatch>callback;a.for_each_match(text,[&](int id,std::size_t l,std::size_t r){callback.push_back({id,l,r});});std::sort(callback.begin(),callback.end(),less);check(callback==matches);
auto copied=a;copied.clear();check(a.match_counts(text)==counts);check(copied.empty()&&copied.state_count()==1&&copied.parent(0)==0);copied.build();check(copied.count_total(text)==0);auto moved=std::move(a);check(moved.match_counts(text)==counts);check(a.empty()&&a.state_count()==1);a.add("a");a.build();check(a.count_total("aaa")==3);AhoCorasick<4,'a'>assigned;assigned=moved;check(assigned.match_counts(text)==counts);auto*alias=&assigned;assigned=*alias;check(assigned.match_counts(text)==counts);assigned=std::move(*alias);check(assigned.match_counts(text)==counts);
#ifndef SKIP_RUNTIME_CHECKS
 rejects<std::logic_error>([&]{moved.add("a");});rejects<std::out_of_range>([&]{(void)moved.step(-1,'a');});rejects<std::out_of_range>([&]{moved.count_total("z");});
#endif
}
}
int main(){std::mt19937_64 rng(0xa8c016a2026ULL);for(int rep=0;rep<600;++rep){std::vector<std::string>patterns(rng()%32);for(auto&s:patterns){s.resize(rng()%14);for(auto&c:s)c=static_cast<char>('a'+rng()%4);}std::string text(rng()%160,'a');for(auto&c:text)c=static_cast<char>('a'+rng()%4);test(patterns,text);}test({"","","a","aa","aaa","a","ba","caba"},"aaaacaba");test({},"");test({"a","aaaa","aaaaaaaa","a"},std::string(500,'a'));
ByteAhoCorasick bytes;std::vector<std::string>p;std::string text(1024,' ');for(auto&c:text)c=static_cast<char>(rng()%256);for(int i=0;i<400;++i){const auto length=static_cast<std::size_t>(rng()%11);p.push_back(text.substr(rng()%(text.size()-length+1),length));bytes.add(p.back());}bytes.build();auto counts=bytes.match_counts(text);for(std::size_t id=0;id<p.size();++id){std::uint64_t expected=0;for(std::size_t at=0;at<=text.size();++at)expected+=p[id].size()<=text.size()-at&&text.compare(at,p[id].size(),p[id])==0;check(counts[id]==expected);}AhoCorasick<>empty;empty.reserve_nodes(100);empty.reserve_patterns(20);check(empty.node_capacity()>=100);rejects<std::length_error>([&]{empty.reserve_nodes(std::numeric_limits<std::size_t>::max());});
#ifndef SKIP_RUNTIME_CHECKS
 rejects<std::logic_error>([&]{empty.count_total("a");});rejects<std::out_of_range>([&]{empty.add("z!");});
#endif
std::printf("Aho reference checks=%ju\n",static_cast<std::uintmax_t>(checks));}
