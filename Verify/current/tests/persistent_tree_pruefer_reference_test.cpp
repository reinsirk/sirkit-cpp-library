// SPDX-License-Identifier: CC0-1.0
#ifndef PERSISTENT_SEGMENT_TREE_SOURCE
#define PERSISTENT_SEGMENT_TREE_SOURCE "../../../DataStructure/Persistent_Segment_Tree.cpp"
#endif
#ifndef PRUEFER_CODE_SOURCE
#define PRUEFER_CODE_SOURCE "../../../Graph/Pruefer_Code.cpp"
#endif
#include PERSISTENT_SEGMENT_TREE_SOURCE
#include PRUEFER_CODE_SOURCE
#include PERSISTENT_SEGMENT_TREE_SOURCE
#include PRUEFER_CODE_SOURCE
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <tuple>

using I = std::int64_t;
std::mt19937_64 rng(0x2d921d731);
int random_int(int n) { return static_cast<int>(rng() % static_cast<unsigned>(n)); }
template<class F> void must_throw(F f) {
    bool caught = false;
    try { f(); } catch (const std::exception&) { caught = true; }
    assert(caught);
}

void persistent_sum_test() {
    using Tree = PersistentSegmentTree<I, std::plus<I>>;
    for (int n = 0; n <= 64; ++n) {
        std::vector<I> initial(static_cast<std::size_t>(n));
        for (auto& x : initial) x = random_int(10);
        Tree tree(initial, {}, 0);
        std::vector<Tree::Root> roots{tree.latest_root(), tree.empty_root()};
        std::vector<std::vector<I>> oracle{initial, std::vector<I>(static_cast<std::size_t>(n))};
        for (int repeat = 0; repeat < 550; ++repeat) {
            const auto version = static_cast<std::size_t>(random_int(static_cast<int>(roots.size())));
            const auto root = roots[version];
            int l = random_int(n+1), r = random_int(n+1); if (l > r) std::swap(l, r);
            I wanted = std::accumulate(oracle[version].begin()+l, oracle[version].begin()+r, I{0});
            assert(tree.prod(root, static_cast<std::size_t>(l), static_cast<std::size_t>(r)) == wanted);
            assert(tree.all_prod(root) == std::accumulate(oracle[version].begin(), oracle[version].end(), I{0}));
            I limit = random_int(80), sum = 0;
            int expected_right = l;
            while (expected_right < n && sum + oracle[version][static_cast<std::size_t>(expected_right)] <= limit)
                sum += oracle[version][static_cast<std::size_t>(expected_right++)];
            assert(tree.max_right(root, static_cast<std::size_t>(l), [limit](I x){return x <= limit;}) == static_cast<std::size_t>(expected_right));
            sum = 0; int expected_left = r;
            while (expected_left > 0 && sum + oracle[version][static_cast<std::size_t>(expected_left-1)] <= limit)
                sum += oracle[version][static_cast<std::size_t>(--expected_left)];
            assert(tree.min_left(root, static_cast<std::size_t>(r), [limit](I x){return x <= limit;}) == static_cast<std::size_t>(expected_left));
            if (n == 0) continue;
            const auto p = static_cast<std::size_t>(random_int(n));
            auto next = oracle[version];
            const auto before = tree.allocated_nodes();
            if (random_int(2)) {
                I replacement = random_int(10); next[p] = replacement;
                roots.push_back(tree.set(root, p, replacement));
            } else {
                I delta = random_int(3); next[p] += delta;
                roots.push_back(tree.transform(root, p, [delta](I x){return x+delta;}));
            }
            oracle.push_back(std::move(next));
            assert(tree.get(roots.back(), p) == oracle.back()[p]);
            assert(tree.allocated_nodes() - before <= 8);
            assert(tree.latest_root() == roots.back());
            if (repeat % 71 == 0) { roots.push_back(tree.build(initial)); oracle.push_back(initial); }
            if (repeat % 73 == 0) tree.reserve(tree.allocated_nodes()+37);
        }
        auto copied = tree;
        for (std::size_t v = 0; v < roots.size(); ++v)
            assert(copied.all_prod(copied.remap_from(tree, roots[v])) == tree.all_prod(roots[v]));
        if (n > 0) {
            auto original = tree.latest_root();
            [[maybe_unused]] auto later = tree.set(original, 0, 100);
            assert(copied.get(copied.latest_root(), 0) != 100);
#ifndef SKIP_RUNTIME_CHECKS
            must_throw([&]{ copied.remap_from(tree, later); });
            must_throw([&]{ copied.all_prod(original); });
#endif
        }
        auto saved = tree.latest_root();
        Tree moved(std::move(tree));
        assert(moved.all_prod(saved) == moved.all_prod(moved.latest_root()));
        tree = copied;
        Tree assigned(0, {}, 0); assigned = tree;
        assert(assigned.all_prod(assigned.remap_from(tree, tree.latest_root())) == tree.all_prod(tree.latest_root()));
        auto assigned_root = assigned.latest_root();
        swap(assigned, moved);
        assert(moved.all_prod(assigned_root) == tree.all_prod(tree.latest_root()));
        moved = std::move(assigned);
        auto stale = moved.latest_root(); moved.reset(7);
        assert(moved.allocated_nodes() == 0 && moved.size() == 7);
        assert(moved.all_prod(moved.latest_root()) == 0);
#ifndef SKIP_RUNTIME_CHECKS
        must_throw([&]{ moved.all_prod(stale); });
        must_throw([&]{ moved.get(moved.empty_root(), 7); });
        must_throw([&]{ moved.prod(moved.empty_root(), 3, 2); });
        must_throw([&]{ moved.build(std::vector<I>(6)); });
        must_throw([&]{ moved.max_right(moved.empty_root(), 0, [](I){return false;}); });
        must_throw([&]{ moved.min_left(moved.empty_root(), 7, [](I){return false;}); });
        must_throw([&]{ moved.all_prod(Tree::Root{}); });
#else
        static_cast<void>(stale);
#endif
        moved.clear(); assert(moved.empty() && moved.allocated_nodes() == 0);
    }
    Tree sparse(std::numeric_limits<std::size_t>::max(), {}, 0);
    auto root = sparse.set(sparse.empty_root(), sparse.size()-1, 7);
    assert(sparse.get(root, sparse.size()-1) == 7 && sparse.all_prod(root) == 7);
    assert(sparse.prod(root, sparse.size()-1, sparse.size()) == 7);
    assert(sparse.max_right(root, 0, [](I x){return x == 0;}) == sparse.size()-1);
    assert(sparse.min_left(root, sparse.size(), [](I x){return x == 0;}) == sparse.size());
}

