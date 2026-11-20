// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md
//
// API一覧
// [reinsirk::detail::UnreleasedVector<T>（Fast内部専用）]
// std::vector<T>を匿名union内へ構築し、外側の破棄時だけvectorの要素破棄・領域解放を省く。
// 成長・代入・erase・clear等は通常のvectorとして破棄・旧領域回収を行う。最終領域はプロセス終了まで累積する。
// 構築/iterator/access/capacity/modifierはstd::vector相当。型/サイズ/添字/確保成功を利用側が保証し、例外時の状態保証はFast APIに従う。
// コピーは独立。moveはnoexcept、自己move代入を除き移動元は空。Tの最後の破棄や資源回収を必要としない場合だけ使う。
// boolはstd::vector<bool>のproxyを使う。真のbool&/bool*が必要な既存Fast APIはFastBuffer<bool>を維持する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNRELEASED_VECTOR_DEFINED
#define SIRKIT_UNRELEASED_VECTOR_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T>
class UnreleasedVector {
    union {std::vector<T> values_;};
public:
    using value_type=T;
    using size_type=typename std::vector<T>::size_type;
    using difference_type=typename std::vector<T>::difference_type;
    using reference=typename std::vector<T>::reference;
    using const_reference=typename std::vector<T>::const_reference;
    using pointer=typename std::vector<T>::pointer;
    using const_pointer=typename std::vector<T>::const_pointer;
    using iterator=typename std::vector<T>::iterator;
    using const_iterator=typename std::vector<T>::const_iterator;
    using reverse_iterator=typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator=typename std::vector<T>::const_reverse_iterator;
    UnreleasedVector()noexcept:values_{}{}
    explicit UnreleasedVector(size_type n):values_(n){}
    UnreleasedVector(size_type n,const T&value):values_(n,value){}
    template<std::input_iterator It> UnreleasedVector(It first,It last):values_(first,last){}
    UnreleasedVector(std::initializer_list<T> values):values_(values){}
    UnreleasedVector(const std::vector<T>&values)requires std::is_copy_constructible_v<T>:values_(values){}
    UnreleasedVector(std::vector<T>&&values)noexcept:values_(std::move(values)){values.clear();}
    UnreleasedVector(const UnreleasedVector&other)requires std::is_copy_constructible_v<T>:values_(other.values_){}
    UnreleasedVector&operator=(const UnreleasedVector&other)requires(std::is_copy_constructible_v<T>&&std::is_copy_assignable_v<T>){if(this!=&other)values_=other.values_;return *this;}
    UnreleasedVector(UnreleasedVector&&other)noexcept:values_(std::move(other.values_)){other.values_.clear();}
    UnreleasedVector&operator=(UnreleasedVector&&other)noexcept{if(this!=&other){values_=std::move(other.values_);other.values_.clear();}return *this;}
    ~UnreleasedVector()noexcept{} // 意図的にactiveなvectorを破棄しない。
    UnreleasedVector&operator=(std::initializer_list<T> values){values_=values;return *this;}
    iterator begin()noexcept{return values_.begin();}
    const_iterator begin()const noexcept{return values_.begin();}
    const_iterator cbegin()const noexcept{return values_.cbegin();}
    iterator end()noexcept{return values_.end();}
    const_iterator end()const noexcept{return values_.end();}
    const_iterator cend()const noexcept{return values_.cend();}
    reverse_iterator rbegin()noexcept{return values_.rbegin();}
    const_reverse_iterator rbegin()const noexcept{return values_.rbegin();}
    const_reverse_iterator crbegin()const noexcept{return values_.crbegin();}
    reverse_iterator rend()noexcept{return values_.rend();}
    const_reverse_iterator rend()const noexcept{return values_.rend();}
    const_reverse_iterator crend()const noexcept{return values_.crend();}
    bool empty()const noexcept{return values_.empty();}
    size_type size()const noexcept{return values_.size();}
    size_type capacity()const noexcept{return values_.capacity();}
    size_type max_size()const noexcept{return values_.max_size();}
    T*data()noexcept requires(!std::is_same_v<T,bool>){return values_.data();}
    const T*data()const noexcept requires(!std::is_same_v<T,bool>){return values_.data();}
    reference operator[](size_type i)noexcept{return values_[i];}
    const_reference operator[](size_type i)const noexcept{return values_[i];}
    reference at(size_type i){return values_.at(i);}
    const_reference at(size_type i)const{return values_.at(i);}
    reference front()noexcept{return values_.front();}
    const_reference front()const noexcept{return values_.front();}
    reference back()noexcept{return values_.back();}
    const_reference back()const noexcept{return values_.back();}
    void reserve(size_type n){values_.reserve(n);}
    void shrink_to_fit(){values_.shrink_to_fit();}
    void clear()noexcept{values_.clear();}
    void resize(size_type n){values_.resize(n);}
    void resize(size_type n,const T&value){values_.resize(n,value);}
    template<class...Args>reference emplace_back(Args&&...args){return values_.emplace_back(std::forward<Args>(args)...);}
    void push_back(const T&value){values_.push_back(value);}
    void push_back(T&&value){values_.push_back(std::move(value));}
    void pop_back()noexcept{values_.pop_back();}
    void assign(size_type n,const T&value){values_.assign(n,value);}
    template<std::input_iterator It>void assign(It first,It last){values_.assign(first,last);}
    void assign(std::initializer_list<T> values){values_.assign(values);}
    iterator insert(const_iterator position,const T&value){return values_.insert(position,value);}
    iterator insert(const_iterator position,T&&value){return values_.insert(position,std::move(value));}
    iterator insert(const_iterator position,size_type n,const T&value){return values_.insert(position,n,value);}
    template<std::input_iterator It>iterator insert(const_iterator position,It first,It last){return values_.insert(position,first,last);}
    iterator insert(const_iterator position,std::initializer_list<T> values){return values_.insert(position,values);}
    template<class...Args>iterator emplace(const_iterator position,Args&&...args){return values_.emplace(position,std::forward<Args>(args)...);}
    iterator erase(const_iterator position){return values_.erase(position);}
    iterator erase(const_iterator first,const_iterator last){return values_.erase(first,last);}
    void swap(UnreleasedVector&other)noexcept{values_.swap(other.values_);}
    friend void swap(UnreleasedVector&a,UnreleasedVector&b)noexcept{a.swap(b);}
};
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
