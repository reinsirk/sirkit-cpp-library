#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group

// Library: DataStructure/Noncommutative_Weighted_Union_Find.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: DataStructure/Noncommutative_Weighted_Union_Find.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/noncommutative_weighted_union_find.md
//
// API一覧
// NoncommutativeWeightedUnionFind<S,op,e,inverse>(n=0): n頂点を独立に構築。O(n+1)時間・領域。
// unite_checked(from,to,w): A_to=A_from*wを追加。Merged/AlreadyConsistent/Contradictionを返す。
// constrain(from,to,w): 矛盾しない場合true。merge: 新規結合ならtrue。unite: 戻り値なし。
// diff(from,to): A_from^{-1}*A_to。try_diffは別成分ならnullopt、consistentは制約を追加せず判定。
// root(i)/leader(i), same(a,b)/issame(a,b), size(i): 代表、連結判定、成分サイズ。
// weight(i): 代表からiへの差。root_with_weight(i): {代表, weight(i)}。
// size()/empty()/component_count(): 全頂点数、空判定、成分数。O(1)。他は償却O(α(n+1))。
// op/e/inverseは群の積/単位元/逆元。Sはコピー可能でムーブ代入が非送出、等値比較可能。
// 頂点は0..n-1、n<=INT_MAX。演算・コピーの計算時間をO(1)としたときの計算量。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NONCOMMUTATIVE_WEIGHTED_UNION_FIND_DEFINED
#define SIRKIT_NONCOMMUTATIVE_WEIGHTED_UNION_FIND_DEFINED

template <std::equality_comparable S, auto op, auto e, auto inverse>
class NoncommutativeWeightedUnionFind
{
    static_assert(std::is_invocable_r_v<S, decltype(op), const S &, const S &>);
    static_assert(std::is_invocable_r_v<S, decltype(e)>);
    static_assert(std::is_invocable_r_v<S, decltype(inverse), const S &>);
    static_assert(std::is_copy_constructible_v<S> && std::is_move_constructible_v<S> &&
                  std::is_nothrow_move_assignable_v<S>);

public:
    enum class MergeResult
    {
        Merged,
        AlreadyConsistent,
        Contradiction,
    };

    explicit NoncommutativeWeightedUnionFind(std::size_t n = 0)
        : parent_(CheckedSize(n), -1), potential_(n, std::invoke(e)), components_(static_cast<int>(n))
    {
    }

    NoncommutativeWeightedUnionFind(const NoncommutativeWeightedUnionFind &) = default;

    NoncommutativeWeightedUnionFind &operator=(const NoncommutativeWeightedUnionFind &other)
    {
        if (this != &other)
        {
            NoncommutativeWeightedUnionFind replacement(other);
            Swap(replacement);
        }
        return *this;
    }

    NoncommutativeWeightedUnionFind(NoncommutativeWeightedUnionFind &&other) noexcept
        : parent_(std::move(other.parent_)), potential_(std::move(other.potential_)),
          components_(std::exchange(other.components_, 0))
    {
        other.parent_.clear();
        other.potential_.clear();
    }

    NoncommutativeWeightedUnionFind &operator=(NoncommutativeWeightedUnionFind &&other) noexcept
    {
        if (this != &other)
        {
            NoncommutativeWeightedUnionFind replacement(std::move(other));
            Swap(replacement);
        }
        return *this;
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(parent_.size());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return parent_.empty();
    }

    [[nodiscard]] int component_count() const noexcept
    {
        return components_;
    }

    [[nodiscard]] int root(int vertex)
    {
        CheckVertex(vertex);
        return Compress(vertex);
    }

    [[nodiscard]] int leader(int vertex)
    {
        return root(vertex);
    }

    [[nodiscard]] bool same(int a, int b)
    {
        CheckVertex(a);
        CheckVertex(b);
        const auto representative = Compress(a);
        return representative == Compress(b);
    }

    [[nodiscard]] bool issame(int a, int b)
    {
        return same(a, b);
    }

    [[nodiscard]] int size(int vertex)
    {
        return -parent_[root(vertex)];
    }

    [[nodiscard]] S weight(int vertex)
    {
        CheckVertex(vertex);
        Compress(vertex);
        return potential_[vertex];
    }

    [[nodiscard]] std::pair<int, S> root_with_weight(int vertex)
    {
        const auto representative = root(vertex);
        return {representative, potential_[vertex]};
    }

    [[nodiscard]] std::optional<S> try_diff(int from, int to)
    {
        if (!same(from, to))
        {
            return std::nullopt;
        }
        return Product(Inverse(potential_[from]), potential_[to]);
    }

