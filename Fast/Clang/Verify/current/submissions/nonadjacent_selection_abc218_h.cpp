#define SKIP_RUNTIME_CHECKS
// Profile: Fast/Clang; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc218/tasks/abc218_h

// Library: Algorithm/Nonadjacent_Selection.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Algorithm/Nonadjacent_Selection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/nonadjacent_selection.md
// 参考（貪欲法）: https://atcoder.jp/contests/abc218/editorial/2633
//
// API一覧
// NonadjacentSelection<Score=int64_t>(weights,limit=上限なし): 隣接しない要素をちょうどk個取る最大和を準備する。
// sums(): sums()[k]が最大和。kの範囲は0〜min(limit,ceil(n/2))。参照をO(1)で返す。
// positions(k): 0始まりの添字の一例を昇順で返す。未計算・実行不能のkはnullopt。O(n+K)時間・領域。
// K=min(limit,ceil(n/2))。前処理O(n+K log(n+1))時間・O(n+K)領域。途中の加減算もScoreに収まること。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <ranges>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_NONADJACENT_SELECTION_DEFINED
#define SIRKIT_NONADJACENT_SELECTION_DEFINED

template <class Score = std::int64_t>
class NonadjacentSelection
{
    static constexpr auto kNoIndex = std::numeric_limits<std::size_t>::max();

    struct Slot
    {
        std::size_t previous = 0;
        std::size_t next = 0;
        std::size_t id = kNoIndex;
        Score gain{};
    };

    struct Candidate
    {
        Score gain{};
        std::size_t position = 0;
        std::size_t id = 0;

        bool operator<(const Candidate &other) const
        {
            return gain < other.gain;
        }
    };

public:
    template <class Range>
        requires std::ranges::input_range<const Range>
    explicit NonadjacentSelection(const Range &weights, std::size_t limit = kNoIndex)
    {
        std::vector<Slot> slots;
        std::vector<Candidate> candidates;
        for (const auto &weight : weights)
        {
            const auto id = slots.size();
            const auto gain = static_cast<Score>(weight);
            slots.push_back({id == 0 ? kNoIndex : id - 1, id + 1, id, gain});
            candidates.push_back({gain, id, id});
        }
        size_ = slots.size();
        const auto count = std::min(limit, size_ / 2 + size_ % 2);
        sums_.reserve(count + 1);
        sums_.push_back(Score{});
        if (count == 0)
        {
            return;
        }

        const auto sentinel = size_;
        slots.front().previous = sentinel;
        slots.push_back({size_ - 1, 0, kNoIndex, Score{}});
        std::priority_queue heap(std::less<Candidate>{}, std::move(candidates));
        choices_.reserve(count);
        replacements_.reserve(count);
        auto link = [&](std::size_t left, std::size_t right)
        {
            slots[left].next = right;
            slots[right].previous = left;
        };
        for (;;)
        {
            const auto best = heap.top();
            heap.pop();
            auto &current = slots[best.position];
            if (current.id != best.id)
            {
                continue;
            }
            sums_.push_back(sums_.back() + best.gain);
            choices_.push_back(best.id);
            if (choices_.size() == count)
            {
                break;
            }
            const auto left = current.previous;
            const auto right = current.next;
            const auto before = left == sentinel ? sentinel : slots[left].previous;
            const auto after = right == sentinel ? sentinel : slots[right].next;
            if (left != sentinel && right != sentinel)
            {
                // 両隣へ入れ替える増分を1要素として残す。
                const auto id = size_ + replacements_.size();
                const Score gain = slots[left].gain + slots[right].gain - best.gain;
                replacements_.push_back({slots[left].id, best.id, slots[right].id});
                current = {before, after, id, gain};
                heap.push({gain, best.position, id});
                link(before, best.position);
                link(best.position, after);
            }
            else
            {
                current.id = kNoIndex;
                link(before, after);
            }
            slots[left].id = slots[right].id = kNoIndex;
        }
    }

    [[nodiscard]] const std::vector<Score> &sums() const noexcept
    {
        return sums_;
    }

    [[nodiscard]] std::optional<std::vector<std::size_t>> positions(std::size_t count) const
    {
        if (count >= sums_.size())
        {
            return std::nullopt;
        }
        std::vector<std::uint8_t> chosen(size_ + replacements_.size());
        for (std::size_t i = 0; i < count; ++i)
        {
            chosen[choices_[i]] ^= 1;
        }
        for (auto id = replacements_.size(); id > 0; --id)
        {
            if (chosen[size_ + id - 1] != 0)
            {
                for (auto child : replacements_[id - 1])
                {
                    chosen[child] ^= 1;
                }
            }
        }
        std::vector<std::size_t> result;
        result.reserve(count);
        for (std::size_t i = 0; i < size_; ++i)
        {
            if (chosen[i] != 0)
            {
                result.push_back(i);
            }
        }
        return result;
    }

private:
    std::size_t size_ = 0;
    std::vector<Score> sums_;
    std::vector<std::size_t> choices_;
    std::vector<std::array<std::size_t, 3>> replacements_;
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


#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    std::size_t n{}, red{};
    verify_in >> n >> red;
    std::vector<std::int64_t> rewards(n);
    for (std::size_t i = 1; i < n; ++i)
    {
        std::int64_t weight{};
        verify_in >> weight;
        rewards[i - 1] += weight;
        rewards[i] += weight;
    }
    red = std::min(red, n - red);
    const sirkit::NonadjacentSelection profile(rewards, red);
    verify_out << profile.sums().back() << '\n';
}
