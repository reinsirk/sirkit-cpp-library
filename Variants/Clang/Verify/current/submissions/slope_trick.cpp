#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc127/tasks/abc127_f

// Library: DataStructure/Slope_Trick.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Slope_Trick.cpp
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
int main(){
 int q{};verify_in>>q;SlopeTrick function;
 while(q--){int type{};verify_in>>type;if(type==1){std::int64_t a{},b{};verify_in>>a>>b;function.add_abs(a);function.add_constant(b);}else verify_out<<*function.argmin().lower<<' '<<function.min_value()<<'\n';}
}
