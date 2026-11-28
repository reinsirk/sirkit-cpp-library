#define SKIP_RUNTIME_CHECKS
// Verify: https://yukicoder.me/problems/no/626

// Library: Algorithm/Sparse_Knapsack.cpp
// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/sparse_knapsack.md
// 参考（Verify問題）: https://yukicoder.me/problems/no/626
//
// API一覧
// [SparseKnapsack::Item] weight,value: 非負uint64_t。各品物を1回まで選ぶ。
// SparseKnapsack(capacity,items): 容量以下の支配されない候補を前計算する。
// n品、処理iの直前にP_i候補ならO(n log(n+1)+ΣP_i)時間、O(n+max P_i)領域。実行時間は候補数に依存する。
// max_value([budget]): budget以下で可能な最大価値。既定はcapacity、budget<=capacity。O(log(P+1))。
// frontier(): 価値が正の候補をstd::span<const Item>で返す。重さ・価値が両方昇順。O(1)。
// 最終最適値がuint64_tを超えるなら構築時にoverflow_error。何も選ばない価値は0。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SPARSE_KNAPSACK_DEFINED
#define SIRKIT_SPARSE_KNAPSACK_DEFINED
class SparseKnapsack
{
public:
    struct Item
    {
        std::uint64_t weight = 0, value = 0;
    };

    SparseKnapsack(std::uint64_t capacity, std::span<const Item> items) : capacity_(capacity)
    {
        frontier_.push_back({0, 0});
        std::vector<Item> ordered;
        ordered.reserve(items.size());
        for (const auto &item : items)
        {
            if (item.weight <= capacity && item.value != 0)
            {
                ordered.push_back(item);
            }
        }
        std::sort(ordered.begin(), ordered.end(), [](const Item &a, const Item &b)
                  {
                      return a.weight > b.weight;
                  });
        std::vector<Item> buffer;
        for (const auto &item : ordered)
        {
            Extend(item, buffer);
        }
        if (frontier_.front().value == 0)
        {
            frontier_.erase(frontier_.begin());
        }
    }

    [[nodiscard]] std::uint64_t max_value(std::uint64_t budget) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (budget > capacity_)
        {
            throw std::out_of_range("SparseKnapsack: budget exceeds prepared capacity");
        }
#endif
        const auto it = UpperWeight(budget);
        return it == frontier_.begin() ? 0 : std::prev(it)->value;
    }

    [[nodiscard]] std::uint64_t max_value() const
    {
        return max_value(capacity_);
    }

    [[nodiscard]] std::span<const Item> frontier() const noexcept
    {
        return frontier_;
    }

private:
    std::uint64_t capacity_;
    std::vector<Item> frontier_;

    std::vector<Item>::const_iterator UpperWeight(std::uint64_t weight) const noexcept
    {
        return std::upper_bound(frontier_.begin(), frontier_.end(), weight,
                                [](std::uint64_t bound, const Item &item)
                                {
                                    return bound < item.weight;
                                });
    }

    void Extend(const Item &item, std::vector<Item> &buffer)
    {
        const auto limit = static_cast<std::size_t>(UpperWeight(capacity_ - item.weight) - frontier_.begin());
        if (item.value > std::numeric_limits<std::uint64_t>::max() - frontier_[limit - 1].value)
        {
            throw std::overflow_error("SparseKnapsack: feasible value exceeds uint64_t");
        }
        buffer.clear();
        const auto append = [&buffer](Item point)
        {
            if (buffer.empty())
            {
                buffer.push_back(point);
            }
            else if (point.value > buffer.back().value)
            {
                if (point.weight == buffer.back().weight)
                {
                    buffer.back().value = point.value;
                }
                else
                {
                    buffer.push_back(point);
                }
            }
        };
        const auto shifted = [&](std::size_t index)
        {
            return Item{frontier_[index].weight + item.weight, frontier_[index].value + item.value};
        };
        // 品物を選ぶ列と選ばない列を重さ順で併合し、重くて価値も低い点を落とす。
        std::size_t a = 0, b = 0;
        while (a < frontier_.size() && b < limit)
        {
            const auto choice = shifted(b);
            if (frontier_[a].weight < choice.weight)
            {
                append(frontier_[a++]);
            }
            else
            {
                append(choice);
                ++b;
            }
        }
        for (; a < frontier_.size(); ++a)
        {
            append(frontier_[a]);
        }
        for (; b < limit; ++b)
        {
            append(shifted(b));
        }
        frontier_.swap(buffer);
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
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{};
    std::uint64_t capacity{};
    verify_in >> n >> capacity;
    std::vector<sirkit::SparseKnapsack::Item> items(n);
    for (auto &[weight, value] : items)
    {
        verify_in >> value >> weight;
    }
    const sirkit::SparseKnapsack knapsack(capacity, items);
    verify_out << knapsack.max_value() << '\n';
}