    [[nodiscard]] S diff(int from, int to)
    {
        auto result = try_diff(from, to);
#ifndef SKIP_RUNTIME_CHECKS
        if (!result)
        {
            throw std::invalid_argument("NoncommutativeWeightedUnionFind: disjoint vertices");
        }
#endif
        return std::move(*result);
    }

    [[nodiscard]] bool consistent(int from, int to, const S &relation)
    {
        return !same(from, to) || Product(potential_[from], relation) == potential_[to];
    }

    MergeResult unite_checked(int from, int to, const S &relation)
    {
        CheckVertex(from);
        CheckVertex(to);
        auto keep = Compress(from);
        auto absorbed = Compress(to);
        const auto proposed = Product(potential_[from], relation);
        if (keep == absorbed)
        {
            return proposed == potential_[to] ? MergeResult::AlreadyConsistent : MergeResult::Contradiction;
        }

        // A_to=A_from*relation。根保持側から吸収側への差はg_from*relation*inv(g_to)。
        auto bridge = Product(proposed, Inverse(potential_[to]));
        if (parent_[keep] > parent_[absorbed])
        {
            bridge = Inverse(bridge);
            std::swap(keep, absorbed);
        }
        potential_[absorbed] = std::move(bridge);
        parent_[keep] += parent_[absorbed];
        parent_[absorbed] = keep;
        --components_;
        return MergeResult::Merged;
    }

    bool constrain(int from, int to, const S &relation)
    {
        return unite_checked(from, to, relation) != MergeResult::Contradiction;
    }

    bool merge(int from, int to, const S &relation)
    {
        return unite_checked(from, to, relation) == MergeResult::Merged;
    }

    void unite(int from, int to, const S &relation)
    {
        (void)unite_checked(from, to, relation);
    }

private:
    std::vector<int> parent_;
    // potential_[v] は親からvへの差。根の値は単位元。
    std::vector<S> potential_;
    int components_;

    static std::size_t CheckedSize(std::size_t n)
    {
        if (n > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            throw std::length_error("NoncommutativeWeightedUnionFind: too many vertices");
        }
        return n;
    }

    void Swap(NoncommutativeWeightedUnionFind &other) noexcept
    {
        parent_.swap(other.parent_);
        potential_.swap(other.potential_);
        std::swap(components_, other.components_);
    }

    void CheckVertex(int vertex) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(vertex) >= parent_.size())
        {
            throw std::out_of_range("NoncommutativeWeightedUnionFind: invalid vertex");
        }
#else
        (void)vertex;
#endif
    }

    static S Product(const S &a, const S &b)
    {
        return std::invoke(op, a, b);
    }

    static S Inverse(const S &value)
    {
        return std::invoke(inverse, value);
    }

    int Compress(int vertex)
    {
        const int parent = parent_[vertex];
        if (parent < 0)
        {
            return vertex;
        }
        if (parent_[parent] < 0)
        {
            return parent;
        }

        const int representative = Compress(parent);
        auto combined = Product(potential_[parent], potential_[vertex]);
        potential_[vertex] = std::move(combined);
        parent_[vertex] = representative;
        return representative;
    }
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

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Matrix
{
    std::uint32_t a, b, c, d;
    bool operator==(const Matrix &) const = default;
};

constexpr std::uint32_t Dot(std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d)
{
    return static_cast<std::uint32_t>((std::uint64_t{a} * b + std::uint64_t{c} * d) % kMod);
}

constexpr Matrix Product(const Matrix &a, const Matrix &b)
{
    return {
        Dot(a.a, b.a, a.b, b.c),
        Dot(a.a, b.b, a.b, b.d),
        Dot(a.c, b.a, a.d, b.c),
        Dot(a.c, b.b, a.d, b.d),
    };
}

constexpr Matrix Identity()
{
    return {1, 0, 0, 1};
}

constexpr std::uint32_t Negate(std::uint32_t value)
{
    return value == 0 ? 0 : kMod - value;
}

constexpr Matrix Inverse(const Matrix &value)
{
    return {value.d, Negate(value.b), Negate(value.c), value.a};
}
}

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    NoncommutativeWeightedUnionFind<Matrix, Product, Identity, Inverse> uf(n);
    while (q-- > 0)
    {
        int type = 0, u = 0, v = 0;
        verify_in >> type >> u >> v;
        if (type == 0)
        {
            Matrix relation{};
            verify_in >> relation.a >> relation.b >> relation.c >> relation.d;
            verify_out << static_cast<int>(uf.constrain(v, u, relation)) << '\n';
        }
        else
        {
            const auto result = uf.try_diff(v, u);
            if (result)
            {
                verify_out << result->a << ' ' << result->b << ' ' << result->c << ' ' << result->d << '\n';
            }
            else
            {
                verify_out << -1 << '\n';
            }
        }
    }
}
