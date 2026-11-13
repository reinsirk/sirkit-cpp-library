// SPDX-License-Identifier: CC0-1.0
// 積の戻り値の型要件と、遅延伝播時の所有型・bool proxyを検証する。
#ifndef SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#define SIRKIT_LAZY_SEGMENT_TEST_SOURCE "../../../DataStructure/Lazy_Segment_Tree.cpp"
#endif
#include SIRKIT_LAZY_SEGMENT_TEST_SOURCE
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// move ctorを明示的にdeleteしても、従来のprodは利用できる。
namespace CopyOnly {
struct S {
 std::int64_t sum;
 S()=delete;
 explicit S(std::int64_t x):sum(x){}
 S(const S&)=default;
 S(S&&)=delete;
 S&operator=(const S&)=default;
 S&operator=(S&&)=default;
};
static_assert(std::is_copy_constructible_v<S>);
static_assert(!std::is_move_constructible_v<S>);
S op(const S&a,const S&b){return S(a.sum+b.sum);}
S e(){return S(0);}
S mapping(const int&f,const S&v,std::size_t length){return S(v.sum+f*static_cast<std::int64_t>(length));}
int compose(const int&f,const int&g){return f+g;}
int id(){return 0;}
void run(){
 for(int n:{0,1,2,3,7,8,9,31,32,33,65}){
  lazy_segtree<S,op,e,int,mapping,compose,id> tree(n);
  std::vector<std::int64_t> values(n);
  for(int step=0;step<40;++step){
   if(n){int l=(step*11)%n,r=l+1+(step*7)%(n-l);tree.apply(l,r,step%5);for(int i=l;i<r;++i)values[i]+=step%5;tree.set(step%n,S(step));values[step%n]=step;}
   std::int64_t total=0;for(int i=0;i<n;++i){if(tree.get(i).sum!=values[i])std::abort();total+=values[i];}if(tree.all_prod().sum!=total)std::abort();
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){std::int64_t want=0;for(int i=l;i<r;++i)want+=values[i];if(tree.prod(l,r).sum!=want)std::abort();}
  }
 }
 std::puts("PASS copy-only S: range mapping/set/all query intervals");
}
}

// 非default構築の所有型の非可換積と、F=boolのタグを照合する。
namespace Noncommutative {
struct Word {
 std::string text;
 Word()=delete;
 explicit Word(std::string s):text(std::move(s)){}
};
Word cat(const Word&a,const Word&b){return Word(a.text+b.text);}
Word empty(){return Word("");}
Word shift(const int&f,const Word&s){auto result=s.text;for(char&c:result)c=static_cast<char>('a'+(c-'a'+f)%26);return Word(std::move(result));}
int comp(const int&f,const int&g){return (f+g)%26;}
int id(){return 0;}
bool yes(){return true;}
bool no(){return false;}
bool bmap(const bool&f,const bool&x){return f||x;}
bool bcomp(const bool&f,const bool&g){return f||g;}
void run(){
 std::mt19937 rng(13147);unsigned checks=0;
 for(int n:{1,2,3,7,8,9,16,17,33,65}){
  std::vector<Word> initial(n,Word("a"));std::vector<std::string> ref(n,"a");
  lazy_segtree<Word,cat,empty,int,shift,comp,id> t(initial);
  std::vector<bool> bits(n,false);lazy_segtree<bool,std::logical_and<bool>{},yes,bool,bmap,bcomp,no> b(bits);
  for(int step=0;step<25;++step){
   int l=rng()%n,r=l+1+rng()%(n-l),f=rng()%26;t.apply(l,r,f);b.apply(l,r,true);
   for(int i=l;i<r;++i){for(char&c:ref[i])c=static_cast<char>('a'+(c-'a'+f)%26);bits[i]=true;}
   const int p=rng()%n;ref[p]=std::string(1+rng()%3,static_cast<char>('a'+rng()%26));t.set(p,Word(ref[p]));bits[p]=false;b.set(p,false);
   for(int x=0;x<=n;++x)for(int y=x;y<=n;++y){std::string want;bool wb=true;for(int i=x;i<y;++i){want+=ref[i];wb=wb&&bits[i];}if(t.prod(x,y).text!=want||b.prod(x,y)!=wb)std::abort();++checks;}
  }
 }
 std::printf("PASS nondefault owning noncommutative S and vector<bool>: %u interval checks\n",checks);
}
}

// 共有する不変タグの所有権、合成順、3引数mappingの物理幅を照合する。
namespace OwningTag {
constexpr std::uint64_t mod=1000000007;
int live=0;
struct Rule { std::uint64_t a,b;Rule(std::uint64_t x,std::uint64_t y):a(x),b(y){++live;}~Rule(){--live;} };
struct Tag { std::shared_ptr<const Rule> rule;Tag()=delete;Tag(std::uint64_t a,std::uint64_t b):rule(std::make_shared<const Rule>(a,b)){} };
std::uint64_t op(const std::uint64_t&a,const std::uint64_t&b){return (a+b)%mod;}
std::uint64_t e(){return 0;}
Tag compose(const Tag&newer,const Tag&older){return Tag(newer.rule->a*older.rule->a%mod,(newer.rule->a*older.rule->b+newer.rule->b)%mod);}
Tag id(){return Tag(1,0);}
std::uint64_t mapping(const Tag&f,const std::uint64_t&v,std::size_t width){if(!width||(width&(width-1)))std::abort();return (f.rule->a*v+f.rule->b*width)%mod;}
void run(){
 std::mt19937 rng(14137);unsigned checks=0;
 for(int n:{1,2,3,7,8,9,31,32,33,65}){
  std::vector<std::uint64_t> ref(n);for(int i=0;i<n;++i)ref[i]=i+1;
  lazy_segtree<std::uint64_t,op,e,Tag,mapping,compose,id> t(ref);
  for(int step=0;step<60;++step){
   int l=rng()%n,r=l+1+rng()%(n-l);std::uint64_t a=rng()%5,b=rng()%11;
   t.apply(l,r,Tag(a,b));for(int i=l;i<r;++i)ref[i]=(a*ref[i]+b)%mod;
   for(int rep=0;rep<20;++rep){int x=rng()%(n+1),y=rng()%(n+1);if(x>y)std::swap(x,y);std::uint64_t want=0;for(int i=x;i<y;++i)want=(want+ref[i])%mod;if(t.prod(x,y)!=want)std::abort();++checks;}
   if(t.values()!=ref)std::abort();++checks;
   auto copy=t;const auto total=copy.all_prod();t.apply(0,n,Tag(2,3));for(auto&v:ref)v=(2*v+3)%mod;if(copy.all_prod()!=total)std::abort();++checks;
  }
 }
 if(live!=0)std::abort();
 std::printf("PASS owning nondefault F, noncommutative composition, physical-width mapping, copies: %u checks\n",checks);
}
}

int main() {
    CopyOnly::run();
    Noncommutative::run();
    OwningTag::run();
    std::puts("lazy_segment_tree_query_types_test: PASS");
}
