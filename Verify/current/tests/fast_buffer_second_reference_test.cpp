// SPDX-License-Identifier: CC0-1.0
#ifndef FAST_BUFFER_SOURCE
#define FAST_BUFFER_SOURCE "../../../Fast/Source/Internal/Buffer.cpp"
#endif
#include FAST_BUFFER_SOURCE
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

template<class T> using Buffer = reinsirk::detail::FastBuffer<T>;
struct NoDefault {
    int value;
    NoDefault() = delete;
    explicit NoDefault(int v) : value(v) {}
};
struct MoveOnly {
    int value;
    explicit MoveOnly(int v) : value(v) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) noexcept : value(std::exchange(other.value,-1)) {}
    MoveOnly& operator=(MoveOnly&& other) noexcept { value=std::exchange(other.value,-1);return *this; }
};
struct CopyPreferred {
    static inline int copies=0, moves=0;
    int value;
    explicit CopyPreferred(int v):value(v){}
    CopyPreferred(const CopyPreferred& x):value(x.value){++copies;}
    CopyPreferred(CopyPreferred&& x) noexcept(false):value(std::exchange(x.value,-1)){++moves;}
};
struct alignas(128) Self {
    const Self* self;
    const int value;
    explicit Self(int v=0):self(this),value(v){}
    Self(const Self& x):self(this),value(x.value){}
    Self(Self&& x) noexcept:self(this),value(x.value){}
};
struct Pair {
    int first,second;
    Pair(const int& a,const int& b):first(a),second(b){}
    Pair(const Pair&)=default;
    Pair(Pair&& x)noexcept:first(std::exchange(x.first,-1)),second(std::exchange(x.second,-1)){}
};
int main(){
    Buffer<NoDefault> no_default;
    for(int i=0;i<80;++i)no_default.emplace_back(i);
    for(int i=0;i<80;++i)assert(no_default[static_cast<std::size_t>(i)].value==i);
    auto ndcopy=no_default;no_default.clear();assert(ndcopy.size()==80);
    no_default.assign(ndcopy.begin()+3,ndcopy.end()-2);assert(no_default.size()==75&&no_default.front().value==3);
    no_default.assign(no_default.begin()+1,no_default.end()-1);assert(no_default.size()==73&&no_default.front().value==4);
    Buffer<MoveOnly> move;
    for(int i=0;i<128;++i)move.emplace_back(i);
    move.emplace_back(std::move(move.front()));
    assert(move.front().value==-1&&move.back().value==0);
    for(int i=1;i<128;++i)assert(move[static_cast<std::size_t>(i)].value==i);
    Buffer<MoveOnly> moved(std::move(move));assert(move.empty());
    move=std::move(moved);assert(moved.empty());auto& same=move;move=std::move(same);assert(move.size()==129);
    move.clear();move.emplace_back(99);assert(move.front().value==99);
    Buffer<CopyPreferred> preferred;preferred.reserve(2);preferred.emplace_back(1);preferred.emplace_back(2);
    CopyPreferred::copies=CopyPreferred::moves=0;preferred.reserve(10);assert(CopyPreferred::copies==2&&CopyPreferred::moves==0);
    Buffer<Pair> pairs;pairs.reserve(1);pairs.emplace_back(11,19);
    pairs.emplace_back(pairs.front().first,pairs.front().second);
    assert(pairs[0].first==11&&pairs[0].second==19&&pairs[1].first==11&&pairs[1].second==19);
    for(int i=0;i<100;++i)pairs.emplace_back(pairs.front().first,pairs.back().second);
    for(auto& p:pairs)assert(p.first==11&&p.second==19);
    Buffer<std::string> strings;strings.reserve(1);strings.emplace_back(100,'q');
    strings.emplace_back(strings.front().data()+3,strings.front().size()-8);
    assert(strings.front()==std::string(100,'q')&&strings.back()==std::string(92,'q'));
    strings.resize(strings.capacity()+17,strings.front());
    for(std::size_t i=2;i<strings.size();++i)assert(strings[i]==std::string(100,'q'));
    std::istringstream input("2 4 6 8 10");Buffer<int> from_input{std::istream_iterator<int>(input),std::istream_iterator<int>()};
    assert(from_input.size()==5&&from_input.back()==10);
    Buffer<int> zero(100);for(int x:zero)assert(x==0);zero.resize(150);for(int x:zero)assert(x==0);
    for(int repeat=0;repeat<70;++repeat){
        Buffer<Self> aligned;aligned.reserve(4);
        for(int i=0;i<4;++i)aligned.emplace_back(i+repeat);
        for(int i=0;i<4;++i)assert(aligned[static_cast<std::size_t>(i)].self==&aligned[static_cast<std::size_t>(i)]);
        aligned.resize(20,aligned.front());
        assert(reinterpret_cast<std::uintptr_t>(aligned.data())%alignof(Self)==0);
        for(std::size_t i=0;i<aligned.size();++i)assert(aligned[i].self==&aligned[i]&&aligned[i].value==(i<4?static_cast<int>(i)+repeat:repeat));
        aligned.resize(1);aligned.resize(20,Self(41));
        for(std::size_t i=1;i<aligned.size();++i)assert(aligned[i].self==&aligned[i]&&aligned[i].value==41);
        aligned.clear();aligned.emplace_back(57);assert(aligned.front().self==aligned.data()&&aligned.front().value==57);
        auto copied=aligned;assert(copied.data()!=aligned.data()&&copied.front().self==copied.data());
        aligned.swap(copied);assert(aligned.front().self==aligned.data());
    }
    std::cout<<"fast buffer second normal-input/alias/lifetime/alignment checks passed\n";
}
