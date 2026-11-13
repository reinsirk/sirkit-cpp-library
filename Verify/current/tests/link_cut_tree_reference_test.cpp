// SPDX-License-Identifier: CC0-1.0
#ifndef LINK_CUT_SOURCE
#define LINK_CUT_SOURCE "../../../DataStructure/Link_Cut_Tree.cpp"
#endif
#include LINK_CUT_SOURCE
#include LINK_CUT_SOURCE
#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <random>
#include <string>

struct Forest {
    int n;
    std::vector<std::vector<bool>> edge;
    explicit Forest(int vertices):n(vertices),edge(n,std::vector<bool>(n)){}
    std::vector<int> path(int first,int last) const {
        std::vector<int> parent(n,-1),queue{first};parent[first]=first;
        for(std::size_t i=0;i<queue.size()&&parent[last]==-1;++i)
            for(int v=0;v<n;++v)if(edge[queue[i]][v]&&parent[v]==-1){parent[v]=queue[i];queue.push_back(v);}
        if(parent[last]==-1)return{};
        std::vector<int> result;
        for(int v=last;;v=parent[v]){result.push_back(v);if(v==first)break;}
        std::reverse(result.begin(),result.end());return result;
    }
    bool link(int u,int v){if(!path(u,v).empty())return false;edge[u][v]=edge[v][u]=true;return true;}
    bool cut(int u,int v){if(!edge[u][v])return false;edge[u][v]=edge[v][u]=false;return true;}
    std::optional<int> lca(int root,int u,int v)const{
        const auto first=path(root,u),second=path(root,v);if(first.empty()||second.empty())return{};
        int result=root;
        for(std::size_t i=0;i<std::min(first.size(),second.size())&&first[i]==second[i];++i)result=first[i];
        return result;
    }
};
using I=std::int64_t;
struct AffineAction { I a,b; };
struct SumMapping { I operator()(const AffineAction& f,I value,std::size_t n)const{return f.a*value+f.b*static_cast<I>(n);} };
struct AffineComposition { AffineAction operator()(const AffineAction& f,const AffineAction& g)const{return {f.a*g.a,f.a*g.b+f.b};} };
using SumTree=LinkCutTree<I,std::plus<I>,AffineAction,SumMapping,AffineComposition>;