struct Concat { std::string operator()(const std::string& a,const std::string& b) const { return a+b; } };
void noncommutative_test() {
    using Tree = PersistentSegmentTree<std::string,Concat>;
    for (int n=0;n<35;++n) {
        std::vector<std::string> values(static_cast<std::size_t>(n));
        for(auto& v:values) v=std::string(1,static_cast<char>('a'+random_int(4)));
        Tree tree(values,{},"");
        std::vector<Tree::Root> roots{tree.latest_root(),tree.empty_root()};
        std::vector<std::vector<std::string>> ref{values,std::vector<std::string>(static_cast<std::size_t>(n))};
        for(int k=0;k<350;++k) {
            auto v=static_cast<std::size_t>(random_int(static_cast<int>(roots.size())));
            int l=random_int(n+1),r=random_int(n+1);if(l>r)std::swap(l,r);
            std::string want;for(int i=l;i<r;++i)want+=ref[v][static_cast<std::size_t>(i)];
            assert(tree.prod(roots[v],static_cast<std::size_t>(l),static_cast<std::size_t>(r))==want);
            // 「ab」を含まないという単調な述語。連結の方向も確認する。
            auto pred=[](const std::string& s){return s.find("ab")==std::string::npos;};
            std::string s;int er=l;while(er<n && pred(s+ref[v][static_cast<std::size_t>(er)]))s+=ref[v][static_cast<std::size_t>(er++)];
            assert(tree.max_right(roots[v],static_cast<std::size_t>(l),pred)==static_cast<std::size_t>(er));
            s.clear();int el=r;while(el>0 && pred(ref[v][static_cast<std::size_t>(el-1)]+s))s=ref[v][static_cast<std::size_t>(--el)]+s;
            assert(tree.min_left(roots[v],static_cast<std::size_t>(r),pred)==static_cast<std::size_t>(el));
            if(n) {int p=random_int(n);auto next=ref[v];next[static_cast<std::size_t>(p)]=std::string(1,static_cast<char>('a'+random_int(4)));roots.push_back(tree.set(roots[v],static_cast<std::size_t>(p),next[static_cast<std::size_t>(p)]));ref.push_back(std::move(next));}
        }
    }
}

