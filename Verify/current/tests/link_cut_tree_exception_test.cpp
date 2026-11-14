// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <new>
static std::int64_t allocations_live=0,fail_after=-1;
static void allocation_check(){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;}
void* operator new(std::size_t bytes){allocation_check();if(void*p=std::malloc(bytes?bytes:1)){++allocations_live;return p;}throw std::bad_alloc();}
void operator delete(void*p)noexcept{if(p){--allocations_live;std::free(p);}}
void operator delete(void*p,std::size_t)noexcept{::operator delete(p);}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete[](void*p)noexcept{::operator delete(p);}
void operator delete[](void*p,std::size_t)noexcept{::operator delete(p);}
void* operator new(std::size_t bytes,std::align_val_t alignment){allocation_check();void*p=nullptr;if(posix_memalign(&p,static_cast<std::size_t>(alignment),bytes?bytes:1)==0){++allocations_live;return p;}throw std::bad_alloc();}
void* operator new[](std::size_t bytes,std::align_val_t alignment){return ::operator new(bytes,alignment);}
void operator delete(void*p,std::align_val_t)noexcept{::operator delete(p);}
void operator delete[](void*p,std::align_val_t)noexcept{::operator delete(p);}
void operator delete(void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
void operator delete[](void*p,std::size_t,std::align_val_t)noexcept{::operator delete(p);}
#ifndef LINK_CUT_SOURCE
#define LINK_CUT_SOURCE "../../../DataStructure/Link_Cut_Tree.cpp"
#endif
#include LINK_CUT_SOURCE
using I=std::int64_t;
struct Budget {int operation=-1,mapping=-1,composition=-1;};
static void step(int& value){if(value==0)throw std::runtime_error("injected operation failure");if(value>0)--value;}
struct Sum {
    std::shared_ptr<Budget> budget;
    I operator()(I a,I b)const{step(budget->operation);return a+b;}
};
struct Map {
    std::shared_ptr<Budget> budget;
    I operator()(I f,I value,std::size_t n)const{step(budget->mapping);return value+f*static_cast<I>(n);}
};
struct Composition {
    std::shared_ptr<Budget> budget;
    I operator()(I f,I g)const{step(budget->composition);return f+g;}
};
using Tree=LinkCutTree<I,Sum,I,Map,Composition>;
static void prepare(Tree& tree){for(int i=1;i<12;++i)assert(tree.link(i,i-1));}
static void verify(Tree& tree){assert(tree.usable()&&tree.size()==12);assert(tree.path_prod(0,11)==78);for(int i=0;i<12;++i)assert(tree.get(i)==i+1);}
static void check_poison(Tree& tree,const std::vector<I>& values){
    assert(!tree.usable());
    try{tree.connected(0,0);assert(false);}catch(const std::logic_error&){}
    auto copy=tree;assert(!copy.usable());copy.clear();assert(copy.usable()&&copy.empty());
    auto moved=std::move(tree);assert(tree.empty()&&tree.usable()&&!moved.usable());
    moved.reset(values);assert(moved.usable()&&moved.size()==values.size()&&!moved.connected(0,1));
    tree.reset(values);assert(tree.usable());
}
void operation_failures(){
    auto budget=std::make_shared<Budget>();std::vector<I> values(12);for(int i=0;i<12;++i)values[i]=i+1;
    Tree source(values,0,Sum{budget},Map{budget},Composition{budget});prepare(source);
    int failures=0;
    for(int kind=0;kind<3;++kind)for(int allowance=0;allowance<18;++allowance)for(int operation=0;operation<11;++operation){
        auto target=source;*budget=Budget{};
        if(kind==0)budget->operation=allowance;
        if(kind==1)budget->mapping=allowance;
        if(kind==2)budget->composition=allowance;
        bool failed=false;
        try{
            switch(operation){
            case 0:target.path_prod(0,11);break;
            case 1:target.cut(4,5);break;
            case 2:target.link(0,11);break;
            case 3:target.make_root(3);break;
            case 4:target.set(5,99);break;
            case 5:target.transform(6,[](I x){return x+13;});break;
            case 6:target.kth_on_path(0,11,4);break;
            case 7:target.lca(0,3,8);break;
            case 8:target.apply(0,11,7);target.apply(0,11,9);target.get(4);break;
            case 9:target.connected(0,11);break;
            case 10:target.apply(4,8);break;
            }
        }catch(const std::runtime_error&){failed=true;++failures;}
        *budget=Budget{};
        if(failed)check_poison(target,values);else assert(target.usable());
        verify(source);
    }
    assert(failures>100);
    std::printf("operation/map/composition injected failures=%d\n",failures);
    // 利用者の変換関数の例外も同じ無効化規則。
    auto target=source;
    try{target.transform(4,[](I)->I{throw std::runtime_error("transform");});assert(false);}catch(const std::runtime_error&){}
    check_poison(target,values);verify(source);
}
void allocation_failures(){
    auto budget=std::make_shared<Budget>();std::vector<I> values(12);for(int i=0;i<12;++i)values[i]=i+1;
    Tree source(values,0,Sum{budget},Map{budget},Composition{budget});prepare(source);
    const std::vector<I> next(40,3);int failures=0;
    for(int operation=0;operation<4;++operation){
        bool success=false;
        for(int allowance=0;allowance<15;++allowance){
            auto target=source;const auto before=allocations_live;fail_after=allowance;bool failed=false;
            try{
                if(operation==0)target.reset(next);
                if(operation==1)target=source;
                if(operation==2){Tree copy(source);assert(copy.size()==12);}
                if(operation==3){Tree created(next,0,Sum{budget},Map{budget},Composition{budget});assert(created.size()==40);}
            }catch(const std::bad_alloc&){failed=true;++failures;}
            fail_after=-1;
            if(failed){assert(allocations_live==before);verify(target);}
            else success=true;
            verify(source);if(success)break;
        }
        assert(success);
    }
    assert(failures>=9);
    // 型側の演算が確保しなければ、祖先スタックも含めて森操作では確保しない。
    fail_after=0;
    source.make_root(8);assert(source.connected(1,11));assert(source.cut(5,6));assert(source.link(5,6));
    source.set(3,7);source.transform(4,[](I x){return x+1;});assert(source.apply(0,11,2));source.apply(2,3);
    assert(source.path_size(0,11)==12&&source.kth_on_path(0,11,7)==7&&source.lca(0,4,8)==4);
    (void)source.path_prod(0,11);(void)source.get(3);fail_after=-1;
    std::printf("allocation injected failures=%d, no allocation during numeric operations OK\n",failures);
}
void string_operation_allocation_failures(){
    const std::vector<std::string> values(24,std::string(40,'x'));
    LinkCutTree<std::string> source(values,"");for(int i=1;i<24;++i)assert(source.link(i-1,i));
    int failures=0;
    for(int allowance=0;allowance<45;++allowance){
        const auto before=allocations_live;
        {
            auto target=source;fail_after=allowance;bool failed=false;
            try{target.path_prod(0,23);}catch(const std::bad_alloc&){failed=true;++failures;}
            fail_after=-1;
            if(failed){assert(!target.usable());try{target.get(0);assert(false);}catch(const std::logic_error&){}target.clear();assert(target.usable()&&target.empty());}
        }
        assert(allocations_live==before);
    }
    assert(failures>10&&source.path_prod(0,23)==std::string(24*40,'x'));
    std::printf("string operation allocation failures=%d\n",failures);
}
struct Tracked {
    static inline int live=0,copies=-1;
    I value;
    explicit Tracked(I x=0):value(x){++live;}
    Tracked(const Tracked& x):value(x.value){step(copies);++live;}
    Tracked(Tracked&& x):value(x.value){step(copies);++live;}
    Tracked& operator=(const Tracked& x){step(copies);value=x.value;return*this;}
    Tracked& operator=(Tracked&& x){step(copies);value=x.value;return*this;}
    ~Tracked(){--live;}
    friend void swap(Tracked&a,Tracked&b){step(copies);std::swap(a.value,b.value);}
};
struct TrackedSum {Tracked operator()(const Tracked&a,const Tracked&b)const{return Tracked(a.value+b.value);}};
void value_failures(){
    using ValueTree=LinkCutTree<Tracked,TrackedSum>;
    std::vector<Tracked> values;for(int i=0;i<8;++i)values.emplace_back(i+1);
    ValueTree source(values,Tracked(0));for(int i=1;i<8;++i)assert(source.link(i-1,i));
    int failures=0;
    for(int allowance=0;allowance<90;++allowance){
        auto target=source;Tracked::copies=allowance;bool failed=false;
        try{target.path_prod(0,7);}catch(const std::runtime_error&){failed=true;++failures;}
        Tracked::copies=-1;
        if(failed){assert(!target.usable());try{target.get(0);assert(false);}catch(const std::logic_error&){}target.reset(values);assert(target.usable());}
        assert(source.path_prod(0,7)->value==36);
    }
    for(int allowance=0;allowance<70;++allowance){
        auto target=source;Tracked::copies=allowance;bool failed=false;
        try{target=source;}catch(const std::runtime_error&){failed=true;++failures;}
        Tracked::copies=-1;
        assert(target.usable()&&target.path_prod(0,7)->value==36);
        if(!failed)break;
    }
    assert(failures>20);std::printf("tracked value injected failures=%d\n",failures);
}
struct ThrowAction {
    static inline int copies=-1,live=0;
    I add;
    explicit ThrowAction(I x=0):add(x){++live;}
    ThrowAction(const ThrowAction&other):add(other.add){step(copies);++live;}
    ThrowAction(ThrowAction&&other):add(other.add){step(copies);++live;}
    ThrowAction& operator=(const ThrowAction&other){step(copies);add=other.add;return*this;}
    ThrowAction& operator=(ThrowAction&&other){step(copies);add=other.add;return*this;}
    ~ThrowAction(){--live;}
};
struct ActionMapping {I operator()(const ThrowAction&f,I x,std::size_t n)const noexcept{return x+f.add*static_cast<I>(n);}};
struct ActionComposition {ThrowAction operator()(const ThrowAction&f,const ThrowAction&g)const{return ThrowAction(f.add+g.add);}};
void action_failures(){
    using ActionTree=LinkCutTree<I,std::plus<I>,ThrowAction,ActionMapping,ActionComposition>;
    std::vector<I> values(8);for(int i=0;i<8;++i)values[i]=i+1;
    ActionTree source(values,0);for(int i=1;i<8;++i)assert(source.link(i-1,i));
    assert(source.apply(0,7,ThrowAction(1)));int failures=0;
    for(int allowance=0;allowance<75;++allowance){
        auto target=source;ThrowAction action(3);ThrowAction::copies=allowance;bool failed=false;
        try{target.apply(0,7,action);target.get(3);target.apply(1,6,action);}catch(const std::runtime_error&){failed=true;++failures;}
        ThrowAction::copies=-1;
        if(failed){assert(!target.usable());try{target.path_prod(0,7);assert(false);}catch(const std::logic_error&){}target.clear();assert(target.usable());target.reset(values);}
        assert(source.path_prod(0,7)==44);
    }
    assert(failures>10);std::printf("action value injected failures=%d\n",failures);
}
struct alignas(64) Aligned {std::uint64_t value=0;};
struct AlignedSum {Aligned operator()(const Aligned&a,const Aligned&b)const noexcept{return {a.value+b.value};}};
void aligned_ownership(){
    LinkCutTree<Aligned,AlignedSum> tree(std::vector<Aligned>{{2},{4},{8}},Aligned{});
    assert(tree.link(2,0)&&tree.link(0,1)&&tree.path_prod(2,1)->value==14);
    auto copy=tree;auto moved=std::move(copy);assert(copy.empty()&&moved.get(1).value==4);
}
int main(){
    assert(allocations_live==0);
    {operation_failures();allocation_failures();string_operation_allocation_failures();value_failures();action_failures();aligned_ownership();}
    assert(Tracked::live==0&&ThrowAction::live==0&&allocations_live==0);
    std::printf("normal_live0=%" PRId64 " tracked_live0=%d\n",allocations_live,Tracked::live);
}
