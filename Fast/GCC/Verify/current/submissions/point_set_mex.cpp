#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc330/tasks/abc330_e

// Library: DataStructure/Point_Set_Mex.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Point_Set_Mex.cpp
// 参考: https://atcoder.jp/contests/abc330/editorial/7752
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/point_set_mex.md
//
// API一覧
// PointSetMex<T=int>(values): 整数列を所有して準備する。長さnは固定。O(n+1)時間・領域。
// PointSetMex<T>(): 空列を用意する。O(1)。コピーは独立。ムーブ元は空列。
// size(): 要素数。get(i): 0始まり添字iの値。mex(): 列にない最小の非負整数。各O(1)。
// set(i,value): 位置iを更新。O(1+log_64(n+1))。負数とnより大きい値も保持し、mexの計算から外す。
// get/setはi<size()が前提。検査有効時の範囲外はstd::out_of_range。
//
// [同梱の階層化ビット集合]
// HierarchicalBitset(n=0): 0..n-1を空集合で初期化。O(1+n/64)時間・ワード。
// HierarchicalBitset(bits): 0/1文字列の各位置を初期状態にする。O(n+1)時間。
// insert(p)/erase(p): 位置pを追加／削除。変更時true。O(1+log_64(n+1))。
// contains(p): 位置pを含むか。O(1)。3操作の位置は0<=p<n。
// next(p): p以上の最小の要素。prev(p): p以下の最大の要素。なければnullopt。O(1+log_64(n+1))。
// size()/count()/empty(): 添字空間長・集合の要素数・集合が空か。O(1)。
// 各64ビット語の非零を次の層で持つ。全体でO(1+n/64)ワード。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#include <concepts>
#include <string>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HIERARCHICAL_BITSET_DEFINED
#define SIRKIT_HIERARCHICAL_BITSET_DEFINED

class HierarchicalBitset
{
public:
    explicit HierarchicalBitset(std::size_t n = 0) : length_(n)
    {
        for (auto bits = length_; bits != 0;)
        {
            const auto words = ((bits - 1) >> 6) + 1;
            levels_.emplace_back(words, 0);
            if (words == 1)
            {
                break;
            }
            bits = words;
        }
    }

    explicit HierarchicalBitset(std::string_view bits) : HierarchicalBitset(bits.size())
    {
        for (std::size_t i = 0; i < length_; ++i)
        {
            const char digit = bits[i];
            if (digit == '1')
            {
                SetBit(levels_[0], i);
                ++count_;
            }
        }
        for (std::size_t level = 1; level < levels_.size(); ++level)
        {
            const auto &lower = levels_[level - 1];
            auto &current = levels_[level];
            for (std::size_t word = 0; word < lower.size(); ++word)
            {
                if (lower[word] != 0)
                {
                    SetBit(current, word);
                }
            }
        }
    }

    HierarchicalBitset(const HierarchicalBitset &) = default;

    HierarchicalBitset(HierarchicalBitset &&other) noexcept
        : levels_(std::move(other.levels_)), length_(std::exchange(other.length_, 0)),
          count_(std::exchange(other.count_, 0))
    {
        other.levels_.clear();
    }