struct ThrowValue {
    int value=0;
    static inline int countdown=-1;
    static void check(){if(countdown==0)throw std::runtime_error("injected copy");if(countdown>0)--countdown;}
    ThrowValue()=default;ThrowValue(int x):value(x){}
    ThrowValue(const ThrowValue& x):value(x.value){check();}
    ThrowValue(ThrowValue&& x) noexcept(false):value(x.value){check();}
    ThrowValue& operator=(const ThrowValue& x){check();value=x.value;return *this;}
    ThrowValue& operator=(ThrowValue&& x) noexcept(false){check();value=x.value;return *this;}
};
struct ThrowOp {
    static inline int countdown=-1;
    ThrowValue operator()(const ThrowValue& a,const ThrowValue& b) const {
        if(countdown==0)throw std::runtime_error("injected op");
        if(countdown>0)--countdown;
        return ThrowValue(a.value+b.value);
    }
};
void throwing_test(){
    using Tree=PersistentSegmentTree<ThrowValue,ThrowOp>;
    for(int kind=0;kind<5;++kind) {
        int failures=0;
        for(int at=0;at<180;++at){
            ThrowValue::countdown=ThrowOp::countdown=-1;
            Tree tree(std::vector<ThrowValue>{1,2,3,4,5,6,7,8},{},ThrowValue{0});
            auto root=tree.latest_root(); auto second=tree.set(root,2,ThrowValue{30});
            auto before=tree.allocated_nodes();auto latest=tree.latest_root();
            std::vector<ThrowValue> values{8,7,6,5,4,3,2,1};
            if(kind<3)ThrowValue::countdown=at;else ThrowOp::countdown=at;
            bool fail=false;
            try {
                if(kind==0||kind==3)tree.set(root,3,ThrowValue{50});
                else if(kind==1||kind==4)tree.build(values);
                else tree.transform(second,1,[](const ThrowValue& x){if(x.value>=0)throw std::runtime_error("transform");return x;});
            }catch(const std::exception&){fail=true;++failures;}
            ThrowValue::countdown=ThrowOp::countdown=-1;
            assert(tree.all_prod(root).value==36&&tree.all_prod(second).value==63);
            if(fail){assert(tree.allocated_nodes()==before);assert(tree.latest_root()==latest);}
            else break;
        }
        assert(failures>0);
    }
}