void sum_tests(){
    std::mt19937_64 rng(0x1c7f0123);
    for(int iteration=0;iteration<100;++iteration){
        const int n=1+static_cast<int>(rng()%33);
        std::vector<I> values(n);for(auto& value:values)value=static_cast<I>(rng()%101)-50;
        SumTree tree(values,0);Forest reference(n);
        for(int step=0;step<2500;++step){
            int u=static_cast<int>(rng()%n),v=static_cast<int>(rng()%n),r=static_cast<int>(rng()%n);
            const auto path=reference.path(u,v);
            switch(rng()%14){
            case 0:assert(tree.link(u,v)==reference.link(u,v));break;
            case 1:assert(tree.cut(u,v)==reference.cut(u,v));break;
            case 2:{const I x=static_cast<I>(rng()%201)-100;tree.set(u,x);values[u]=x;break;}
            case 3:{const I x=static_cast<I>(rng()%31)-15;tree.transform(u,[&](I old){return old+x;});values[u]+=x;break;}
            case 4:{I expected=0;for(int x:path)expected+=values[x];assert(tree.path_prod(u,v)==(path.empty()?std::nullopt:std::optional<I>(expected)));break;}
            case 5:{const auto size=path.size();assert(tree.path_size(u,v)==size);assert(tree.path_length(u,v)==(size==0?std::nullopt:std::optional<std::size_t>(size-1)));break;}
            case 6:{const auto k=static_cast<std::size_t>(rng()%(n+2));assert(tree.kth_on_path(u,v,k)==(k<path.size()?std::optional<int>(path[k]):std::nullopt));break;}
            case 7:tree.make_root(r);for(int x=0;x<n;++x)if(!reference.path(r,x).empty())assert(tree.find_root(x)==r);break;
            case 8:assert(tree.lca(r,u,v)==reference.lca(r,u,v));break;
            case 9:assert(tree.connected(u,v)==!path.empty());break;
            case 10:{const AffineAction f{static_cast<I>(rng()%3)-1,static_cast<I>(rng()%11)-5};assert(tree.apply(u,v,f)==!path.empty());for(int x:path)values[x]=f.a*values[x]+f.b;break;}
            case 11:{const AffineAction f{static_cast<I>(rng()%3)-1,static_cast<I>(rng()%11)-5};tree.apply(u,f);values[u]=f.a*values[u]+f.b;break;}
            case 12:{auto copy=tree;copy.set(u,values[u]+999);assert(tree.get(u)==values[u]);auto moved=std::move(copy);assert(copy.empty()&&copy.usable());copy.reset(values);assert(copy.get(u)==values[u]);for(int x=0;x<n;++x)assert(copy.path_size(u,x)==static_cast<std::size_t>(x==u));copy=std::move(moved);assert(moved.empty());auto& same=copy;copy=same;copy=std::move(same);break;}
            case 13:assert(tree.get(u)==values[u]);break;
            }
            assert(tree.usable()&&tree.size()==static_cast<std::size_t>(n));
            if(step%31==0){
                for(int x=0;x<n;++x)assert(tree.get(x)==values[x]);
                for(int check=0;check<5;++check){
                    const int a=static_cast<int>(rng()%n),b=static_cast<int>(rng()%n);const auto p=reference.path(a,b);I sum=0;for(int x:p)sum+=values[x];
                    assert(tree.path_prod(a,b)==(p.empty()?std::nullopt:std::optional<I>(sum)));
                    assert(tree.connected(a,b)==!p.empty());assert(tree.path_size(a,b)==p.size());
                    if(!p.empty()){assert(tree.kth_on_path(a,b,0)==a);assert(tree.kth_on_path(a,b,p.size()-1)==b);}
                }
            }
        }
        auto copy=tree;tree.clear();assert(tree.empty()&&tree.usable());swap(tree,copy);assert(tree.size()==static_cast<std::size_t>(n)&&copy.empty());
        tree.reset(std::span<const I>{});assert(tree.empty());
    }
}
struct Concat { std::string operator()(const std::string&a,const std::string&b)const{return a+b;} };
struct AssignMapping { std::string operator()(char x,const std::string&,std::size_t n)const{return std::string(n,x);} };
struct AssignComposition { char operator()(char newer,char)const{return newer;} };
using StringTree=LinkCutTree<std::string,Concat,char,AssignMapping,AssignComposition>;
void noncommutative_tests(){
    std::mt19937_64 rng(0xb1d1f7);
    for(int run=0;run<70;++run){
        int n=1+static_cast<int>(rng()%26);std::vector<std::string> value(n);
        for(int i=0;i<n;++i)value[i]=std::string(1,static_cast<char>('A'+i%26));
        StringTree tree(value,"");Forest reference(n);
        for(int step=0;step<1800;++step){
            int u=static_cast<int>(rng()%n),v=static_cast<int>(rng()%n);auto p=reference.path(u,v);
            switch(rng()%8){
            case 0:assert(tree.link(u,v)==reference.link(u,v));break;
            case 1:assert(tree.cut(u,v)==reference.cut(u,v));break;
            case 2:{char c=static_cast<char>('a'+rng()%26);assert(tree.apply(u,v,c)==!p.empty());for(int x:p)value[x]=std::string(1,c);break;}
            case 3:{std::string s(1,static_cast<char>('0'+rng()%10));tree.set(u,s);value[u]=s;break;}
            case 4:tree.make_root(u);break;
            case 5:{const int root=static_cast<int>(rng()%n);assert(tree.lca(root,u,v)==reference.lca(root,u,v));break;}
            case 6:{auto copy=tree;copy.apply(u,'!');assert(tree.get(u)==value[u]);break;}
            default:assert(tree.connected(u,v)==!p.empty());break;
            }
            p=reference.path(u,v);std::string expected;for(int x:p)expected+=value[x];
            assert(tree.path_prod(u,v)==(p.empty()?std::nullopt:std::optional<std::string>(expected)));
            std::reverse(expected.begin(),expected.end());assert(tree.path_prod(v,u)==(p.empty()?std::nullopt:std::optional<std::string>(expected)));
            if(step%41==0)for(int i=0;i<n;++i)assert(tree.get(i)==value[i]);
        }
    }
}
constexpr std::uint64_t mod=998244353;
struct Function { std::uint64_t a,b; friend bool operator==(const Function&,const Function&)=default; };
struct Compose { Function operator()(const Function& first,const Function& second)const noexcept{return {first.a*second.a%mod,(first.b*second.a+second.b)%mod};} };
void composite_tests(){
    std::mt19937_64 rng(0xa771c7);
    for(int run=0;run<35;++run){
        const int n=3+static_cast<int>(rng()%25);std::vector<Function> values(n);for(auto& f:values)f={rng()%mod,rng()%mod};
        LinkCutTree<Function,Compose> tree(values,Function{1,0});Forest ref(n);
        for(int step=0;step<1800;++step){
            const int u=static_cast<int>(rng()%n),v=static_cast<int>(rng()%n);
            switch(rng()%5){case 0:assert(tree.link(u,v)==ref.link(u,v));break;case 1:assert(tree.cut(u,v)==ref.cut(u,v));break;case 2:values[u]={rng()%mod,rng()%mod};tree.set(u,values[u]);break;case 3:tree.make_root(u);break;default:break;}
            const auto p=ref.path(u,v);const auto got=tree.path_prod(u,v);assert(got.has_value()==!p.empty());
            if(got)for(auto seed:{std::uint64_t{0},std::uint64_t{1},rng()%mod}){auto expected=seed;for(int x:p)expected=(expected*values[x].a+values[x].b)%mod;assert((seed*got->a+got->b)%mod==expected);}
        }
    }
}
struct BoolOr { bool operator()(bool a,bool b)const noexcept{return a||b;} };
struct NonDefault {
    I value;
    NonDefault()=delete;
    explicit NonDefault(I x):value(x){}
};
struct MoveOnlyModSum {
    std::unique_ptr<I> modulus;
    explicit MoveOnlyModSum(I p):modulus(std::make_unique<I>(p)){}
    MoveOnlyModSum(MoveOnlyModSum&&)=default;
    MoveOnlyModSum(const MoveOnlyModSum&)=delete;
    NonDefault operator()(const NonDefault&a,const NonDefault&b)const{return NonDefault((a.value+b.value)%*modulus);}
};
void configuration_ownership(){
    using ConfigTree=LinkCutTree<NonDefault,MoveOnlyModSum>;
    const std::vector<NonDefault> values{NonDefault(4),NonDefault(6)};
    ConfigTree first(values,NonDefault(0),MoveOnlyModSum(7));assert(first.link(1,0)&&first.path_prod(0,1)->value==3);
    ConfigTree second(values,NonDefault(0),MoveOnlyModSum(11));assert(second.link(0,1)&&second.path_prod(0,1)->value==10);
    swap(first,second);assert(first.path_prod(0,1)->value==10&&second.path_prod(0,1)->value==3);
    auto copy=first;auto moved=std::move(copy);assert(copy.empty());
    copy.reset(values);assert(copy.link(1,0)&&copy.path_prod(0,1)->value==10);
    second=moved;assert(second.path_prod(1,0)->value==10);
    first.clear();assert(first.empty());first.reset(std::vector<NonDefault>{NonDefault(1),NonDefault(10)});assert(first.link(0,1)&&first.path_prod(0,1)->value==0);
}
void boundaries(){
    LinkCutTree<> empty;assert(empty.empty()&&empty.usable());empty.clear();
    LinkCutTree<bool,BoolOr> bits(std::vector<bool>{false,true,false},false);assert(bits.link(2,0)&&bits.link(0,1));assert(bits.path_prod(2,1)==true);bits.set(1,false);assert(bits.path_prod(1,2)==false);
    LinkCutTree<> single(std::vector<I>{42});assert(single.get(0)==42&&!single.link(0,0)&&!single.cut(0,0));assert(single.path_prod(0,0)==42&&single.path_length(0,0)==0&&single.path_size(0,0)==1);assert(single.kth_on_path(0,0,0)==0&&!single.kth_on_path(0,0,1));assert(single.lca(0,0,0)==0);
#ifndef SKIP_RUNTIME_CHECKS
    try{empty.get(0);assert(false);}catch(const std::out_of_range&){}
    try{single.connected(-1,0);assert(false);}catch(const std::out_of_range&){}
    try{single.link(0,1);assert(false);}catch(const std::out_of_range&){}
    assert(single.usable()&&single.get(0)==42);
#endif
}
int main(){boundaries();configuration_ownership();sum_tests();noncommutative_tests();composite_tests();std::cout<<"link cut independent dynamic forest/sum/affine-action/noncommutative/composite OK\n";}
