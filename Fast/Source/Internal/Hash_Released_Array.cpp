// SPDX-License-Identifier: CC0-1.0
// Fast内部用。明示的なrelease_storage以外では要素・領域を破棄しない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_FAST_HASH_RELEASED_ARRAY_DEFINED
#define SIRKIT_FAST_HASH_RELEASED_ARRAY_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T>class FastHashReleasedArray {
    T* data_=nullptr;
    std::size_t size_=0;
public:
    FastHashReleasedArray() noexcept=default;
    explicit FastHashReleasedArray(std::size_t n):data_(n?new T[n]:nullptr),size_(n){}
    FastHashReleasedArray(std::size_t n,const T& value):FastHashReleasedArray(n){if(n)std::fill_n(data_,n,value);}
    FastHashReleasedArray(const FastHashReleasedArray&)=delete;
    FastHashReleasedArray& operator=(const FastHashReleasedArray&)=delete;
    FastHashReleasedArray(FastHashReleasedArray&& other)noexcept:data_(std::exchange(other.data_,nullptr)),size_(std::exchange(other.size_,0)){}
    FastHashReleasedArray& operator=(FastHashReleasedArray&& other)noexcept{if(this!=&other){data_=std::exchange(other.data_,nullptr);size_=std::exchange(other.size_,0);}return *this;}
    ~FastHashReleasedArray()=default;
    static constexpr std::size_t max_size()noexcept{return static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max())/sizeof(T);}
    std::size_t size()const noexcept{return size_;}
    bool empty()const noexcept{return size_==0;}
    T*data()noexcept{return data_;}const T*data()const noexcept{return data_;}
    T*begin()noexcept{return data_;}const T*begin()const noexcept{return data_;}
    T*end()noexcept{return size_?data_+size_:data_;}const T*end()const noexcept{return size_?data_+size_:data_;}
    T&operator[](std::size_t i)noexcept{return data_[i];}const T&operator[](std::size_t i)const noexcept{return data_[i];}
    void release_storage()noexcept{delete[] data_;data_=nullptr;size_=0;}
    void swap(FastHashReleasedArray& other)noexcept{std::swap(data_,other.data_);std::swap(size_,other.size_);}
};
}
#endif


} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
