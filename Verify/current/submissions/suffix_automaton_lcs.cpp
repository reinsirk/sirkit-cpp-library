#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/longest_common_substring

// Library: String/Suffix_Automaton.cpp
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

#include <string>
int main(){std::string first,second;verify_in>>first>>second;SuffixAutomaton<>automaton(first);auto match=automaton.longest_common_substring(second);verify_out<<match.first_start<<' '<<match.first_start+match.length<<' '<<match.second_start<<' '<<match.second_start+match.length<<'\n';}
