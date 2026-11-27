#define SKIP_RUNTIME_CHECKS
// Verify: https://judge.yosupo.jp/problem/static_range_mode_query
// Submission: https://judge.yosupo.jp/submission/419935

// Library: DataStructure/Static_Range_Mode.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/static_range_mode.md
//
// API一覧
// StaticRangeMode<T>(values): 長さnの静的な列を前計算する。O(1+n√n)時間・O(n+1)領域。
// query(l,r): 半開区間[l,r)の最頻値1つと出現回数のpairを返す。空区間はnullopt。O(√n+1)。
// size(): 要素数を返す。O(1)。添字は0始まり、n<=INT_MAX。Tは全順序付きでコピー可能。
// 前計算後は、照会の一覧を用意せず1件ずつ答えられる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_STATIC_RANGE_MODE_DEFINED
#define SIRKIT_STATIC_RANGE_MODE_DEFINED

namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/
{
inline void CheckStaticRangeModeSize(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("StaticRangeMode: too many values");
    }
}
}

template <std::totally_ordered T = std::int64_t>
    requires std::copyable<T>
class StaticRangeMode
{
public:
    explicit StaticRangeMode(std::span<const T> input = {})
    {
        /*SIRKIT_NS_REWRITE_V1:R:OjpyZWluc2lyazo6ZGV0YWls*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::CheckStaticRangeModeSize(input.size());
        if (input.empty())
        {
            return;
        }

        alphabet_.assign(input.begin(), input.end());
        std::sort(alphabet_.begin(), alphabet_.end());
        alphabet_.erase(std::unique(alphabet_.begin(), alphabet_.end()), alphabet_.end());

        ids_.resize(input.size());
        offsets_.resize(alphabet_.size() + 1);
        for (std::size_t i = 0; i < input.size(); ++i)
        {
            const auto id = std::lower_bound(alphabet_.begin(), alphabet_.end(), input[i]) - alphabet_.begin();
            ids_[i] = static_cast<int>(id);
            ++offsets_[id + 1];
        }
        std::partial_sum(offsets_.begin(), offsets_.end(), offsets_.begin());
        positions_.resize(input.size());
        ranks_.resize(input.size());
        {
            auto next = offsets_;
            for (int i = 0; i < size(); ++i)
            {
                const auto rank = next[ids_[i]]++;
                ranks_[i] = rank;
                positions_[rank] = i;
            }
        }
        BuildBlockModes();
    }

    [[nodiscard]] int size() const noexcept
    {
        return static_cast<int>(ids_.size());
    }

    [[nodiscard]] std::optional<std::pair<T, int>> query(int left, int right) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left < 0 || left > right || right > size())
        {
            throw std::out_of_range("StaticRangeMode: invalid range");
        }
#endif
        if (left == right)
        {
            return std::nullopt;
        }

        const int first_block = left / block_size_ + (left % block_size_ != 0);
        const int last_block = right / block_size_;
        Candidate best{};
        int left_end = right;
        int right_begin = right;
        if (first_block < last_block)
        {
            best = block_modes_[TableIndex(first_block, last_block - 1)];
            left_end = first_block * block_size_;
            right_begin = last_block * block_size_;
        }

        // 左端の候補は右へ、右端の候補は左へ数える。
        // bestより1回多く出るかだけを調べ、見つかった分だけしきい値を上げる。
        for (int i = left; i < left_end; ++i)
        {
            const auto id = ids_[i];
            const auto rank = ranks_[i];
            const auto remaining = offsets_[id + 1] - rank;
            while (best.count < remaining && positions_[rank + best.count] < right)
            {
                best = {id, best.count + 1};
            }
        }
        for (int i = right; i > right_begin;)
        {
            --i;
            const auto id = ids_[i];
            const auto rank = ranks_[i];
            const auto preceding = rank - offsets_[id];
            while (best.count <= preceding && positions_[rank - best.count] >= left)
            {
                best = {id, best.count + 1};
            }
        }
        return std::pair<T, int>{alphabet_[best.id], best.count};
    }

private:
    struct Candidate
    {
        int id = 0, count = 0;
    };

    std::vector<T> alphabet_;
    std::vector<int> ids_, offsets_, positions_, ranks_;
    std::vector<Candidate> block_modes_;
    int block_size_ = 1, blocks_ = 0;

    [[nodiscard]] std::size_t TableIndex(int first, int last) const noexcept
    {
        return static_cast<std::size_t>(first) * blocks_ + last;
    }

    void BuildBlockModes()
    {
        block_size_ = static_cast<int>(std::sqrt(size())) + 1;
        blocks_ = size() / block_size_;
        block_modes_.resize(static_cast<std::size_t>(blocks_) * blocks_);
        std::vector<int> counts(alphabet_.size());
        for (int first = 0; first < blocks_; ++first)
        {
            std::fill(counts.begin(), counts.end(), 0);
            Candidate best{};
            int i = first * block_size_;
            for (int last = first; last < blocks_; ++last)
            {
                const int end = (last + 1) * block_size_;
                for (; i < end; ++i)
                {
                    const auto id = ids_[i];
                    const auto count = ++counts[id];
                    if (count > best.count)
                    {
                        best = {id, count};
                    }
                }
                block_modes_[TableIndex(first, last)] = best;
            }
        }
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
    int n = 0, q = 0;
    verify_in >> n >> q;
    std::vector<int> data(n);
    for (auto &value : data)
    {
        verify_in >> value;
    }
    const StaticRangeMode<int> mode(data);
    while (q-- > 0)
    {
        int left = 0, right = 0;
        verify_in >> left >> right;
        const auto [value, count] = *mode.query(left, right);
        verify_out << value << ' ' << count << '\n';
    }
}