std::vector<std::pair<int,int>> heap_decode(int n,const std::vector<int>& code){
    if(n==1)return{};
    std::vector<int>d(static_cast<std::size_t>(n),1);for(int x:code)++d[static_cast<std::size_t>(x)];
    std::priority_queue<int,std::vector<int>,std::greater<int>>q;for(int i=0;i<n;++i)if(d[static_cast<std::size_t>(i)]==1)q.push(i);
    std::vector<std::pair<int,int>>edges;
    for(int x:code){int leaf=q.top();q.pop();edges.emplace_back(leaf,x);if(--d[static_cast<std::size_t>(x)]==1)q.push(x);}
    int a=q.top();q.pop();edges.emplace_back(a,q.top());return edges;
}
std::vector<int> heap_encode(int n,const std::vector<std::pair<int,int>>& edges){
    std::vector<std::set<int>>g(static_cast<std::size_t>(n));for(auto[u,v]:edges){g[static_cast<std::size_t>(u)].insert(v);g[static_cast<std::size_t>(v)].insert(u);}
    std::priority_queue<int,std::vector<int>,std::greater<int>>q;for(int i=0;i<n;++i)if(g[static_cast<std::size_t>(i)].size()==1)q.push(i);
    std::vector<int>code;for(int i=0;i<n-2;++i){int u=q.top();q.pop();int v=*g[static_cast<std::size_t>(u)].begin();code.push_back(v);g[static_cast<std::size_t>(u)].clear();g[static_cast<std::size_t>(v)].erase(u);if(g[static_cast<std::size_t>(v)].size()==1)q.push(v);}return code;
}
void check_completed(int n,const std::vector<int>&deg,const std::vector<std::pair<int,int>>&old,const std::vector<std::pair<int,int>>&add){
    auto all=old;all.insert(all.end(),add.begin(),add.end());assert(all.size()==static_cast<std::size_t>(n-1));
    std::vector<int>d(static_cast<std::size_t>(n)),p(static_cast<std::size_t>(n));std::iota(p.begin(),p.end(),0);
    auto find=[&](int v){while(p[static_cast<std::size_t>(v)]!=v)v=p[static_cast<std::size_t>(v)];return v;};
    for(auto[u,v]:all){assert(u>=0&&u<n&&v>=0&&v<n);++d[static_cast<std::size_t>(u)];++d[static_cast<std::size_t>(v)];int a=find(u),b=find(v);assert(a!=b);p[static_cast<std::size_t>(a)]=b;}assert(d==deg);
}
void pruefer_one(int n,const std::vector<int>&code){
    auto edges=pruefer_decode(n,code);assert(edges==heap_decode(n,code));
    auto d=pruefer_degrees(n,code);auto rebuilt=tree_from_degrees(d);assert(rebuilt);check_completed(n,d,{},*rebuilt);
    assert(pruefer_encode(n,edges)==code);assert(heap_encode(n,edges)==code);
    for(auto&[u,v]:edges)if(random_int(2))std::swap(u,v);
    std::shuffle(edges.begin(),edges.end(),rng);
    assert(pruefer_encode(n,edges)==code);
    if(n>=2)assert(pruefer_decode(code)==heap_decode(n,code));
    std::vector<std::pair<int,int>>forest;for(auto e:edges)if(random_int(2))forest.push_back(e);
    auto completion=complete_tree_with_degrees(d,forest);assert(completion);check_completed(n,d,forest,*completion);
}
void pruefer_tests(){
    for(int n=1;n<=7;++n){std::size_t count=1;for(int i=0;i<n-2;++i)count*=static_cast<std::size_t>(n);for(std::size_t mask=0;mask<count;++mask){auto x=mask;std::vector<int>code(static_cast<std::size_t>(std::max(0,n-2)));for(auto&v:code){v=static_cast<int>(x%static_cast<std::size_t>(n));x/=static_cast<std::size_t>(n);}pruefer_one(n,code);}}
    for(int repeat=0;repeat<1200;++repeat){int n=random_int(400)+2;std::vector<int>code(static_cast<std::size_t>(n-2));for(auto&x:code)x=random_int(n);pruefer_one(n,code);}
    assert(!tree_from_degrees({}));assert(!tree_from_degrees(std::vector<int>{1}));assert(!tree_from_degrees(std::vector<int>{2,2,2}));assert(!tree_from_degrees(std::vector<int>{0,2}));assert(!tree_from_degrees(std::vector<int>{-1,3}));
    // 全ての小さい単純グラフを列挙し、既存辺を含む指定次数の木が存在するかを独立に判定。
    for(int n=2;n<=5;++n){std::vector<std::pair<int,int>>possible;for(int u=0;u<n;++u)for(int v=u+1;v<n;++v)possible.emplace_back(u,v);std::vector<std::pair<std::vector<int>,unsigned>>trees;std::size_t total=1;for(int i=0;i<n-2;++i)total*=static_cast<std::size_t>(n);for(std::size_t c=0;c<total;++c){auto x=c;std::vector<int>code(static_cast<std::size_t>(n-2));for(auto&v:code){v=static_cast<int>(x%static_cast<std::size_t>(n));x/=static_cast<std::size_t>(n);}auto e=heap_decode(n,code);std::vector<int>d(static_cast<std::size_t>(n));unsigned mask=0;for(auto[u,v]:e){++d[static_cast<std::size_t>(u)];++d[static_cast<std::size_t>(v)];if(u>v)std::swap(u,v);auto pos=std::find(possible.begin(),possible.end(),std::pair{u,v})-possible.begin();mask|=1u<<pos;}trees.emplace_back(d,mask);}
        for(int k=0;k<1500;++k){unsigned mask=static_cast<unsigned>(rng())&((1u<<possible.size())-1);std::vector<std::pair<int,int>>old;for(std::size_t e=0;e<possible.size();++e)if(mask>>e&1u)old.push_back(possible[e]);std::vector<int>d(static_cast<std::size_t>(n));for(auto&x:d)x=random_int(n+1);bool exists=false;for(const auto&[td,tm]:trees)if(td==d&&(tm&mask)==mask)exists=true;auto result=complete_tree_with_degrees(d,old);assert(result.has_value()==exists);if(result)check_completed(n,d,old,*result);}}
#ifndef SKIP_RUNTIME_CHECKS
    must_throw([]{pruefer_decode(0,{});});must_throw([]{pruefer_decode(4,std::vector<int>{1});});must_throw([]{pruefer_decode(3,std::vector<int>{-1});});must_throw([]{pruefer_encode(3,std::vector<std::pair<int,int>>{{0,1},{0,3}});});must_throw([]{pruefer_encode(4,std::vector<std::pair<int,int>>{{0,1},{1,2},{2,0}});});
    must_throw([]{pruefer_encode(4,std::vector<std::pair<int,int>>{{0,1},{0,1},{2,3}});});
    must_throw([]{complete_tree_with_degrees(std::vector<int>{1,1},std::vector<std::pair<int,int>>{{-1,0}});});
#endif
    // 長い鎖と星。スタックの深さはnに依存しない。
    for(bool star:{false,true}){int n=250000;std::vector<std::pair<int,int>>edges;edges.reserve(static_cast<std::size_t>(n-1));for(int v=1;v<n;++v)edges.emplace_back(star?0:v-1,v);auto code=pruefer_encode(n,edges);auto decoded=pruefer_decode(n,code);assert(pruefer_encode(n,decoded)==code);}
}
int main(){persistent_sum_test();noncommutative_test();throwing_test();pruefer_tests();std::cout<<"all persistent/pruefer checks passed\n";}
