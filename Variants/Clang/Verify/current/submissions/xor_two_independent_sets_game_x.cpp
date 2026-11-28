#define SKIP_RUNTIME_CHECKS
// Profile: Variants/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/game-24/tasks/game_x

// Library: Math/Xor_Two_Independent_Sets.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Math/Xor_Two_Independent_Sets.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/xor_two_independent_sets.md
// 参考（線形マトロイド和）: https://atcoder.jp/contests/game-24/editorial/25827
//
// API一覧
// XorTwoIndependentSets(): 空にする。uint64_tをGF(2)のベクトルとして扱う。
// insert(value): ベクトルを追加する。最大個数が増えたときtrue。各入力の番号は0始まり。
// rank(): 別々に線形独立な二組へ分けられる入力の最大個数。O(1)。
// groups(): 最大個数を実現する二組の入力番号を、それぞれ昇順で返す。O(W)時間・領域。
// W=64。n回のinsertは合計O(nW+W^3)時間・O(W)領域。乱数は使わない。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_XOR_TWO_INDEPENDENT_SETS_DEFINED
#define SIRKIT_XOR_TWO_INDEPENDENT_SETS_DEFINED

namespace detail
{
class XorUnionBasis
{
    static constexpr auto kBits = std::numeric_limits<std::uint64_t>::digits;

public:
    [[nodiscard]] std::pair<std::uint64_t, std::uint64_t> Reduce(std::uint64_t value) const noexcept
    {
        std::uint64_t mask = 0;
        while (value != 0)
        {
            const auto bit = std::bit_width(value) - 1;
            const auto &[pivot, trace] = rows_[bit];
            if (pivot == 0)
            {
                break;
            }
            value ^= pivot;
            mask ^= trace;
        }
        return {value, mask};
    }

    void Add(std::uint64_t value, std::uint64_t identity = 0) noexcept
    {
        const auto [remainder, trace] = Reduce(value);
        if (remainder != 0)
        {
            rows_[std::bit_width(remainder) - 1] = {remainder, identity ^ trace};
        }
    }

private:
    std::array<std::pair<std::uint64_t, std::uint64_t>, kBits> rows_{};
};
} // namespace detail

class XorTwoIndependentSets
{
    static constexpr auto kBits = std::numeric_limits<std::uint64_t>::digits;
    static constexpr std::size_t kGroups = 2;
    static constexpr std::size_t kCapacity = kGroups * kBits;
    static constexpr std::size_t kUnseen = kCapacity + 1;

    struct Selected
    {
        std::uint64_t value = 0;
        std::size_t original = 0;
        std::size_t group = kGroups;
    };

    struct GroupBasis
    {
        detail::XorUnionBasis basis;
        std::array<std::size_t, kBits> members{};
        std::size_t size = 0;
    };

public:
    bool insert(std::uint64_t value) noexcept
    {
        const auto index = input_count_++;
        if (used_ == kCapacity || blocked_.Reduce(value).first == 0)
        {
            return false;
        }

        const auto start = used_;
        selected_[start] = {value, index, kGroups};
        std::array<std::size_t, kCapacity + 1> parent{};
        parent.fill(kUnseen);
        parent[start] = start;
        std::array<std::size_t, kCapacity + 1> queue{};
        queue[0] = start;
        std::size_t back = 1;
        for (std::size_t front = 0; front < back; ++front)
        {
            const auto id = queue[front];
            for (std::size_t group = 0; group < kGroups; ++group)
            {
                if (selected_[id].group == group)
                {
                    continue;
                }
                const auto &destination = groups_[group];
                const auto [remainder, trace] = destination.basis.Reduce(selected_[id].value);
                if (remainder != 0)
                {
                    Assign(id, group, parent);
                    return true;
                }
                for (auto support = trace; support != 0; support &= support - 1)
                {
                    const auto replace = destination.members[std::countr_zero(support)];
                    if (parent[replace] == kUnseen)
                    {
                        parent[replace] = id;
                        queue[back++] = replace;
                    }
                }
            }
        }
        // 到達したベクトルが張る空間を両組で飽和している。今後の入力もこの空間内なら追加できない。
        for (std::size_t i = 0; i < back; ++i)
        {
            blocked_.Add(selected_[queue[i]].value);
        }
        return false;
    }

    [[nodiscard]] std::size_t rank() const noexcept
    {
        return used_;
    }

    [[nodiscard]] std::array<std::vector<std::size_t>, kGroups> groups() const
    {
        std::array<std::vector<std::size_t>, kGroups> answer;
        for (std::size_t i = 0; i < used_; ++i)
        {
            answer[selected_[i].group].push_back(selected_[i].original);
        }
        return answer;
    }

private:
    void Assign(std::size_t id, std::size_t group, const std::array<std::size_t, kCapacity + 1> &parent) noexcept
    {
        while (id != used_)
        {
            const auto previous_group = selected_[id].group;
            selected_[id].group = group;
            group = previous_group;
            id = parent[id];
        }
        selected_[used_++].group = group;
        groups_ = {};
        for (std::size_t i = 0; i < used_; ++i)
        {
            auto &destination = groups_[selected_[i].group];
            destination.members[destination.size] = i;
            destination.basis.Add(selected_[i].value, std::uint64_t{1} << destination.size);
            ++destination.size;
        }
    }

    std::size_t input_count_ = 0;
    std::size_t used_ = 0;
    std::array<Selected, kCapacity + 1> selected_{};
    std::array<GroupBasis, kGroups> groups_{};
    detail::XorUnionBasis blocked_;
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


#include <cstddef>
#include <cstdint>

int main()
{
    std::size_t tests{};
    verify_in >> tests;
    for (std::size_t i = 0; i < tests; ++i)
    {
        std::size_t count{};
        std::uint64_t target{};
        verify_in >> count >> target;
        sirkit::XorTwoIndependentSets groups;
        for (std::size_t j = 0; j < count; ++j)
        {
            std::uint64_t value{};
            verify_in >> value;
            groups.insert(value);
        }
        groups.insert(target);
        verify_out << (groups.insert(target) ? "Bob\n" : "Alice\n");
    }
}
