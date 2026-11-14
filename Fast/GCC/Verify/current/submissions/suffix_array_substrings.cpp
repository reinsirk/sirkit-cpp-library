#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
// Verify: https://judge.yosupo.jp/problem/number_of_substrings

// Library: String/Suffix_Array.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Suffix_Array.cpp
// 参考（数学的なSA-ISの原論文。コードは独自実装）:
// https://doi.org/10.1109/DCC.2009.42
// 参考（ACLの公開API）: https://atcoder.github.io/ac-library/production/document_ja/string.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/suffix_array.md
//
// API一覧
// suffix_array(const std::string&)/suffix_array(string_view): 符号なしバイトの辞書順で接尾辞の開始位置を返す。空接尾辞は含めない。
// C文字列のsuffix_array(char*/const char*)も可。終端あり・nullptr不可。文字列を複製しない。
// suffix_array(vector<int>, upper): 全要素0..upperの整数列。SA-ISでO(n+upper)時間・領域。
// suffix_array<T>(const vector<T>&): Tの<で圧縮して構築。負数/uint64_t最大値も可、O(n log n)時間・O(n)領域。
// upperは0以上INT_MAX-1未満。文字列版はO(n)時間・領域。全版で列長はINT_MAX-1未満、空なら{}。返却添字はint。
// lcp_array(const string&,sa)/lcp_array<T>(const vector<T>&,sa): 隣り合う接尾辞の共通接頭辞長を返す。n<2なら{}、それ以外はn-1個。O(n)時間・領域。
// 追加のlcp_array(sequence,sa)はstring_view/array等にも対応。
// lcp_arrayのsaはsの正しい接尾辞配列が前提。既定検査はサイズ・添字・重複を確認し、辞書順までは再検査しない。
// SuffixArrayEqualRange(s,sa,pattern): patternを接頭辞に持つ接尾辞のsa内半開区間を返す。
// 検索はO((m+1)log(n+1))時間・追加O(1)。string_view版は符号なしバイト順、整数vector版はTの順序。
// 各比較/添字アクセスがO(1)の場合の計算量。Tの<は一貫した順序が前提。
// 空patternは全接尾辞に一致。検索はsaの妥当性を全走査せず、実際に参照する添字だけ検査する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUFFIX_ARRAY_DETAIL_DEFINED
#define SIRKIT_SUFFIX_ARRAY_DETAIL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
inline void suffix_length_check(std::size_t n) {
    if (n >= static_cast<std::size_t>(std::numeric_limits<int>::max() - 1))
        throw std::length_error("suffix_array: sequence too long");
}

