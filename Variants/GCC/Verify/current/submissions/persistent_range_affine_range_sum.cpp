#define SKIP_RUNTIME_CHECKS
// Profile: Variants/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/persistent_range_affine_range_sum

// Library: DataStructure/Persistent_Affine_Range_Sum.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: DataStructure/Persistent_Affine_Range_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_affine_range_sum.md
//
// API一覧
// PersistentAffineRangeSum<Mod>(values): 初期列を構築。O(n+1)時間・領域。法の既定値998244353。
// initial_root(): 初期版のRoot。size()/allocated_nodes(): 長さ・共有領域の節点数。各O(1)。
// range_affine(root,l,r,a,b): 版rootの[l,r)にx→a*x+bを適用した新しい版。
// range_copy(dst,src,l,r): dstの同じ区間にsrcの値を写した新しい版。区間外も両元版も保つ。
// range_sum(root,l,r) / all_sum(root): 指定版の区間和／全体和の非負剰余。const、確保なし。
// 更新・コピーは償却O(log(n+1))時間・追加領域。照会はO(log(n+1))、全体和O(1)。
// 全区間更新・コピーと空区間操作はO(1)で追加節点なし。添字は0始まり、n<=INT_MAX。
// コピーした木は保存領域を共有。Rootは非所有で、保存領域を持つ木が残る間有効。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_AFFINE_RANGE_SUM_DEFINED
#define SIRKIT_PERSISTENT_AFFINE_RANGE_SUM_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline std::size_t CheckedPersistentAffineSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("PersistentAffineRangeSum: too many elements");
    }
    return size;
}

inline std::size_t CheckedPersistentAffineBudget(std::size_t used, std::size_t extra, std::size_t limit)
{
    if (extra > limit - used)
    {
        throw std::length_error("PersistentAffineRangeSum: too many nodes");
    }
    return used + extra;
}
}

template <std::uint32_t Mod = 998244353>
class PersistentAffineRangeSum
{
    static_assert(Mod > 0);
    using Index = std::uint32_t;
    static constexpr std::uint32_t kOne = 1 % Mod;

    struct Affine
    {
        std::uint32_t a, b;

        [[nodiscard]] bool identity() const noexcept
        {
            return a == kOne && b == 0;
        }

        friend constexpr bool operator==(Affine left, Affine right) noexcept
        {
            return left.a == right.a && left.b == right.b;
        }
    };

    struct View
    {
        Index index;
        Affine pending;

        friend constexpr bool operator==(View left, View right) noexcept
        {
            return left.index == right.index && left.pending == right.pending;
        }
    };

    struct Node
    {
        View left, right;
        std::uint32_t sum;
    };

    struct Storage
    {
        static constexpr unsigned kBlockBits = 12;
        static constexpr std::size_t kBlockSize = std::size_t{1} << kBlockBits;
        static constexpr std::size_t kLimit = std::numeric_limits<Index>::max();

        std::vector<std::unique_ptr<Node[]>> blocks;
        std::size_t used = 0;
        int n = 0;
        View initial{0, {kOne, 0}};

        void reserve_more(std::size_t extra)
        {
            const auto needed = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPersistentAffineBudget(used, extra, kLimit);
            const auto count = needed / kBlockSize + (needed % kBlockSize != 0);
            while (blocks.size() < count)
            {
                blocks.push_back(std::make_unique_for_overwrite<Node[]>(kBlockSize));
            }
        }

        [[nodiscard]] const Node &at(Index index) const noexcept
        {
            return blocks[index >> kBlockBits][index & (kBlockSize - 1)];
        }

        View append(const Node &node) noexcept
        {
            const auto index = static_cast<Index>(used++);
            blocks[index >> kBlockBits][index & (kBlockSize - 1)] = node;
            return {index, {kOne, 0}};
        }
    };

public:
    class Root
    {
        friend class PersistentAffineRangeSum;

    public:
        Root() = default;

    private:
        View view_{0, {kOne, 0}};
#ifndef SKIP_RUNTIME_CHECKS
        const Storage *owner_ = nullptr;
#endif
    };

