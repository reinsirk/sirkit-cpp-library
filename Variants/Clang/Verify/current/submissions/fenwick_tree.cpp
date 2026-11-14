#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/point_add_range_sum

// Library: DataStructure/Fenwick_Tree.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Fenwick_Tree.cpp
// 参考（データ構造・検証問題）: https://nyaannyaan.github.io/library/data-structure/binary-indexed-tree.hpp
// 参考（公開API）: https://atcoder.github.io/ac-library/production/document_ja/fenwicktree.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fenwick_tree.md
//
// API一覧
// [fenwick_tree<T>]
// fenwick_tree()/fenwick_tree(int n)/fenwick_tree(vector<T>): 空列/n要素の0/列から構築。O(n+1)時間・領域。
// add(int p,T x): 点加算。sum(int l,int r): 半開区間[l,r)の和をT値で返す。O(log(n+1))、空区間はT{}。
// 点添字は0<=p<size()、区間は0<=l<=r<=size()。
// ACL形式の公開APIをsirkit内に定義する。bool以外の整数は内部を符号なしにして型幅のmod 2^bitで計算する。
// その他のTはT{}が単位元の可換加法群とし、加減算・コピーをO(1)とする。要素数はintの範囲内。
// size()/empty(): 要素数/空か、O(1)。get(int p)/set(int p,T value): 点取得/代入、O(log(n+1))。
// lower_bound(target)/upper_bound(target): sum(0,i+1)>=target/>targetとなる最小int添字、なければsize()。O(log(n+1))。
// 境界探索は全要素が非負で、途中の和がTに収まり、加算と順序が整合すること。全要素の検査はしない。
// values(): 列を復元して返す。O(n+1)時間・追加O(n)。reset(int n)/reset(): n要素/現在の長さを0にする。O(旧n+n+1)。
// clear()/swap(other): 空にして容量保持/交換。O(旧n+1)/O(1)。コピーは独立、移動元は空として使える。
// 添字・区間は既定で検査しSKIP_RUNTIME_CHECKSで除く。容量・添字のオーバーフロー対策は残す。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
#define SIRKIT_HLD_FENWICK_ACCESS_FORWARD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ { template<class Range> struct HldFenwickAccess; }
#endif

#ifndef SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
#define SIRKIT_FENWICK_STORAGE_TYPE_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template<class T, bool = std::is_integral_v<T> && !std::is_same_v<T, bool>>
struct FenwickStorage { using type = T; };
template<class T>
struct FenwickStorage<T, true> { using type = std::make_unsigned_t<T>; };
#if defined(__SIZEOF_INT128__)
template<> struct FenwickStorage<__int128> { using type = unsigned __int128; };
template<> struct FenwickStorage<unsigned __int128> { using type = unsigned __int128; };
#endif
}
#endif

#ifndef SIRKIT_FENWICK_TREE_DEFINED
#define SIRKIT_FENWICK_TREE_DEFINED
template<class T>
class fenwick_tree {
    // HLD所有時の読取り専用接続。通常のFenwick操作には使わない。
    using HldFenwickTag = void;
    template<class> friend struct /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::HldFenwickAccess;
    using Storage = typename /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::FenwickStorage<T>::type;
    std::vector<Storage> tree_;
    static decltype(auto) to_storage(const T& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<Storage>(value);
    }
    static decltype(auto) from_storage(Storage& value) {
        if constexpr (std::is_same_v<T, Storage>) return (value);
        else return static_cast<T>(value);
    }
    static std::size_t checked_size(int n) {
#ifndef SKIP_RUNTIME_CHECKS
        if (n < 0) throw std::invalid_argument("fenwick_tree: negative size");
#endif
        return static_cast<std::size_t>(n);
    }
    static std::size_t storage_size(std::size_t n) {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max())
            || n + 1 > std::vector<Storage>{}.max_size())
            throw std::length_error("fenwick_tree: size");
        return n == 0 ? 0 : n + 1;
    }
    static std::size_t lowbit(std::size_t x) noexcept { return x & (~x + 1); }
    void check_index(int i) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (i < 0 || i >= size()) throw std::out_of_range("fenwick_tree: index");
#else
        (void)i;
