// SPDX-License-Identifier: CC0-1.0
// 参考（数学的構成・機能）: https://nyaannyaan.github.io/library/string/suffix-automaton.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/suffix_automaton.md
//
// API一覧
// [SuffixAutomaton<Alphabet=26,Base='a'>] 連続するbyte値[Base,Base+Alphabet)のSuffix Automaton。1<=Alphabet<=256、Base+Alphabet<=256。
// SuffixAutomaton()/SuffixAutomaton(string_view): 空/与えた文字列で構築。byte単位、Unicode文字単位ではない。
// append(uint8_t)/append(string_view): 末尾へ追加。固定Alphabetで償却O(Alphabet)/O(Alphabet*m)、1byte追加は最悪O(n+Alphabet)。
// 確保を先行し1byte追加は強い例外保証。文字列版の確保失敗では変更せず、途中の不正byteでは処理済み接頭辞を保持。
// contains(pattern): 部分文字列か。空はtrue。occurrences(pattern): 重なる出現回数、空はn+1。範囲外byteを含む照会は不一致。
// containsはO(m)。occurrencesはappend後の最初の一致照会でO(n+states+m)、以後O(m)。出現回数はconstな遅延キャッシュを使う。
// longest_common_substring(text): Match{first_start,second_start,length}。自分/引数側の開始位置と長さ、最長が複数なら引数で先に終わるもの。O(m)。
// size()/empty()/state_count()/distinct_substring_count(): 元の長さ/空か/保存状態数/空を除く相異なる部分文字列数。O(1)。空は状態を確保しない。
// reserve(text_capacity): その長さの最悪状態数を予約、必要ならO(Alphabet*states)。reset(text={}): 新しい文字列へ置換、強い例外保証。
// clear(): 空へ戻して全領域を解放、O(states+n+1)。swap: O(1)。コピーは独立、コピー代入は強い例外保証。
// moveはnoexcept・O(1)、移動元は空で再利用可能。領域はO(Alphabet*states+n+1)、states<=2*n+1。
// 入力byte検査はSKIP_RUNTIME_CHECKSで除く。照会の不一致判定と、int32_t状態番号/長さ・容量の保護は常に維持。
// キャッシュ構築の確保失敗は文字列・オートマトンを保つ。同じ個体への並行したキャッシュ照会は外部で同期する。

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#ifndef SIRKIT_SUFFIX_AUTOMATON_DEFINED
#define SIRKIT_SUFFIX_AUTOMATON_DEFINED
template<std::size_t Alphabet=26,std::uint16_t Base='a'>
class SuffixAutomaton {
    static_assert(Alphabet>0&&Alphabet<=256&&Base<256&&Alphabet<=256-Base);
    using Index=std::int32_t;
    struct State {
        std::array<Index,Alphabet> next;
        Index link=-1,length=0,first_end=0;
        bool terminal=false;
        State(){next.fill(-1);}
    };
    std::vector<State> states_;
    std::size_t length_=0;
    Index last_=0;
    std::uint64_t distinct_=0;
    mutable std::vector<std::uint64_t> counts_;
    mutable bool counts_valid_=false;
    static constexpr std::size_t index_limit=static_cast<std::size_t>(std::numeric_limits<Index>::max());
    void prepare_append(std::size_t count) {
        if(count==0)return;
        if(count>index_limit-length_)throw std::length_error("SuffixAutomaton: length");
        const auto limit=std::min(index_limit,states_.max_size());
        const std::size_t root=states_.empty()?1:0;
        if(states_.size()>limit || root>limit-states_.size() || count>(limit-states_.size()-root)/2)
            throw std::length_error("SuffixAutomaton: states");
        const auto required=states_.size()+root+2*count;
        if(states_.capacity()<required){const auto capacity=states_.capacity();states_.reserve(std::max(required,capacity>limit/2?limit:capacity*2));}
    }
    static void check_byte(std::uint8_t byte) {
#ifndef SKIP_RUNTIME_CHECKS
        if(byte<Base || static_cast<std::size_t>(byte-Base)>=Alphabet)throw std::invalid_argument("SuffixAutomaton: byte");
#else
        (void)byte;
#endif
    }
    void append_prepared(std::uint8_t byte) {
        const auto key=static_cast<std::size_t>(byte)-Base;
        if(states_.empty())states_.emplace_back();
        const Index current=static_cast<Index>(states_.size());
        states_.emplace_back();states_[current].length=static_cast<Index>(length_+1);states_[current].first_end=static_cast<Index>(length_);states_[current].terminal=true;
        Index p=last_;
        while(p!=-1&&states_[p].next[key]==-1){states_[p].next[key]=current;p=states_[p].link;}
        if(p==-1)states_[current].link=0;
        else {
            const Index q=states_[p].next[key];
            if(states_[p].length+1==states_[q].length)states_[current].link=q;
            else {
                const Index clone=static_cast<Index>(states_.size());
                states_.push_back(states_[q]);states_[clone].length=states_[p].length+1;states_[clone].terminal=false;
                while(p!=-1&&states_[p].next[key]==q){states_[p].next[key]=clone;p=states_[p].link;}
                states_[q].link=states_[current].link=clone;
            }
        }
        distinct_+=static_cast<std::uint64_t>(states_[current].length-states_[states_[current].link].length);
        last_=current;++length_;counts_valid_=false;
    }
    Index find(std::string_view text) const noexcept {
        Index current=0;
        for(unsigned char byte:text){
            if(byte<Base||static_cast<std::size_t>(byte-Base)>=Alphabet||states_.empty())return -1;
            current=states_[current].next[static_cast<std::size_t>(byte)-Base];if(current==-1)return -1;
        }
        return current;
    }
    void build_counts() const {
        if(counts_valid_)return;
        std::vector<std::uint64_t> counts(states_.size());
        std::vector<Index> order(states_.size());
        std::vector<std::uint32_t> buckets(length_+1);
        for(const auto&state:states_)++buckets[static_cast<std::size_t>(state.length)];
        for(std::size_t i=1;i<buckets.size();++i)buckets[i]+=buckets[i-1];
        for(std::size_t i=0;i<states_.size();++i){order[--buckets[static_cast<std::size_t>(states_[i].length)]]=static_cast<Index>(i);counts[i]=states_[i].terminal?1:0;}
        for(std::size_t i=order.size();i>0;--i){const Index v=order[i-1];if(states_[v].link!=-1)counts[states_[v].link]+=counts[v];}
        counts_.swap(counts);counts_valid_=true;
    }
public:
    struct Match {std::size_t first_start=0,second_start=0,length=0;};
    SuffixAutomaton()=default;
    explicit SuffixAutomaton(std::string_view text){append(text);}
    SuffixAutomaton(const SuffixAutomaton&)=default;
    SuffixAutomaton& operator=(const SuffixAutomaton&other){if(this!=&other){SuffixAutomaton next(other);swap(next);}return *this;}
    SuffixAutomaton(SuffixAutomaton&&other) noexcept
        :states_(std::move(other.states_)),length_(std::exchange(other.length_,0)),last_(std::exchange(other.last_,0)),distinct_(std::exchange(other.distinct_,0)),counts_(std::move(other.counts_)),counts_valid_(std::exchange(other.counts_valid_,false)){other.states_.clear();other.counts_.clear();}
    SuffixAutomaton&operator=(SuffixAutomaton&&other)noexcept{if(this!=&other){SuffixAutomaton next(std::move(other));swap(next);}return *this;}
    void swap(SuffixAutomaton&other)noexcept{using std::swap;states_.swap(other.states_);swap(length_,other.length_);swap(last_,other.last_);swap(distinct_,other.distinct_);counts_.swap(other.counts_);swap(counts_valid_,other.counts_valid_);}
    friend void swap(SuffixAutomaton&a,SuffixAutomaton&b)noexcept{a.swap(b);}
    std::size_t size()const noexcept{return length_;}
    bool empty()const noexcept{return length_==0;}
    std::size_t state_count()const noexcept{return states_.size();}
    std::uint64_t distinct_substring_count()const noexcept{return distinct_;}
    void reserve(std::size_t text_capacity){if(text_capacity<=length_)return;prepare_append(text_capacity-length_);}
    void append(std::uint8_t byte){check_byte(byte);prepare_append(1);append_prepared(byte);}
    void append(std::string_view text){prepare_append(text.size());for(unsigned char byte:text){check_byte(byte);append_prepared(byte);}}
    bool contains(std::string_view pattern)const noexcept{return find(pattern)!=-1;}
    std::uint64_t occurrences(std::string_view pattern)const {
        if(pattern.empty())return static_cast<std::uint64_t>(length_)+1;
        const auto state=find(pattern);if(state==-1)return 0;build_counts();return counts_[state];
    }
    Match longest_common_substring(std::string_view text)const noexcept {
        Match best;Index current=0;std::size_t matched=0;
        for(std::size_t i=0;i<text.size();++i){const auto byte=static_cast<unsigned char>(text[i]);
            if(states_.empty()||byte<Base||static_cast<std::size_t>(byte-Base)>=Alphabet){current=0;matched=0;continue;}
            const auto key=static_cast<std::size_t>(byte)-Base;
            while(current!=0&&states_[current].next[key]==-1){current=states_[current].link;matched=static_cast<std::size_t>(states_[current].length);}
            if(states_[current].next[key]!=-1){current=states_[current].next[key];++matched;}else matched=0;
            if(matched>best.length)best={static_cast<std::size_t>(states_[current].first_end)+1-matched,i+1-matched,matched};
        }
        return best;
    }
    void clear()noexcept{std::vector<State>{}.swap(states_);std::vector<std::uint64_t>{}.swap(counts_);length_=0;last_=0;distinct_=0;counts_valid_=false;}
    void reset(std::string_view text={}){SuffixAutomaton next(text);swap(next);}
};
#endif