    explicit PersistentAffineRangeSum(std::span<const std::int64_t> values = {})
        : storage_(std::make_shared<Storage>())
    {
        const auto count = /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckedPersistentAffineSize(values.size());
        storage_->n = static_cast<int>(count);
        if (count != 0)
        {
            storage_->reserve_more(2 * count - 1);
            storage_->initial = Build(values);
        }
    }

    [[nodiscard]] int size() const noexcept
    {
        return storage_ ? storage_->n : 0;
    }

    [[nodiscard]] std::size_t allocated_nodes() const noexcept
    {
        return storage_ ? storage_->used : 0;
    }

    [[nodiscard]] Root initial_root() const noexcept
    {
        return storage_ ? MakeRoot(storage_->initial) : Root{};
    }

    [[nodiscard]] Root range_affine(Root from, int left, int right, std::int64_t a, std::int64_t b)
    {
        Check(from, left, right);
        const Affine change{Normalize(a), Normalize(b)};
        if (left == right || change.identity())
        {
            return from;
        }
        if (left == 0 && right == size())
        {
            return MakeRoot(With(from.view_, change));
        }

        ReserveChange();
        return MakeRoot(Update(from.view_, 0, size(), left, right, change));
    }

    [[nodiscard]] Root range_copy(Root dst, Root src, int left, int right)
    {
        Check(dst, left, right);
        CheckOwner(src);
        if (left == right || dst.view_ == src.view_)
        {
            return dst;
        }
        if (left == 0 && right == size())
        {
            return src;
        }

        ReserveChange();
        return MakeRoot(Copy(dst.view_, src.view_, 0, size(), left, right));
    }

    [[nodiscard]] std::uint32_t range_sum(Root root, int left, int right) const
    {
        Check(root, left, right);
        if (left == right)
        {
            return 0;
        }
        return Query(root.view_, 0, size(), left, right);
    }

    [[nodiscard]] std::uint32_t all_sum(Root root) const
    {
        CheckOwner(root);
        return size() == 0 ? 0 : ReadSum(root.view_, size());
    }

private:
    std::shared_ptr<Storage> storage_;

    [[nodiscard]] Root MakeRoot(View view) const noexcept
    {
        Root root;
        root.view_ = view;
#ifndef SKIP_RUNTIME_CHECKS
        root.owner_ = storage_.get();
#endif
        return root;
    }

    void CheckOwner(Root root) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (root.owner_ != storage_.get())
        {
            throw std::invalid_argument("PersistentAffineRangeSum: foreign root");
        }
#else
        (void)root;
#endif
    }

    void Check(Root root, int left, int right) const
    {
        CheckOwner(root);
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || left > right || right > size())
        {
            throw std::out_of_range("PersistentAffineRangeSum: invalid range");
        }
#else
        (void)left;
        (void)right;