// 正の整数列に一意な終端0を付けた列を扱う。再帰先にも終端0を残す。
template<class Symbol> std::vector<int> suffix_induced(std::span<const Symbol> symbols, int alphabet) {
    const int n = static_cast<int>(symbols.size());
    if (n == 1) return {0};
    // 0=L型、1=S型、3=LMS。隣接2位置の型を決めてから境界印を重ねる。
    std::vector<std::uint8_t> small(n);
    std::vector<int> selected;
    selected.reserve(static_cast<std::size_t>(n / 2));
    std::vector<int> starts(static_cast<std::size_t>(alphabet) + 1), ends(starts.size());
    ++ends[symbols[n - 1]];
    small[n - 1] = 1;
    for (int i = n - 2; i >= 0; --i) {
        ++ends[symbols[i]];
        small[i] = symbols[i] < symbols[i + 1] ||
            (symbols[i] == symbols[i + 1] && small[i + 1]);
        if (!small[i] && small[i + 1]) { small[i + 1] |= 2; selected.push_back(i + 1); }
    }
    std::reverse(selected.begin(), selected.end());
    int total = 0;
    for (int symbol = 0; symbol <= alphabet; ++symbol) {
        starts[symbol] = total; total += ends[symbol]; ends[symbol] = total;
    }
    std::vector<int> suffixes(n, -1), cursor(ends.size());
    const auto induce = [&](const std::vector<int>& order, bool clear) {
        if (clear) std::fill(suffixes.begin(), suffixes.end(), -1);
        cursor = ends;
        for (auto it = order.rbegin(); it != order.rend(); ++it)
            suffixes[--cursor[symbols[*it]]] = *it;
        cursor = starts;
        for (int slot = 0; slot < n; ++slot) {
            const int before = suffixes[slot] - 1;
            if (before >= 0 && !small[before])
                suffixes[cursor[symbols[before]]++] = before;
        }
        cursor = ends;
        for (int slot = n - 1; slot >= 0; --slot) {
            const int before = suffixes[slot] - 1;
            if (before >= 0 && small[before])
                suffixes[--cursor[symbols[before]]] = before;
        }
    };
    induce(selected, false);
    const int count = static_cast<int>(selected.size());
    // LMSが終端だけなら、再誘導しても同じ順序になる。
    if (count == 1) return suffixes;
    std::vector<int> reduced(count);
    // LMSは隣接しないため位置/2が一意。LMSの欄だけ設定・参照する。
    auto selected_id = std::make_unique_for_overwrite<int[]>(static_cast<std::size_t>((n + 1) / 2));
    for (int i = 0; i < count; ++i) selected_id[selected[i] / 2] = i;
    int previous = -1, name = -1;
    for (int position : suffixes) {
        if ((small[position] & 2) == 0) continue;
        bool different = previous < 0;
        if (!different) {
            int offset = 0;
            for (;;) {
                if (symbols[position + offset] != symbols[previous + offset] ||
                    small[position + offset] != small[previous + offset]) {
                    different = true; break;
                }
                if (offset > 0 && ((small[position + offset] & 2) || (small[previous + offset] & 2))) {
                    different = (small[position + offset] & 2) != (small[previous + offset] & 2); break;
                }
                ++offset;
            }
        }
        if (different) ++name;
        reduced[selected_id[position / 2]] = name;
        previous = position;
    }
    selected_id.reset();
    const bool distinct_names = name + 1 == count;
    auto reduced_order = distinct_names ? std::vector<int>(count) : suffix_induced(std::span<const int>(reduced), name);
    if (distinct_names) {
        for (int i = 0; i < count; ++i) reduced_order[reduced[i]] = i;
    }
    for (int& position : reduced_order) position = selected[position];
    induce(reduced_order, true);
    return suffixes;
}

inline std::vector<int> suffix_string(std::string_view s) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    auto encoded = std::make_unique_for_overwrite<std::uint16_t[]>(s.size() + 1);
    encoded[s.size()] = 0;
    for (std::size_t i = 0; i < s.size(); ++i) encoded[i] = static_cast<std::uint16_t>(static_cast<unsigned char>(s[i]) + 1);
    auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const std::uint16_t>(encoded.get(), s.size() + 1), 256);
    result.erase(result.begin());
    return result;
}

template <class Sequence, class Pattern, class Convert>
std::pair<int, int> suffix_search(const Sequence& s, const std::vector<int>& sa,
                                 const Pattern& pattern, Convert convert) {
    suffix_length_check(s.size());
    const auto compare = [&](int slot) {
        const int position = sa[slot];
        std::size_t offset = 0;
        const std::size_t remaining = s.size() - static_cast<std::size_t>(position);
        while (offset < pattern.size() && offset < remaining) {
            const auto a = convert(s[static_cast<std::size_t>(position) + offset]);
            const auto b = convert(pattern[offset]);
            if (a < b) return -1;
            if (b < a) return 1;
            ++offset;
        }
        return offset == pattern.size() ? 0 : -1;
    };
    int first = 0, last = static_cast<int>(sa.size());
    while (first < last) {
        const int middle = first + (last - first) / 2;
        if (compare(middle) < 0) first = middle + 1; else last = middle;
    }
    const int lower = first;
    last = static_cast<int>(sa.size());
    while (first < last) {
        const int middle = first + (last - first) / 2;
        if (compare(middle) <= 0) first = middle + 1; else last = middle;
    }
    return {lower, first};
}
}
#endif

#ifndef SIRKIT_SUFFIX_ARRAY_DEFINED
#define SIRKIT_SUFFIX_ARRAY_DEFINED
inline std::vector<int> suffix_array(const std::string& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(s);
}
template<class View> requires std::same_as<View, std::string_view>
std::vector<int> suffix_array(View s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(s);
}
template<class Char> requires (std::same_as<Char, char> || std::same_as<Char, const char>)
std::vector<int> suffix_array(Char* s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_string(std::string_view(s));
}

inline std::vector<int> suffix_array(const std::vector<int>& s, int upper) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    if (upper < 0 || upper >= std::numeric_limits<int>::max() - 1)
        throw std::length_error("suffix_array: alphabet too large");
    std::vector<int> encoded(s.size() + 1);
    for (std::size_t i = 0; i < s.size(); ++i) {
        encoded[i] = s[i] + 1;
    }
    auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), upper + 1);
    result.erase(result.begin());
    return result;
}