#endif
    }
    void check_range(int l, int r) const {
#ifndef SKIP_RUNTIME_CHECKS
        if (l < 0 || l > r || r > size()) throw std::out_of_range("fenwick_tree: range");
#else
        (void)l; (void)r;
#endif
    }
    Storage prefix_unchecked(std::size_t r) const {
        Storage result{};
        for (; r; r -= lowbit(r)) result += tree_[r];
        return result;
    }
    Storage get_unchecked(std::size_t i) const {
        auto node = i + 1;
        Storage result = tree_[node];
        const auto begin = node - lowbit(node);
        for (--node; node > begin; node -= lowbit(node)) result -= tree_[node];
        return result;
    }
    void add_unchecked(std::size_t i, const Storage& delta) {
        const auto n = static_cast<std::size_t>(size());
        for (++i; i <= n;) {
            tree_[i] += delta;
            const auto step = lowbit(i);
            if (step > n - i) break;
            i += step;
        }
    }
public:
    fenwick_tree() = default;
    explicit fenwick_tree(int n) : tree_(storage_size(checked_size(n)), Storage{}) {}
    template<class Values> requires std::is_same_v<Values, std::vector<T>>
    explicit fenwick_tree(const Values& values) : tree_(storage_size(values.size()), Storage{}) {
        const auto n = values.size();
        for (std::size_t i = 1; i <= n; ++i) {
            tree_[i] += to_storage(values[i - 1]);
            const auto step = lowbit(i);
            if (step <= n - i) tree_[i + step] += tree_[i];
        }
    }
    fenwick_tree(const fenwick_tree&) = default;
    fenwick_tree& operator=(const fenwick_tree&) = default;
    fenwick_tree(fenwick_tree&& other) noexcept : tree_(std::move(other.tree_)) { other.tree_.clear(); }
    fenwick_tree& operator=(fenwick_tree&& other) noexcept {
        if (this != &other) { tree_ = std::move(other.tree_); other.tree_.clear(); }
        return *this;
    }
    int size() const noexcept { return tree_.empty() ? 0 : static_cast<int>(tree_.size() - 1); }
    bool empty() const noexcept { return size() == 0; }
    void add(int i, T delta) {
        check_index(i);
        add_unchecked(static_cast<std::size_t>(i), to_storage(delta));
    }
    T get(int i) const { check_index(i); return static_cast<T>(get_unchecked(static_cast<std::size_t>(i))); }
    void set(int i, T value) {
        check_index(i);
        const Storage delta = to_storage(value) - get_unchecked(static_cast<std::size_t>(i));
        add_unchecked(static_cast<std::size_t>(i), delta);
    }
    T sum(int l, int r) {
        check_range(l, r);
        return static_cast<T>(prefix_unchecked(static_cast<std::size_t>(r)) - prefix_unchecked(static_cast<std::size_t>(l)));
    }
    int lower_bound(const T& target) const {
        if (!(T{} < target)) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (from_storage(candidate) < target) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    int upper_bound(const T& target) const {
        if (target < T{}) return 0;
        const auto n = static_cast<std::size_t>(size());
        std::size_t index = 0;
        Storage current{};
        for (auto jump = std::bit_floor(n); jump; jump >>= 1) {
            if (jump <= n - index) {
                const auto next = index + jump;
                Storage candidate = current + tree_[next];
                if (!(target < from_storage(candidate))) { current = std::move(candidate); index = next; }
            }
        }
        return static_cast<int>(index);
    }
    std::vector<T> values() const {
        if (tree_.empty()) return {};
        std::vector<T> result(tree_.begin() + 1, tree_.end());
        const auto n = static_cast<std::size_t>(size());
        for (auto i = n; i; --i) {
            const auto step = lowbit(i);
            if (step <= n - i) {
                if constexpr (std::is_same_v<T, Storage>) result[i + step - 1] -= result[i - 1];
                else {
                    const Storage difference = static_cast<Storage>(result[i + step - 1]) - static_cast<Storage>(result[i - 1]);
                    result[i + step - 1] = static_cast<T>(difference);
                }
            }
        }
        return result;
    }
    void reset(int n) {
        std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});
        tree_.swap(replacement);
    }
    void reset() { std::fill(tree_.begin(), tree_.end(), Storage{}); }
    void clear() noexcept { tree_.clear(); }
    void swap(fenwick_tree& other) noexcept { tree_.swap(other.tree_); }
};
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

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
int main() {
    int n{}, q{}; verify_in >> n >> q;
    std::vector<std::int64_t> a(n);
    for (auto& x : a) verify_in >> x;
    fenwick_tree<std::int64_t> tree(a);
    while (q--) {
        int type{}, l{}; std::int64_t r{}; verify_in >> type >> l >> r;
        if (type == 0) tree.add(l, r);
        else verify_out << tree.sum(l, static_cast<int>(r)) << '\n';
    }
}