#endif
    }

    void ReserveChange()
    {
        // 各深さで部分被覆の境界に接する節点だけを作る。
        storage_->reserve_more(2 * std::bit_width(static_cast<unsigned>(size())));
    }

    static std::uint32_t Normalize(std::int64_t value) noexcept
    {
        const auto residue = value % static_cast<std::int64_t>(Mod);
        return static_cast<std::uint32_t>(residue < 0 ? residue + Mod : residue);
    }

    static std::uint32_t Add(std::uint32_t a, std::uint32_t b) noexcept
    {
        const auto sum = std::uint64_t{a} + b;
        return static_cast<std::uint32_t>(sum >= Mod ? sum - Mod : sum);
    }

    static std::uint32_t Transform(Affine f, std::uint32_t sum, int length) noexcept
    {
        const auto product = std::uint64_t{f.a} * sum;
        const auto offset = std::uint64_t{f.b} * static_cast<unsigned>(length);
        if constexpr (Mod <= (std::uint64_t{1} << 31))
        {
            return static_cast<std::uint32_t>((product + offset) % Mod);
        }
        else
        {
            return Add(static_cast<std::uint32_t>(product % Mod), static_cast<std::uint32_t>(offset % Mod));
        }
    }

    // 親の変換は子が既に持つ変換より後に適用する。
    static View With(View view, Affine after) noexcept
    {
        if (!after.identity())
        {
            view.pending = {
                static_cast<std::uint32_t>(std::uint64_t{after.a} * view.pending.a % Mod),
                Transform(after, view.pending.b, 1),
            };
        }
        return view;
    }

    std::uint32_t ReadSum(View view, int length) const noexcept
    {
        const auto sum = storage_->at(view.index).sum;
        return view.pending.identity() ? sum : Transform(view.pending, sum, length);
    }

    View Combine(View left, View right, int left_size, int right_size) noexcept
    {
        return storage_->append({left, right, Add(ReadSum(left, left_size), ReadSum(right, right_size))});
    }

    View Build(std::span<const std::int64_t> values) noexcept
    {
        if (values.size() == 1)
        {
            return storage_->append({{}, {}, Normalize(values.front())});
        }
        const auto midpoint = values.size() / 2;
        const auto left = Build(values.first(midpoint));
        const auto right = Build(values.subspan(midpoint));
        return Combine(left, right, static_cast<int>(midpoint), static_cast<int>(values.size() - midpoint));
    }

    View Update(View current, int start, int end, int left, int right, Affine change) noexcept
    {
        if (left <= start && end <= right)
        {
            return With(current, change);
        }

        const auto &node = storage_->at(current.index);
        auto low = With(node.left, current.pending);
        auto high = With(node.right, current.pending);
        const auto midpoint = start + (end - start) / 2;
        if (left < midpoint)
        {
            low = Update(low, start, midpoint, left, right, change);
        }
        if (midpoint < right)
        {
            high = Update(high, midpoint, end, left, right, change);
        }
        return Combine(low, high, midpoint - start, end - midpoint);
    }

    View Copy(View target, View source, int start, int end, int left, int right) noexcept
    {
        if (target == source)
        {
            return target;
        }
        if (left <= start && end <= right)
        {
            return source;
        }

        const auto &dst = storage_->at(target.index);
        const auto &src = storage_->at(source.index);
        auto low = With(dst.left, target.pending);
        auto high = With(dst.right, target.pending);
        const auto midpoint = start + (end - start) / 2;
        if (left < midpoint)
        {
            low = Copy(low, With(src.left, source.pending), start, midpoint, left, right);
        }
        if (midpoint < right)
        {
            high = Copy(high, With(src.right, source.pending), midpoint, end, left, right);
        }
        return Combine(low, high, midpoint - start, end - midpoint);
    }

    std::uint32_t Query(View view, int start, int end, int left, int right) const noexcept
    {
        if (left <= start && end <= right)
        {
            return ReadSum(view, end - start);
        }

        const auto &node = storage_->at(view.index);
        const auto midpoint = start + (end - start) / 2;
        if (right <= midpoint)
        {
            return Query(With(node.left, view.pending), start, midpoint, left, right);
        }
        if (left >= midpoint)
        {
            return Query(With(node.right, view.pending), midpoint, end, left, right);
        }
        const auto low = Query(With(node.left, view.pending), start, midpoint, left, right);
        const auto high = Query(With(node.right, view.pending), midpoint, end, left, right);
        return Add(low, high);
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
#include <vector>

int main()
{
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<std::int64_t> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    PersistentAffineRangeSum<> tree(values);
    std::vector<PersistentAffineRangeSum<>::Root> roots(q + 1);
    roots[0] = tree.initial_root();
    for (int i = 1; i <= q; ++i)
    {
        int type = 0, k = 0, left = 0, right = 0;
        verify_in >> type >> k;
        if (type == 1)
        {
            int src = 0;
            verify_in >> src >> left >> right;
            roots[i] = tree.range_copy(roots[k + 1], roots[src + 1], left, right);
        }
        else
        {
            verify_in >> left >> right;
            if (type == 0)
            {
                std::int64_t a = 0, b = 0;
                verify_in >> a >> b;
                roots[i] = tree.range_affine(roots[k + 1], left, right, a, b);
            }
            else
            {
                verify_out << tree.range_sum(roots[k + 1], left, right) << '\n';
            }
        }
    }
}
