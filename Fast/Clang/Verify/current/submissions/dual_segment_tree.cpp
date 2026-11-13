#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/range_affine_point_get

// Library: DataStructure/Dual_Segment_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Dual_Segment_Tree.cpp
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
        (void)p;
    }
    void check_range(std::size_t l,std::size_t r) const {
        (void)l; (void)r;
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
#include <vector>
inline constexpr std::uint64_t modulus=998244353;
struct AffineAction {std::uint32_t a,b;};
struct AffineMapping {std::uint32_t operator()(const AffineAction&f,std::uint32_t x)const noexcept{return static_cast<std::uint32_t>((std::uint64_t{f.a}*x+f.b)%modulus);}};
struct AffineComposition {AffineAction operator()(const AffineAction&f,const AffineAction&g)const noexcept{return {static_cast<std::uint32_t>(std::uint64_t{f.a}*g.a%modulus),static_cast<std::uint32_t>((std::uint64_t{f.a}*g.b+f.b)%modulus)};}};
int main(){
 int n{},q{};verify_in>>n>>q;std::vector<std::uint32_t> values(n);for(auto&x:values)verify_in>>x;
 DualSegmentTree<std::uint32_t,AffineAction,AffineMapping,AffineComposition> tree(values,{},{});
 while(q--){int type{},l{};verify_in>>type>>l;if(type==0){int r{};AffineAction f{};verify_in>>r>>f.a>>f.b;tree.apply(l,r,f);}else verify_out<<tree.get(l)<<'\n';}
}
