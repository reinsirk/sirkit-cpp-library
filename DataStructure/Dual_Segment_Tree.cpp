// SPDX-License-Identifier: CC0-1.0
// 参考（数学的構成・機能）: https://ei1333.github.io/library/structure/segment-tree/dual-segment-tree.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dual_segment_tree.md
//
// API一覧
// [DualSegmentTree<T,Action,Mapping,Composition>]
// DualSegmentTree(values, mapping, composition): vector/spanの値で構築。O(n+1)時間・領域。
// DualSegmentTree(n,value,mapping,composition): 同じ値n個で構築。O(n+1)。T/Actionはcopyable。
// mapping(f,value)は作用後の値、composition(new,old)はoldの後にnewを作用。設定はconst呼出可能・move_constructible。
// 作用は結合的に合成できること。Actionの単位元・比較・既定構築は不要。数値中間値の範囲は利用者の前提。
// apply(l,r,f): 半開区間[l,r)へ作用。空区間は何もしない。apply(p,f): 1点へ作用。O(log(n+1))。
// get(p)/operator[](p): 点の値をコピー。set(p,value): 点を置換。transform(p,fn): fn(旧値)で更新。O(log(n+1))。
// values(): 全点を伝播してvectorで返す。O(n+1)。取得も内部を更新するため非const。
// size()/empty()/usable(): 要素数/空か/例外後も操作可能か。O(1)。swap(other): O(1)。
// reset(values): 同じ設定で再構築。reset(n,value)も可。O(旧n+n+1)。clear(): 空へ戻して配列領域を解放、O(n+1)。
// 添字は0..n-1、区間は0<=l<=r<=n。SKIP_RUNTIME_CHECKSで入力検査を除く。内部の容量・添字表現保護は常に維持。
// コピーの値/遅延は独立、設定はconst共有。コピー代入/resetの確保・コピー失敗時は元を保つ。
// moveはnoexcept・O(1)、移動元は同じ設定の空として再利用できる。外部設定の状態は巻き戻さない。
// 操作中に値/作用/写像等が例外を送出したらusable()==false。論理状態は保証せず、reset/clear/代入で復旧するまで操作不可。
// 失敗後もT/Actionを安全に破棄できること。値/作用/写像から同じ木へ再入しない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_DUAL_SEGMENT_TREE_DEFINED
#define SIRKIT_DUAL_SEGMENT_TREE_DEFINED
template<class T, class Action, class Mapping, class Composition>
class DualSegmentTree {
    static_assert(std::copyable<T> && std::copyable<Action>);
    static_assert(std::move_constructible<Mapping> && std::move_constructible<Composition>);
    static_assert(std::is_invocable_r_v<T,const Mapping&,const Action&,const T&>);
    static_assert(std::is_invocable_r_v<Action,const Composition&,const Action&,const Action&>);
    struct Config {
        [[no_unique_address]] Mapping mapping;
        [[no_unique_address]] Composition composition;
        Config(Mapping map,Composition compose):mapping(std::move(map)),composition(std::move(compose)){}
    };
    std::shared_ptr<const Config> config_;
    std::vector<T> values_;
    std::vector<std::optional<Action>> lazy_;
    std::size_t base_=1;
    unsigned height_=0;
    bool failed_=false;