template <class T>
std::vector<int> suffix_array(const std::vector<T>& s) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    // 先頭要素の同値類がn個なら、その順序だけで接尾辞順が決まる。
    if constexpr (std::integral<T>) {
        std::vector<T> alphabet = s;
        std::sort(alphabet.begin(), alphabet.end());
        alphabet.erase(std::unique(alphabet.begin(), alphabet.end()), alphabet.end());
        if (alphabet.size() == s.size()) {
            std::vector<int> result(s.size());
            for (std::size_t i = 0; i < s.size(); ++i)
                result[std::lower_bound(alphabet.begin(), alphabet.end(), s[i]) - alphabet.begin()] = static_cast<int>(i);
            return result;
        }
        std::vector<int> encoded(s.size() + 1);
        for (std::size_t i = 0; i < s.size(); ++i)
            encoded[i] = static_cast<int>(std::lower_bound(alphabet.begin(), alphabet.end(), s[i]) - alphabet.begin()) + 1;
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), static_cast<int>(alphabet.size()));
        result.erase(result.begin());
        return result;
    } else {
        std::vector<int> positions(s.size()), encoded(s.size() + 1);
        for (std::size_t i = 0; i < s.size(); ++i) positions[i] = static_cast<int>(i);
        std::sort(positions.begin(), positions.end(), [&](int a, int b) { return s[a] < s[b]; });
        int alphabet = 0;
        for (std::size_t i = 0; i < positions.size(); ++i) {
            if (i == 0 || s[positions[i - 1]] < s[positions[i]]) ++alphabet;
            encoded[positions[i]] = alphabet;
        }
        if (alphabet == static_cast<int>(s.size())) return positions;
        auto result = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_induced(std::span<const int>(encoded), alphabet);
        result.erase(result.begin());
        return result;
    }
}
#endif

#ifndef SIRKIT_LCP_ARRAY_IMPL_DEFINED
#define SIRKIT_LCP_ARRAY_IMPL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
template <class Sequence>
std::vector<int> suffix_lcp(const Sequence& s, const std::vector<int>& sa) {
    /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_length_check(s.size());
    const int n = static_cast<int>(s.size());
    if (n == 0) return {};
    // 妥当なsaの順列から全要素を設定してから参照する。
    auto rank = std::make_unique_for_overwrite<int[]>(s.size());
    for (int i = 0; i < n; ++i) {
        rank[sa[i]] = i;
    }
    if (n < 2) return {};
    std::vector<int> lcp(static_cast<std::size_t>(n - 1));
    int length = 0;
    for (int i = 0; i < n; ++i) {
        const int order = rank[i];
        if (order + 1 == n) { length = 0; continue; }
        const int j = sa[order + 1];
        while (length < n - i && length < n - j && s[i + length] == s[j + length]) ++length;
        lcp[order] = length;
        if (length != 0) --length;
    }
    return lcp;
}
}
#endif

#ifndef SIRKIT_LCP_ARRAY_DEFINED
#define SIRKIT_LCP_ARRAY_DEFINED
template<class T>
std::vector<int> lcp_array(const std::vector<T>& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
inline std::vector<int> lcp_array(const std::string& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> lcp_array(const Sequence& s, const std::vector<int>& sa) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_lcp(s, sa);
}
#endif

#ifndef SIRKIT_SUFFIX_ARRAY_EQUAL_RANGE_DEFINED
#define SIRKIT_SUFFIX_ARRAY_EQUAL_RANGE_DEFINED
inline std::pair<int, int> SuffixArrayEqualRange(std::string_view s, const std::vector<int>& sa,
                                               std::string_view pattern) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_search(s, sa, pattern, [](char c) { return static_cast<unsigned char>(c); });
}
template <std::integral T>
std::pair<int, int> SuffixArrayEqualRange(const std::vector<T>& s, const std::vector<int>& sa,
                                       const std::vector<T>& pattern) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::suffix_search(s, sa, pattern, [](T value) { return value; });
}
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
#include <string>
int main() {
    std::string s; verify_in >> s;
    const auto sa = suffix_array(s);
    const auto lcp = lcp_array(s, sa);
    const auto n = static_cast<std::uint64_t>(s.size());
    auto answer = n * (n + 1) / 2;
    for (int length : lcp) answer -= static_cast<std::uint64_t>(length);
    verify_out << answer << '\n';
}
