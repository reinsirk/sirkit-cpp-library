// SPDX-License-Identifier: CC0-1.0
// 参考（数学的構成・機能）: https://nyaannyaan.github.io/library/data-structure/slope-trick.hpp.html
// 参考（数学的構成・機能）: https://ei1333.github.io/library/structure/others/slope-trick.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/slope_trick.md
//
// API一覧
// [SlopeTrick] 整数位置の折点と整数傾きを持ち、最小値が有限な区分線形凸関数。初期値f(x)=0。
// add_left_hinge(a)/add_right_hinge(a): max(a-x,0)/max(x-a,0)を加える。add_abs(a): |x-a|を加える。
// 各追加は償却O(log(k+1))、折点数k。単位傾きの折点を1個/絶対値は2個追加。
// add_constant(c): f+=c。min_value(): 最小値。O(1)。
// argmin(): Argmin{optional<int64_t> lower,upper}。最小点の閉区間、欠けた端はその方向の無限大。O(1)。
// shift(a): f(x)=旧f(x-a)。shift(a,b): f(x)=min_{x-b<=y<=x-a}旧f(y)、a<=b。O(1)。
// clear_right(): f(x)=min_{y<=x}旧f(y)、clear_left(): f(x)=min_{y>=x}旧f(y)。除いた側の領域を解放、O(k+1)。
// eval(x): f(x)を返す、関数を壊さずO(k+1)。merge(other): f+=other、otherを保持。自己merge可。
// mergeは強い例外保証のためコピーを用い、O(k+m*log(k+m+1))時間・O(k+m+1)追加領域。
// size()/empty(): 折点数/定数関数か。O(1)。clear(): f=0へ戻して領域を解放、O(k+1)。swap: O(1)。
// reserve(left,right): 各側の格納容量を予約。現在の折点数未満は変化なし。確保を伴うとO(k+1)。
// コピーは独立、コピー代入は強い例外保証。moveはnoexcept・O(1)、移動元はf=0。
// 座標・offset・保存キー(座標-offset)・演算する差/和はint64_t範囲。範囲外の演算はoverflow_error。
// shiftは全折点を走査せずoffsetだけを変更する。実座標が範囲外になった折点は、参照時に例外となる場合がある。
// 全操作は確保/整数範囲の失敗時に関数を保つ（容量の変化は可）。容量・整数範囲保護はSKIPでも維持。
// SKIP_RUNTIME_CHECKSはshift(a,b)のa<=b検査だけを除く。SIRKIT_FORCE_STANDARD_ARITHMETICで標準整数演算へ切替。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_SLOPE_TRICK_DEFINED
#define SIRKIT_SLOPE_TRICK_DEFINED
class SlopeTrick {
public:
    struct Argmin {std::optional<std::int64_t> lower,upper;};
private:
    std::vector<std::int64_t> left_,right_;
    std::int64_t left_shift_=0,right_shift_=0,minimum_=0;
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
    __extension__ using Wide = __int128;
#endif
    static std::int64_t add(std::int64_t a,std::int64_t b) {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        const Wide result=static_cast<Wide>(a)+b;
        if(result<std::numeric_limits<std::int64_t>::min() || result>std::numeric_limits<std::int64_t>::max())
            throw std::overflow_error("SlopeTrick: addition");
        return static_cast<std::int64_t>(result);
#else
        if((b>0&&a>std::numeric_limits<std::int64_t>::max()-b) || (b<0&&a<std::numeric_limits<std::int64_t>::min()-b))
            throw std::overflow_error("SlopeTrick: addition");
        return a+b;
#endif
    }
    static std::int64_t subtract(std::int64_t a,std::int64_t b) {
#if defined(__SIZEOF_INT128__) && !defined(SIRKIT_FORCE_STANDARD_ARITHMETIC)
        const Wide result=static_cast<Wide>(a)-b;
        if(result<std::numeric_limits<std::int64_t>::min() || result>std::numeric_limits<std::int64_t>::max())
            throw std::overflow_error("SlopeTrick: subtraction");
        return static_cast<std::int64_t>(result);
#else
        if((b>0&&a<std::numeric_limits<std::int64_t>::min()+b) || (b<0&&a>std::numeric_limits<std::int64_t>::max()+b))
            throw std::overflow_error("SlopeTrick: subtraction");
        return a-b;
#endif
    }
    static void grow(std::vector<std::int64_t>& heap) {
        if(heap.size()==heap.max_size()) throw std::length_error("SlopeTrick: capacity");
        if(heap.size()<heap.capacity())return;
        const auto capacity=heap.capacity();
        heap.reserve(capacity>heap.max_size()/2?heap.max_size():std::max<std::size_t>(1,capacity*2));
    }
    void push_left(std::int64_t key) {left_.push_back(key);std::push_heap(left_.begin(),left_.end());}
    void push_right(std::int64_t key) {right_.push_back(key);std::push_heap(right_.begin(),right_.end(),std::greater<>{});}
    void pop_left() {std::pop_heap(left_.begin(),left_.end());left_.pop_back();}
    void pop_right() {std::pop_heap(right_.begin(),right_.end(),std::greater<>{});right_.pop_back();}
    std::int64_t top_left() const {return add(left_.front(),left_shift_);}
    std::int64_t top_right() const {return add(right_.front(),right_shift_);}
public:
    SlopeTrick()=default;
    SlopeTrick(const SlopeTrick&)=default;
    SlopeTrick& operator=(const SlopeTrick& other) {if(this!=&other){SlopeTrick next(other);swap(next);}return *this;}
    SlopeTrick(SlopeTrick&& other) noexcept
        :left_(std::move(other.left_)),right_(std::move(other.right_)),
         left_shift_(std::exchange(other.left_shift_,0)),right_shift_(std::exchange(other.right_shift_,0)),minimum_(std::exchange(other.minimum_,0)) {
        other.left_.clear();other.right_.clear();
    }
    SlopeTrick& operator=(SlopeTrick&& other) noexcept {if(this!=&other){SlopeTrick next(std::move(other));swap(next);}return *this;}
    void swap(SlopeTrick& other) noexcept {
        using std::swap;left_.swap(other.left_);right_.swap(other.right_);swap(left_shift_,other.left_shift_);swap(right_shift_,other.right_shift_);swap(minimum_,other.minimum_);
    }
    friend void swap(SlopeTrick& a,SlopeTrick& b) noexcept {a.swap(b);}
    std::size_t size() const noexcept{return left_.size()+right_.size();}
    bool empty() const noexcept{return left_.empty()&&right_.empty();}
    std::int64_t min_value() const noexcept{return minimum_;}
    Argmin argmin() const {
        return {left_.empty()?std::nullopt:std::optional<std::int64_t>(top_left()),right_.empty()?std::nullopt:std::optional<std::int64_t>(top_right())};
    }
    void reserve(std::size_t left,std::size_t right) {left_.reserve(left);right_.reserve(right);}
    void add_constant(std::int64_t value) {minimum_=add(minimum_,value);}
    void add_left_hinge(std::int64_t a) {
        if(!right_.empty()&&a>top_right()) {
            const auto r=top_right(),next_min=add(minimum_,subtract(a,r));
            const auto lkey=subtract(r,left_shift_),rkey=subtract(a,right_shift_);
            grow(left_); // 以後の整数ヒープ操作は確保も例外もない。
            pop_right();push_right(rkey);push_left(lkey);minimum_=next_min;
        }else{const auto key=subtract(a,left_shift_);grow(left_);push_left(key);}
    }
    void add_right_hinge(std::int64_t a) {
        if(!left_.empty()&&a<top_left()) {
            const auto l=top_left(),next_min=add(minimum_,subtract(l,a));
            const auto lkey=subtract(a,left_shift_),rkey=subtract(l,right_shift_);
            grow(right_);
            pop_left();push_left(lkey);push_right(rkey);minimum_=next_min;
        }else{const auto key=subtract(a,right_shift_);grow(right_);push_right(key);}
    }
    void add_abs(std::int64_t a) {
        if(!left_.empty()&&a<top_left()) {
            const auto l=top_left(),next_min=add(minimum_,subtract(l,a));
            const auto lkey=subtract(a,left_shift_),rkey=subtract(l,right_shift_);
            grow(left_);grow(right_);
            pop_left();push_left(lkey);push_left(lkey);push_right(rkey);minimum_=next_min;
        }else if(!right_.empty()&&a>top_right()) {
            const auto r=top_right(),next_min=add(minimum_,subtract(a,r));
            const auto lkey=subtract(r,left_shift_),rkey=subtract(a,right_shift_);
            grow(left_);grow(right_);
            pop_right();push_right(rkey);push_right(rkey);push_left(lkey);minimum_=next_min;
        }else{
            const auto lkey=subtract(a,left_shift_),rkey=subtract(a,right_shift_);
            grow(left_);grow(right_);push_left(lkey);push_right(rkey);
        }
    }
    void shift(std::int64_t a) {shift(a,a);}
    void shift(std::int64_t a,std::int64_t b) {
#ifndef SKIP_RUNTIME_CHECKS
        if(a>b)throw std::invalid_argument("SlopeTrick: shift range");
#endif
        const auto l=left_.empty()?0:add(left_shift_,a),r=right_.empty()?0:add(right_shift_,b);
        left_shift_=l;right_shift_=r;
    }
    void clear_right() noexcept {std::vector<std::int64_t>{}.swap(right_);right_shift_=0;}
    void clear_left() noexcept {std::vector<std::int64_t>{}.swap(left_);left_shift_=0;}
    void clear() noexcept {clear_left();clear_right();minimum_=0;}
    std::int64_t eval(std::int64_t x) const {
        auto result=minimum_;
        for(auto key:left_){const auto a=add(key,left_shift_);if(a>x)result=add(result,subtract(a,x));}
        for(auto key:right_){const auto a=add(key,right_shift_);if(a<x)result=add(result,subtract(x,a));}
        return result;
    }
    void merge(const SlopeTrick& other) {
        SlopeTrick next(*this);next.add_constant(other.minimum_);
        for(auto key:other.left_)next.add_left_hinge(add(key,other.left_shift_));
        for(auto key:other.right_)next.add_right_hinge(add(key,other.right_shift_));
        swap(next);
    }
};
#endif