    template<class Values> void initialize(const Values& values) {
        const std::size_t n=values.size();
        if(n>values_.max_size()) throw std::length_error("DualSegmentTree: capacity");
        while(base_<n) {
            if(base_>std::numeric_limits<std::size_t>::max()/2 || base_>lazy_.max_size()/2)
                throw std::length_error("DualSegmentTree: capacity");
            base_*=2; ++height_;
        }
        if(base_>std::numeric_limits<std::size_t>::max()/2 || base_>lazy_.max_size())
            throw std::length_error("DualSegmentTree: capacity");
        values_.reserve(n);
        for(std::size_t i=0;i<n;++i) values_.push_back(values[i]);
        lazy_.resize(base_);
    }
    template<class Values> DualSegmentTree(const Values& values,std::shared_ptr<const Config> config)
        :config_(std::move(config)){initialize(values);}
    void check_point(std::size_t p) const {
#ifndef SKIP_RUNTIME_CHECKS
        if(p>=values_.size()) throw std::out_of_range("DualSegmentTree: point");
#else
        (void)p;
#endif
    }
    void check_range(std::size_t l,std::size_t r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if(l>r || r>values_.size()) throw std::out_of_range("DualSegmentTree: range");
#else
        (void)l; (void)r;
#endif
    }
    template<class Fn> decltype(auto) operate(Fn&& fn) {
        if(failed_) throw std::logic_error("DualSegmentTree: reset required");
        try { return std::forward<Fn>(fn)(); }
        catch(...) {failed_=true;throw;}
    }
    void apply_node(std::size_t p,const Action& f) {
        if(p<base_) {
            if(lazy_[p]) lazy_[p]=std::invoke(config_->composition,f,*lazy_[p]);
            else lazy_[p]=f;
        } else values_[p-base_]=std::invoke(config_->mapping,f,values_[p-base_]);
    }
    void push(std::size_t p) {
        if(!lazy_[p]) return;
        apply_node(p*2,*lazy_[p]); apply_node(p*2+1,*lazy_[p]);
        lazy_[p].reset();
    }
    void push_path(std::size_t p) {
        for(unsigned h=height_;h>0;--h) push(p>>h);
    }
public:
    explicit DualSegmentTree(const std::vector<T>& values,Mapping mapping,Composition composition)
        :config_(std::make_shared<const Config>(std::move(mapping),std::move(composition))){initialize(values);}
    explicit DualSegmentTree(std::span<const T> values,Mapping mapping,Composition composition)
        :config_(std::make_shared<const Config>(std::move(mapping),std::move(composition))){initialize(values);}
    DualSegmentTree(std::size_t n,const T& value,Mapping mapping,Composition composition)
        :DualSegmentTree(std::vector<T>(n,value),std::move(mapping),std::move(composition)){}
    DualSegmentTree(const DualSegmentTree&)=default;
    DualSegmentTree& operator=(const DualSegmentTree& other) {
        if(this!=&other) {DualSegmentTree next(other);swap(next);} return *this;
    }
    DualSegmentTree(DualSegmentTree&& other) noexcept
        :config_(other.config_),values_(std::move(other.values_)),lazy_(std::move(other.lazy_)),
         base_(std::exchange(other.base_,1)),height_(std::exchange(other.height_,0)),failed_(std::exchange(other.failed_,false)) {
        other.values_.clear();other.lazy_.clear();
    }
    DualSegmentTree& operator=(DualSegmentTree&& other) noexcept {
        if(this!=&other) {DualSegmentTree next(std::move(other));swap(next);} return *this;
    }
    void swap(DualSegmentTree& other) noexcept {
        using std::swap;swap(config_,other.config_);values_.swap(other.values_);lazy_.swap(other.lazy_);
        swap(base_,other.base_);swap(height_,other.height_);swap(failed_,other.failed_);
    }
    friend void swap(DualSegmentTree& a,DualSegmentTree& b) noexcept {a.swap(b);}
    std::size_t size() const noexcept{return values_.size();}
    bool empty() const noexcept{return values_.empty();}
    bool usable() const noexcept{return !failed_;}
    void clear() noexcept {
        std::vector<T>{}.swap(values_);std::vector<std::optional<Action>>{}.swap(lazy_);
        base_=1;height_=0;failed_=false;
    }
    void reset(const std::vector<T>& values) {DualSegmentTree next(values,config_);swap(next);}
    void reset(std::span<const T> values) {DualSegmentTree next(values,config_);swap(next);}
    void reset(std::size_t n,const T& value) {reset(std::vector<T>(n,value));}
    void apply(std::size_t l,std::size_t r,const Action& f) {
        check_range(l,r);
        operate([&]{
            if(l==r)return;
            l+=base_;r+=base_;
            // 境界の古い作用を先に降ろし、残る被覆節点には新しい作用を後から合成する。
            push_path(l);push_path(r-1);
            while(l<r) {if(l&1)apply_node(l++,f);if(r&1)apply_node(--r,f);l/=2;r/=2;}
        });
    }
    void apply(std::size_t p,const Action& f) {
        check_point(p);operate([&]{push_path(base_+p);values_[p]=std::invoke(config_->mapping,f,values_[p]);});
    }
    T get(std::size_t p) {
        check_point(p);return operate([&]{push_path(base_+p);return values_[p];});
    }
    T operator[](std::size_t p) {return get(p);}
    void set(std::size_t p,const T& value) {
        check_point(p);operate([&]{push_path(base_+p);values_[p]=value;});
    }
    template<class Fn> void transform(std::size_t p,Fn&& fn) {
        check_point(p);operate([&]{push_path(base_+p);values_[p]=std::invoke(std::forward<Fn>(fn),values_[p]);});
    }
    std::vector<T> values() {
        return operate([&]{for(std::size_t p=1;p<base_;++p)push(p);return values_;});
    }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
