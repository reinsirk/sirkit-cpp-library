#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/eertree

// Library: String/Palindromic_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
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
#ifndef SKIP_RUNTIME_CHECKS
        if(byte<Base||static_cast<std::size_t>(byte-Base)>=Alphabet)throw std::invalid_argument("PalindromicTree: byte");
#else
        (void)byte;
#endif
    }
    void check_id(Index id)const{
#ifndef SKIP_RUNTIME_CHECKS
        if(id < -1 || id>static_cast<Index>(distinct_palindrome_count()))
            throw std::out_of_range("PalindromicTree: node");
#else
        (void)id;
#endif
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


// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif

#include <cstdint>
#include <string>
#include <vector>
int main(){std::string text;verify_in>>text;PalindromicTree<>tree;tree.reserve(text.size());std::vector<std::int32_t>suffixes;suffixes.reserve(text.size());for(unsigned char byte:text)suffixes.push_back(tree.append(byte));verify_out<<tree.distinct_palindrome_count()<<'\n';for(std::int32_t id=1;id<=static_cast<std::int32_t>(tree.distinct_palindrome_count());++id)verify_out<<tree.parent(id)<<' '<<tree.suffix_link(id)<<'\n';for(std::size_t i=0;i<suffixes.size();++i){if(i)verify_out<<' ';verify_out<<suffixes[i];}verify_out<<'\n';}
