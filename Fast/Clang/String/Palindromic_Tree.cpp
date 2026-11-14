// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Palindromic_Tree.cpp
// 参考（数学的構成・機能）: https://ei1333.github.io/library/string/palindromic-tree.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/palindromic_tree.md
//
// API一覧
// [PalindromicTree<Alphabet=26,Base='a'>] 連続byte値[Base,Base+Alphabet)の回文木。1<=Alphabet<=256、Base+Alphabet<=256。
// PalindromicTree()/PalindromicTree(string_view): 空/与えた文字列で構築。byte単位、Unicode文字単位ではない。
// append(uint8_t): 末尾へ追加し最長回文接尾辞の頂点番号を返す。固定Alphabetで償却O(Alphabet)、最悪O(n+Alphabet)。
// append(string_view): 順に追加、O(Alphabet*m)。自分のtext()の部分viewも可、その入力部分だけコピーする。
// 確保を先行し1byte追加は強い例外保証。文字列版の確保失敗では変更せず、途中の不正byteでは処理済み接頭辞を保持。
// size()/empty()/distinct_palindrome_count()/last_node()/longest_suffix_length(): 長さ/空か/空を除く回文数/最後の頂点/最長回文接尾辞長。O(1)。
// longest_palindrome(): Match{start,length}、最長回文の最初の出現。text(): 保存文字列のstring_view。O(1)。viewは更新/移動/交換まで。
// 頂点番号はODD=-1、EVEN=0、空でない回文は初出の順に1..distinct_palindrome_count()。
// length(id)/parent(id)/suffix_link(id)/first_position(id): 回文長/両端を除いた頂点/最長の真の回文接尾辞/最初の開始位置。O(1)。根の開始位置は0。
// find(pattern): 対応頂点のoptional、回文でない/不一致ならnullopt、空はEVEN。contains(pattern): 存在する回文か。O(m)。
// occurrences(pattern): 重なる出現回数、空はn+1。node_info(id): NodeInfo{id,length,parent,suffix_link,start,occurrences}。
// palindromes(): 空でない全頂点のNodeInfoを初出順のvectorで返す。初回/append後の出現回数集計はO(n+1)、以後find以外O(1)。列挙はO(n+1)。
// 範囲外byteを含む照会は不一致。rootの出現回数はODD=0、EVEN=n+1。
// reserve(text_capacity): その長さの最悪頂点数と文字列領域を予約。reset(text={}): 強い例外保証で置換。
// clear(): 空へ戻して全領域を解放、O(n+1)。swap: O(1)。コピーは独立、コピー代入は強い例外保証。
// moveはnoexcept・O(1)、移動元は空で再利用可能。領域O(Alphabet*n+1)、空は根も確保しない。
// SKIP_RUNTIME_CHECKSで追加byte/頂点番号の検査を除く。照会の不一致判定とint32_t番号/長さ・容量保護は常に維持。
// 出現回数はconstな遅延キャッシュ。集計の確保失敗は木/文字列を保つ。同じ個体の並行キャッシュ照会は外部で同期する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PALINDROMIC_TREE_DEFINED
#define SIRKIT_PALINDROMIC_TREE_DEFINED
template<std::size_t Alphabet=26,std::uint16_t Base='a'>
class PalindromicTree {
    static_assert(Alphabet>0&&Alphabet<=256&&Base<256&&Alphabet<=256-Base);
    using Index=std::int32_t;
    struct Node {
        std::array<Index,Alphabet> next;
        Index length=0,link=0,parent=0,start=0;
        std::uint32_t terminal=0;
        Node(){next.fill(-1);}
    };
    std::vector<Node> nodes_;
    std::string text_;
    Index last_=1,best_=1;
    mutable std::vector<std::uint64_t> counts_;
    mutable bool counts_valid_=false;
    static constexpr std::size_t index_limit=static_cast<std::size_t>(std::numeric_limits<Index>::max());
    static void check_byte(std::uint8_t byte){
        (void)byte;
    }
    void check_id(Index id)const{
        (void)id;
    }
    void prepare_append(std::size_t count){
        if(count==0)return;
        const auto limit=std::min(index_limit,nodes_.max_size());
        if(text_.size()>index_limit-2||count>index_limit-2-text_.size()||count>text_.max_size()-text_.size())throw std::length_error("PalindromicTree: length");
        const std::size_t roots=nodes_.empty()?2:0;
        if(nodes_.size()>limit||roots>limit-nodes_.size()||count>limit-nodes_.size()-roots)throw std::length_error("PalindromicTree: nodes");
        const auto required=nodes_.size()+roots+count;
        if(nodes_.capacity()<required){const auto capacity=nodes_.capacity();nodes_.reserve(std::max(required,capacity>limit/2?limit:capacity*2));}
        const auto text_required=text_.size()+count;
        if(text_.capacity()<text_required){const auto capacity=text_.capacity(),text_limit=std::min(index_limit-2,text_.max_size());text_.reserve(std::max(text_required,capacity>text_limit/2?text_limit:capacity*2));}
    }
    Index find_extendable(Index node,std::size_t position,std::uint8_t byte)const noexcept{
        for(;;){const auto distance=static_cast<std::size_t>(nodes_[node].length+1);if(position>=distance&&static_cast<unsigned char>(text_[position-distance])==byte)return node;node=nodes_[node].link;}
    }
    Index append_prepared(std::uint8_t byte){
        if(nodes_.empty()){nodes_.emplace_back();nodes_[0].length=-1;nodes_.emplace_back();}
        const auto position=text_.size();text_.push_back(static_cast<char>(byte));
        const auto key=static_cast<std::size_t>(byte)-Base;const Index previous=find_extendable(last_,position,byte);
        Index current=nodes_[previous].next[key];
        if(current==-1){
            current=static_cast<Index>(nodes_.size());nodes_.emplace_back();nodes_[current].length=nodes_[previous].length+2;nodes_[current].parent=previous;nodes_[current].start=static_cast<Index>(position+1-static_cast<std::size_t>(nodes_[current].length));
            if(nodes_[current].length==1)nodes_[current].link=1;
            else{const auto suffix=find_extendable(nodes_[previous].link,position,byte);nodes_[current].link=nodes_[suffix].next[key];}
            nodes_[previous].next[key]=current;
        }
        ++nodes_[current].terminal;last_=current;if(nodes_[current].length>nodes_[best_].length)best_=current;counts_valid_=false;return current-1;
    }
    void build_counts()const{
        if(counts_valid_)return;
        std::vector<std::uint64_t> counts(nodes_.size());for(std::size_t i=2;i<nodes_.size();++i)counts[i]=nodes_[i].terminal;
        // 真の回文接尾辞は短く、必ず先に出現済み。作成順の逆順で集計できる。
        for(std::size_t i=nodes_.size();i>2;--i)counts[nodes_[i-1].link]+=counts[i-1];
        counts_.swap(counts);counts_valid_=true;
    }
public:
    struct Match {std::size_t start=0,length=0;};
    struct NodeInfo {Index id,length,parent,suffix_link;std::size_t start;std::uint64_t occurrences;};
    PalindromicTree()=default;
    explicit PalindromicTree(std::string_view text){append(text);}
    PalindromicTree(const PalindromicTree&)=default;
    PalindromicTree&operator=(const PalindromicTree&other){if(this!=&other){PalindromicTree next(other);swap(next);}return *this;}
    PalindromicTree(PalindromicTree&&other)noexcept
        :nodes_(std::move(other.nodes_)),text_(std::move(other.text_)),last_(std::exchange(other.last_,1)),best_(std::exchange(other.best_,1)),counts_(std::move(other.counts_)),counts_valid_(std::exchange(other.counts_valid_,false)){other.nodes_.clear();other.text_.clear();other.counts_.clear();}
    PalindromicTree&operator=(PalindromicTree&&other)noexcept{if(this!=&other){PalindromicTree next(std::move(other));swap(next);}return *this;}
    void swap(PalindromicTree&other)noexcept{using std::swap;nodes_.swap(other.nodes_);text_.swap(other.text_);swap(last_,other.last_);swap(best_,other.best_);counts_.swap(other.counts_);swap(counts_valid_,other.counts_valid_);}
    friend void swap(PalindromicTree&a,PalindromicTree&b)noexcept{a.swap(b);}
    std::size_t size()const noexcept{return text_.size();}
    bool empty()const noexcept{return text_.empty();}
    std::size_t distinct_palindrome_count()const noexcept{return nodes_.empty()?0:nodes_.size()-2;}
    Index last_node()const noexcept{return last_-1;}
    std::size_t longest_suffix_length()const noexcept{return nodes_.empty()?0:static_cast<std::size_t>(nodes_[last_].length);}
    Match longest_palindrome()const noexcept{return nodes_.empty()?Match{}:Match{static_cast<std::size_t>(nodes_[best_].start),static_cast<std::size_t>(nodes_[best_].length)};}
    std::string_view text()const noexcept{return text_;}
    Index length(Index id)const{check_id(id);return id<=0?id:nodes_[static_cast<std::size_t>(id)+1].length;}
    Index parent(Index id)const{check_id(id);return id<=0?-1:nodes_[static_cast<std::size_t>(id)+1].parent-1;}
    Index suffix_link(Index id)const{check_id(id);return id<=0?-1:nodes_[static_cast<std::size_t>(id)+1].link-1;}
    std::size_t first_position(Index id)const{check_id(id);return id<=0?0:static_cast<std::size_t>(nodes_[static_cast<std::size_t>(id)+1].start);}
    void reserve(std::size_t text_capacity){if(text_capacity>size())prepare_append(text_capacity-size());}
    Index append(std::uint8_t byte){check_byte(byte);prepare_append(1);return append_prepared(byte);}
    void append(std::string_view more){
        if(more.empty())return;
        // text()から得たviewは、再確保より前にその部分だけ退避する。
        const auto less=std::less<const char*>{};
        if(!less(more.data(),text_.data())&&less(more.data(),text_.data()+text_.size())){const std::string copy(more);append(std::string_view(copy));return;}
        prepare_append(more.size());for(unsigned char byte:more){check_byte(byte);append_prepared(byte);}
    }
    std::optional<Index> find(std::string_view pattern)const noexcept{
        if(pattern.empty())return 0;if(nodes_.empty())return std::nullopt;
        Index current=(pattern.size()&1)?0:1;
        for(std::size_t i=pattern.size()/2+pattern.size()%2;i>0;--i){const auto byte=static_cast<unsigned char>(pattern[i-1]);
            if(pattern[i-1]!=pattern[pattern.size()-i]||byte<Base||static_cast<std::size_t>(byte-Base)>=Alphabet)return std::nullopt;
            current=nodes_[current].next[static_cast<std::size_t>(byte)-Base];if(current==-1)return std::nullopt;
        }
        return current-1;
    }
    bool contains(std::string_view pattern)const noexcept{return find(pattern).has_value();}
    std::uint64_t occurrences(std::string_view pattern)const{const auto id=find(pattern);if(!id)return 0;if(*id==0)return static_cast<std::uint64_t>(size())+1;build_counts();return counts_[static_cast<std::size_t>(*id)+1];}
    NodeInfo node_info(Index id)const{check_id(id);if(id<=0)return {id,id,-1,-1,0,id==-1?0:static_cast<std::uint64_t>(size())+1};build_counts();const auto&node=nodes_[static_cast<std::size_t>(id)+1];return {id,node.length,node.parent-1,node.link-1,static_cast<std::size_t>(node.start),counts_[static_cast<std::size_t>(id)+1]};}
    std::vector<NodeInfo> palindromes()const{build_counts();std::vector<NodeInfo> answer;answer.reserve(distinct_palindrome_count());for(std::size_t i=2;i<nodes_.size();++i){const auto&node=nodes_[i];answer.push_back({static_cast<Index>(i-1),node.length,node.parent-1,node.link-1,static_cast<std::size_t>(node.start),counts_[i]});}return answer;}
    void clear()noexcept{std::vector<Node>{}.swap(nodes_);std::string{}.swap(text_);std::vector<std::uint64_t>{}.swap(counts_);last_=best_=1;counts_valid_=false;}
    void reset(std::string_view text={}){PalindromicTree next(text);swap(next);}
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
