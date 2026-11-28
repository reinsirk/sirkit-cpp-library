#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/set_xor_min

// Library: DataStructure/Persistent_Binary_Trie.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: DataStructure/Persistent_Binary_Trie.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_binary_trie.md
//
// API一覧
// PersistentBinaryTrie<T,MAX_LOG>(expected_updates=0): 空の多重集合。更新回数の見込みに応じて容量を確保する。
// Tはboolを除く符号なし整数。MAX_LOGは最高位ビット番号で既定値はTのビット数-1。B=MAX_LOG+1。
// Version{}: 空集合の版。各版は作成した木で使う。キー・マスクはBビット以内。
// size(version): 多重集合の要素数を返す。O(1)。count(version,key): キーの個数を返す。O(B)。
// insert(version,key): キーを1個追加した版を返す。元の版は保持する。償却O(B)時間・追加O(B)領域。
// erase(version,key): キーを1個除いた版を返す。存在しない場合nullopt。償却O(B)時間・追加O(B)領域。
// kth_xor(before,after,k,mask=0): 差分中でkey^maskが0始まりk番目の値。なければnullopt。O(B)。
// count_xor_less(before,after,bound,mask=0): 差分中でkey^mask<boundとなる要素数。O(B)。
// beforeはafterの部分多重集合。insertで作った接頭辞の版を使えば添字区間を照会できる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_PERSISTENT_BINARY_TRIE_DEFINED
#define SIRKIT_PERSISTENT_BINARY_TRIE_DEFINED

template <std::unsigned_integral T = std::uint32_t, int MAX_LOG = std::numeric_limits<T>::digits - 1>
    requires(!std::same_as<T, bool> && MAX_LOG >= 0 && MAX_LOG < std::numeric_limits<T>::digits)
class PersistentBinaryTrie
{
    using Index = std::uint32_t;
    static constexpr std::size_t kPathNodes = MAX_LOG + 2;

public:
    class Version
    {
        friend class PersistentBinaryTrie;

    public:
        Version() = default;

    private:
        explicit Version(Index root) : root_(root)
        {
        }

        Index root_ = 0;
    };

    explicit PersistentBinaryTrie(std::size_t expected_updates = 0)
    {
        Prepare(expected_updates);
    }

    std::uint32_t size(Version version) const
    {
        CheckVersion(version);
        return Get(version.root_).count;
    }

    std::uint32_t count(Version version, T key) const
    {
        CheckVersion(version);
        CheckKey(key);
        Index node = version.root_;
        for (int bit = MAX_LOG; bit >= 0 && node != 0; --bit)
        {
            node = Get(node).children[Bit(key, bit)];
        }
        return Get(node).count;
    }

    Version insert(Version version, T key)
    {
        CheckVersion(version);
        CheckKey(key);
        Prepare(1);
        return Version(Change<true>(version.root_, key, MAX_LOG));
    }

    std::optional<Version> erase(Version version, T key)
    {
        if (count(version, key) == 0)
        {
            return std::nullopt;
        }
        Prepare(1);
        return Version(Change<false>(version.root_, key, MAX_LOG));
    }

    std::optional<T> kth_xor(Version before, Version after, std::uint32_t k, T mask = 0) const
    {
        CheckInterval(before, after);
        CheckKey(mask);
        if (k >= Difference(before.root_, after.root_))
        {
            return std::nullopt;
        }
        T result = 0;
        Index left = before.root_;
        Index right = after.root_;
        for (int bit = MAX_LOG; bit >= 0; --bit)
        {
            const auto side = Bit(mask, bit);
            const auto count_zero = Difference(Get(left).children[side], Get(right).children[side]);
            std::size_t direction = side;
            if (k >= count_zero)
            {
                k -= count_zero;
                direction ^= 1;
                result |= T{1} << bit;
            }
            left = Get(left).children[direction];
            right = Get(right).children[direction];
        }
        return result;
    }

    std::uint32_t count_xor_less(Version before, Version after, T bound, T mask = 0) const
    {
        CheckInterval(before, after);
        CheckKey(mask);
        Index left = before.root_;
        Index right = after.root_;
        if (!Fits(bound))
        {
            return Difference(left, right);
        }
        std::uint32_t smaller = 0;
        for (int bit = MAX_LOG; bit >= 0 && left != right; --bit)
        {
            const auto zero = Bit(mask, bit);
            const auto allows_zero = Bit(bound, bit);
            if (allows_zero)
            {
                smaller += Difference(Get(left).children[zero], Get(right).children[zero]);
            }
            const auto direction = zero ^ allows_zero;
            left = Get(left).children[direction];
            right = Get(right).children[direction];
        }
        return smaller;
    }

private:
    struct Node
    {
        std::array<Index, 2> children{};
        Index count{};
    };

    static constexpr Node kEmpty{};
    std::vector<Node> nodes_;

    const Node &Get(Index id) const
    {
        return id == 0 ? kEmpty : nodes_[id - 1];
    }

    static std::size_t Bit(T key, int bit)
    {
        return (key >> bit) & T{1};
    }

    static bool Fits(T key)
    {
        return (key >> MAX_LOG) <= T{1};
    }

    void Prepare(std::size_t updates)
    {
        const auto limit = std::min<std::size_t>(std::numeric_limits<Index>::max(), nodes_.max_size());
        if (updates > (limit - nodes_.size()) / kPathNodes)
        {
            throw std::length_error("PersistentBinaryTrie: too many nodes");
        }
        const auto need = nodes_.size() + updates * kPathNodes;
        const auto capacity = nodes_.capacity();
        if (need > capacity)
        {
            const auto growth = std::min(capacity / 2, limit - capacity);
            nodes_.reserve(std::max(need, capacity + growth));
        }
    }

    template <bool Add>
    Index Change(Index id, T key, int bit)
    {
        Node copy = Get(id);
        if constexpr (Add)
        {
            ++copy.count;
        }
        else
        {
            if (--copy.count == 0)
            {
                return 0;
            }
        }
        if (bit >= 0)
        {
            const auto side = Bit(key, bit);
            copy.children[side] = Change<Add>(copy.children[side], key, bit - 1);
        }
        nodes_.push_back(copy);
        return static_cast<Index>(nodes_.size());
    }

    std::uint32_t Difference(Index before, Index after) const
    {
        return Get(after).count - Get(before).count;
    }

    void CheckVersion([[maybe_unused]] Version version) const
    {
    }

    static void CheckKey([[maybe_unused]] T key)
    {
    }

    void CheckInterval([[maybe_unused]] Version before, [[maybe_unused]] Version after) const
    {
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

int main()
{
    unsigned q;
    verify_in >> q;
    using Trie = sirkit::PersistentBinaryTrie<std::uint32_t, 29>;
    Trie trie(q);
    Trie::Version current;
    while (q-- > 0)
    {
        int type;
        std::uint32_t value;
        verify_in >> type >> value;
        if (type == 0)
        {
            if (trie.count(current, value) == 0)
            {
                current = trie.insert(current, value);
            }
        }
        else if (type == 1)
        {
            if (const auto next = trie.erase(current, value))
            {
                current = *next;
            }
        }
        else
        {
            verify_out << *trie.kth_xor({}, current, 0, value) << '\n';
        }
    }
}
