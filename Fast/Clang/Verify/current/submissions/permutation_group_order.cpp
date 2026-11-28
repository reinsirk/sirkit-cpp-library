#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/permutation_group_order

// Library: Math/Permutation_Group.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Math/Permutation_Group.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/permutation_group.md
// 参考（固定部分群の構築）: https://arxiv.org/abs/math/9201304
//
// API一覧
// PermutationGroup(n, generators): [0,n)の置換から生成される群を構築。生成元G個でO(n^5+G*n^2)時間、O(n^3)領域。
// degree(): 各置換の長さn。O(1)。
// contains(p): pが群に属するか。O(n^2)時間、O(n)補助領域。
// stabilizer_indices(): i未満の点をすべて固定する置換で、iを移せる点の数をiごとに返す。O(n)。
// order_mod(mod): 群の位数を正のmodで割った余り。O(n)。
// 長さ0の置換と空の生成元列も使える。各置換の要素に重複がなく、0以上n未満であること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERMUTATION_GROUP_DEFINED
#define SIRKIT_PERMUTATION_GROUP_DEFINED
class PermutationGroup
{
public:
    using Permutation = std::vector<int>;

    PermutationGroup(int n, std::span<const Permutation> generators) : degree_(n)
    {
        if (n < 0)
        {
            throw std::invalid_argument("PermutationGroup: degree must be nonnegative");
        }
        Permutation identity(static_cast<std::size_t>(n));
        std::iota(identity.begin(), identity.end(), 0);
        levels_.reserve(identity.size());
        for (int point = 0; point < n; ++point)
        {
            Level level;
            level.slot.assign(static_cast<std::size_t>(n - point), -1);
            level.slot.front() = 0;
            level.representatives.push_back({identity, 0});
            levels_.push_back(std::move(level));
        }
        for (const auto &generator : generators)
        {
            ExtendIfMissing(0, CopyPermutation(generator));
        }
    }

    [[nodiscard]] int degree() const noexcept
    {
        return degree_;
    }

    [[nodiscard]] bool contains(std::span<const int> permutation) const
    {
        return Sift(0, CopyPermutation(permutation));
    }

    [[nodiscard]] std::vector<int> stabilizer_indices() const
    {
        std::vector<int> result;
        result.reserve(levels_.size());
        for (const auto &level : levels_)
        {
            result.push_back(static_cast<int>(level.representatives.size()));
        }
        return result;
    }

    [[nodiscard]] std::uint32_t order_mod(std::uint32_t modulus) const
    {
        std::uint64_t order = 1U % modulus;
        for (const auto &level : levels_)
        {
            order = order * level.representatives.size() % modulus;
        }
        return static_cast<std::uint32_t>(order);
    }

private:
    struct Representative
    {
        Permutation inverse;
        std::size_t checked_generators;
    };

    struct Level
    {
        std::vector<int> slot;
        std::vector<Representative> representatives;
        std::vector<Permutation> generators;
    };

    int degree_;
    std::vector<Level> levels_;

    Permutation CopyPermutation(std::span<const int> permutation) const
    {
        return Permutation(permutation.begin(), permutation.end());
    }

    void Reduce(Permutation &permutation, int base, const Permutation &inverse) const
    {
        for (int point = base; point < degree_; ++point)
        {
            permutation[point] = inverse[permutation[point]];
        }
    }

    bool Sift(int first, Permutation permutation) const
    {
        for (int base = first; base < degree_; ++base)
        {
            const int image = permutation[base];
            if (image == base)
            {
                continue;
            }
            const auto &level = levels_[base];
            const int slot = level.slot[image - base];
            if (slot < 0)
            {
                return false;
            }
            Reduce(permutation, base, level.representatives[slot].inverse);
        }
        return true;
    }

    void ExtendIfMissing(int base, Permutation generator)
    {
        if (Sift(base, generator))
        {
            return;
        }
        auto &level = levels_[base];
        level.generators.push_back(std::move(generator));
        // 各代表元と生成元の積は、現れた順に1回だけ調べる。
        for (std::size_t index = 0; index < level.representatives.size(); ++index)
        {
            while (level.representatives[index].checked_generators < level.generators.size())
            {
                auto &representative = level.representatives[index];
                const auto &next_generator = level.generators[representative.checked_generators++];
                Permutation product = next_generator;
                for (int point = base; point < degree_; ++point)
                {
                    product[representative.inverse[point]] = next_generator[point];
                }
                RegisterProduct(base, std::move(product));
            }
        }
    }

    void RegisterProduct(int base, Permutation product)
    {
        auto &level = levels_[base];
        auto &slot = level.slot[product[base] - base];
        if (slot < 0)
        {
            Permutation inverse = product;
            for (int point = base; point < degree_; ++point)
            {
                inverse[product[point]] = point;
            }
            slot = static_cast<int>(level.representatives.size());
            level.representatives.push_back({std::move(inverse), 0});
            return;
        }
        const Permutation &inverse = level.representatives[slot].inverse;
        // 既知の代表元を取り除くと、先頭base+1点が固定される。
        Reduce(product, base, inverse);
        ExtendIfMissing(base + 1, std::move(product));
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

#include <vector>

int main()
{
    int n{}, k{};
    verify_in >> n >> k;
    using Permutation = sirkit::PermutationGroup::Permutation;
    std::vector<Permutation> generators(k, Permutation(n));
    for (auto &generator : generators)
    {
        for (auto &value : generator)
        {
            verify_in >> value;
        }
    }
    const sirkit::PermutationGroup group(n, generators);
    verify_out << group.order_mod(998'244'353) << '\n';
}