    HierarchicalBitset &operator=(HierarchicalBitset other) noexcept
    {
        levels_.swap(other.levels_);
        std::swap(length_, other.length_);
        std::swap(count_, other.count_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return length_;
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return count_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return count_ == 0;
    }

    [[nodiscard]] bool contains(std::size_t position) const
    {
        CheckPosition(position);
        return (levels_[0][position >> 6] & Mask(position)) != 0;
    }

    bool insert(std::size_t position)
    {
        return Change<true>(position);
    }

    bool erase(std::size_t position)
    {
        return Change<false>(position);
    }

    [[nodiscard]] std::optional<std::size_t> next(std::size_t from) const noexcept
    {
        if (from >= length_)
        {
            return std::nullopt;
        }
        return Find<true>(from);
    }

    [[nodiscard]] std::optional<std::size_t> prev(std::size_t from) const noexcept
    {
        if (length_ == 0)
        {
            return std::nullopt;
        }
        return Find<false>(std::min(from, length_ - 1));
    }

private:
    using Word = std::uint64_t;
    static constexpr Word kAll = ~Word{0};
    std::vector<std::vector<Word>> levels_;
    std::size_t length_, count_ = 0;

    void CheckPosition([[maybe_unused]] std::size_t position) const
    {
    }

    static Word Mask(std::size_t position) noexcept
    {
        return Word{1} << (position & 63);
    }

    static void SetBit(std::vector<Word> &words, std::size_t position) noexcept
    {
        words[position >> 6] |= Mask(position);
    }

    template <bool Present>
    bool Change(std::size_t position)
    {
        if (contains(position) == Present)
        {
            return false;
        }
        if constexpr (Present)
        {
            ++count_;
        }
        else
        {
            --count_;
        }
        for (auto &level : levels_)
        {
            auto &word = level[position >> 6];
            if constexpr (Present)
            {
                const auto before = word;
                word |= Mask(position);
                if (before != 0)
                {
                    break;
                }
            }
            else
            {
                word &= ~Mask(position);
                if (word != 0)
                {
                    break;
                }
            }
            position >>= 6;
        }
        return true;
    }

    template <bool Forward>
    static unsigned SelectBit(Word word) noexcept
    {
        if constexpr (Forward)
        {
            return static_cast<unsigned>(std::countr_zero(word));
        }
        else
        {
            return static_cast<unsigned>(std::bit_width(word) - 1);
        }
    }

    template <bool Forward>
    std::size_t Descend(std::size_t level, std::size_t position) const noexcept
    {
        while (level > 0)
        {
            const auto word = levels_[--level][position];
            position = (position << 6) | SelectBit<Forward>(word);
        }
        return position;
    }

    template <bool Forward>
    std::optional<std::size_t> Find(std::size_t position) const noexcept
    {
        for (std::size_t level = 0; level < levels_.size(); ++level)
        {
            const auto block = position >> 6;
            const auto &words = levels_[level];
            if constexpr (Forward)
            {
                // 上位層で最後の語の直後を参照する場合がある。
                if (block >= words.size())
                {
                    return std::nullopt;
                }
            }
            const auto bit = position & 63;
            const auto mask = Forward ? kAll << bit : kAll >> (63 - bit);
            const auto available = words[block] & mask;
            if (available != 0)
            {
                const auto found = (block << 6) | SelectBit<Forward>(available);
                return Descend<Forward>(level, found);
            }
            if constexpr (Forward)
            {
                position = block + 1;
            }
            else
            {
                if (block == 0)
                {
                    return std::nullopt;
                }
                position = block - 1;
            }
        }
        return std::nullopt;
    }
};

#endif

#ifndef SIRKIT_POINT_SET_MEX_DEFINED
#define SIRKIT_POINT_SET_MEX_DEFINED

template <std::integral T = int>
    requires (sizeof(T) <= sizeof(std::uintmax_t))
class PointSetMex
{
    std::vector<T> values_;
    std::vector<std::size_t> counts_;
    HierarchicalBitset missing_;
    std::size_t mex_ = 0;

    [[nodiscard]] bool tracked(T value) const noexcept
    {
        if constexpr (std::signed_integral<T>)
        {
            if (value < 0)
            {
                return false;
            }
        }
        return static_cast<std::uintmax_t>(value) < counts_.size();
    }

    void check_position([[maybe_unused]] std::size_t position) const
    {
    }

public:
    PointSetMex() = default;

    explicit PointSetMex(std::vector<T> values)
        : values_(std::move(values)), counts_(values_.size() + 1)
    {
        for (T value : values_)
        {
            if (tracked(value))
            {
                ++counts_[static_cast<std::size_t>(value)];
            }
        }
        std::string marks(counts_.size(), '0');
        for (std::size_t i = 0; i < counts_.size(); ++i)
        {
            marks[i] = counts_[i] == 0 ? '1' : '0';
        }
        missing_ = HierarchicalBitset(marks);
        mex_ = *missing_.next(0);
    }

    PointSetMex(const PointSetMex &) = default;

    PointSetMex(PointSetMex &&other) noexcept
        : values_(std::move(other.values_)), counts_(std::move(other.counts_)), missing_(std::move(other.missing_)),
          mex_(std::exchange(other.mex_, 0))
    {
        other.values_.clear();
        other.counts_.clear();
    }

    PointSetMex &operator=(PointSetMex other) noexcept
    {
        values_.swap(other.values_);
        counts_.swap(other.counts_);
        std::swap(missing_, other.missing_);
        std::swap(mex_, other.mex_);
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return values_.size();
    }

    [[nodiscard]] T get(std::size_t position) const
    {
        check_position(position);
        return values_[position];
    }

    [[nodiscard]] std::size_t mex() const noexcept
    {
        return mex_;
    }

    void set(std::size_t position, T value)
    {
        check_position(position);
        const T previous = values_[position];
        if (previous == value)
        {
            return;
        }
        const auto removed = static_cast<std::size_t>(previous);
        if (tracked(previous) && --counts_[removed] == 0)
        {
            missing_.insert(removed);
            mex_ = std::min(mex_, removed);
        }
        const auto added = static_cast<std::size_t>(value);
        if (tracked(value) && counts_[added]++ == 0)
        {
            missing_.erase(added);
            if (mex_ == added)
            {
                // n個の値で0..nのすべてを埋めることはできない。
                mex_ = *missing_.next(added + 1);
            }
        }
        values_[position] = value;
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


#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    std::size_t n{}, q{};
    verify_in >> n >> q;
    std::vector<int> values(n);
    for (auto &value : values)
    {
        verify_in >> value;
    }
    sirkit::PointSetMex index(std::move(values));
    for (std::size_t k = 0; k < q; ++k)
    {
        std::size_t position{};
        int value{};
        verify_in >> position >> value;
        index.set(position - 1, value);
        verify_out << index.mex() << '\n';
    }
}
