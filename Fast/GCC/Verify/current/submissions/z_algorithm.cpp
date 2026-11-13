#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://judge.yosupo.jp/problem/zalgorithm

// Library: String/Z_Algorithm.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Z_Algorithm.cpp
// 参考（Z配列の用途とVerify先。実装は独自構成）:
// https://judge.yosupo.jp/problem/zalgorithm
// 参考（ACLのAPI）: https://atcoder.github.io/ac-library/production/document_ja/string.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/z_algorithm.md
//
// API一覧
// z_algorithm(const std::string&)/z_algorithm<T>(const std::vector<T>&): ACL形式のZ配列を返す。
// 追加のz_algorithm(sequence)は任意の添字アクセス可能な列に対応。空なら{}、非空ならz[0]=n。
// PrefixMatches(pattern,text): textの各開始位置とpatternの共通接頭辞長を返す。
// ZMatches(pattern,text): 完全一致する開始位置を昇順に返す。空patternは0..text.size()。
// size()/添字アクセスと同値関係の==が使える列が前提。比較O(1)ならZはO(n)、後2者はO(n+m)時間。
// 各列長はINT_MAX未満。Z/PrefixMatchesの領域は返却値とpattern長、ZMatchesはこれに一致数。

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef SIRKIT_Z_ALGORITHM_IMPL_DEFINED
#define SIRKIT_Z_ALGORITHM_IMPL_DEFINED
namespace reinsirk::detail {
template <class Sequence>
std::vector<int> z_algorithm_impl(const Sequence& s) {
    if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("z_algorithm: sequence too long");
    const int n = static_cast<int>(s.size());
    std::vector<int> z(s.size());
    if (n == 0) return z;
    z[0] = n;
    int i = 1, length = 0;
    while (i < n) {
        if (length == 0) {
            // 未処理位置のz[i]は零初期化済み。不一致なら書き直さない。
            if (!static_cast<bool>(s[0] == s[i])) { ++i; continue; }
            length = 1;
        }
        while (length < n - i && s[length] == s[i + length]) ++length;
        z[i] = length;
        // 末尾まで一致した箱では、残りも確定済みの値だけで埋められる。
        if (length == n - i) {
            if (i == 1) {
                for (++i; i < n; ++i) z[i] = n - i;
            } else {
                const int left = i;
                for (++i; i < n; ++i) z[i] = std::min(n - i, z[i - left]);
            }
            break;
        }
        // 箱内で不一致まで確定する位置を写し、次の位置へ一致長を持ち越す。
        int advance = 1;
        while (advance < length && z[advance] < length - advance) {
            z[i + advance] = z[advance];
            ++advance;
        }
        i += advance;
        length = std::max(0, length - advance);
    }
    return z;
}
}
#endif

#ifndef SIRKIT_Z_ALGORITHM_DEFINED
#define SIRKIT_Z_ALGORITHM_DEFINED
template<class T>
std::vector<int> z_algorithm(const std::vector<T>& s) {
    return reinsirk::detail::z_algorithm_impl(s);
}
inline std::vector<int> z_algorithm(const std::string& s) {
    return reinsirk::detail::z_algorithm_impl(s);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> z_algorithm(const Sequence& s) {
    return reinsirk::detail::z_algorithm_impl(s);
}
#endif

#ifndef SIRKIT_Z_MATCHES_DEFINED
#define SIRKIT_Z_MATCHES_DEFINED
template <class Pattern, class Text>
std::vector<int> PrefixMatches(const Pattern& pattern, const Text& text) {
    if (text.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("PrefixMatches: text too long");
    const auto z = z_algorithm(pattern);
    const int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
    std::vector<int> matches(text.size());
    int left = 0, right = 0;
    for (int i = 0; i < n; ++i) {
        int length = i < right ? std::min(right - i, z[i - left]) : 0;
        while (length < m && length < n - i && pattern[length] == text[i + length]) ++length;
        matches[i] = length;
        if (i + length > right) { left = i; right = i + length; }
    }
    return matches;
}

template <class Pattern, class Text>
std::vector<int> ZMatches(const Pattern& pattern, const Text& text) {
    auto lengths = PrefixMatches(pattern, text);
    std::vector<int> positions;
    const int m = static_cast<int>(pattern.size());
    for (int i = 0; i < static_cast<int>(lengths.size()); ++i)
        if (lengths[i] == m) positions.push_back(i);
    if (m == 0) positions.push_back(static_cast<int>(text.size()));
    return positions;
}
#endif


// SPDX-License-Identifier: CC0-1.0
// zalgorithm用の入出力。stdinを一括で読み、空白類(<= ' ')を飛ばした最初の語をSとする。
// Z配列の値は0..nで非負。0..9は直接、10..9999999は8バイト単位で十進化し、他はto_charsで書く。
// SIRKIT_DISABLE_SIMD・非ASCII・8ビット以外のchar・非リトルエンディアンでは標準の要素処理を使う。
#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace reinsirk::detail {
// stdinを末尾まで読み、所有する領域と読んだバイト数を返す。容量は不足時に倍にする。
inline std::pair<std::unique_ptr<char[]>, std::size_t> verify_read_stdin() {
    std::size_t capacity = std::size_t{1} << 20, size = 0;
    auto data = std::make_unique_for_overwrite<char[]>(capacity);
    while ((size += std::fread(data.get() + size, 1, capacity - size, stdin)) == capacity) {
        if (capacity > static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / 2)
            throw std::length_error("verify input is too large");
        auto grown = std::make_unique_for_overwrite<char[]>(capacity * 2);
        std::memcpy(grown.get(), data.get(), size);
        data = std::move(grown);
        capacity *= 2;
    }
    if (std::ferror(stdin)) throw std::runtime_error("failed to read verify input");
    return {std::move(data), size};
}

// [first,last)で最初の空白類(<= ' ')の位置を返す。なければlast。
inline const char* verify_find_space(const char* first, const char* last) {
#ifndef SIRKIT_DISABLE_SIMD
    if constexpr (std::numeric_limits<unsigned char>::digits == 8
                  && ' ' == 0x20 && '0' == 0x30
                  && std::endian::native == std::endian::little) {
        constexpr std::uint64_t ones = 0x0101010101010101, highs = 0x8080808080808080;
        // 0x21未満のバイトに印を付ける。最下位の印は下位からの借りの影響を受けない。
        for (; last - first >= 8; first += 8) {
            std::uint64_t word;
            std::memcpy(&word, first, sizeof(word));
            const std::uint64_t hit = (word - ones * 0x21) & ~word & highs;
            if (hit != 0) return first + std::countr_zero(hit) / 8;
        }
    }
#endif
    while (first != last && static_cast<unsigned char>(*first) > ' ') ++first;
    return first;
}

// valueを十進で書いて空白を続け、次の書き込み位置を返す。pから最大11バイトを使う。
inline char* verify_write_with_space(char* p, std::uint32_t value) {
    if (value < 10) {
        p[0] = static_cast<char>('0' + value);
        p[1] = ' ';
        return p + 2;
    }
// Clangでは標準の整数変換が速いため、語単位の十進化を使わない。
#if !defined(SIRKIT_DISABLE_SIMD) && !defined(__clang__)
    if constexpr (std::numeric_limits<unsigned char>::digits == 8
                  && ' ' == 0x20 && '0' == 0x30
                  && std::endian::native == std::endian::little) {
        if (value < 10'000'000) {
            // 8桁を上位から1バイトずつ並べる。4桁→2桁→1桁へ、除算を乗算とシフトで分ける。
            constexpr std::uint64_t mask100 = 0x0000007F0000007F, mask10 = 0x000F000F000F000F;
            constexpr std::uint64_t zeros = 0x3030303030303030;
            std::uint64_t digits = (value / 10000) | (std::uint64_t{value % 10000} << 32);
            std::uint64_t q = ((digits * 10486) >> 20) & mask100;
            digits = q | ((digits - q * 100) << 16);
            q = ((digits * 103) >> 10) & mask10;
            digits = q | ((digits - q * 10) << 8);
            // 2〜7桁なので、除く先頭の0はskip/8個（8 <= skip <= 48）。
            const int skip = std::countr_zero(digits) & ~7;
            const std::uint64_t text = ((digits | zeros) >> skip) | (std::uint64_t{' '} << (64 - skip));
            std::memcpy(p, &text, sizeof(text));
            return p + (9 - skip / 8);
        }
    }
#endif
    p = std::to_chars(p, p + 10, value).ptr;
    *p = ' ';
    return p + 1;
}
}

int main() {
    const auto [input, input_size] = reinsirk::detail::verify_read_stdin();
    const char* first = input.get();
    const char* const last = first + input_size;
    while (first != last && static_cast<unsigned char>(*first) <= ' ') ++first;
    const char* const end = reinsirk::detail::verify_find_space(first, last);
    const std::string_view s(first, static_cast<std::size_t>(end - first));
    const auto z = z_algorithm(s);

    constexpr std::size_t chunk = std::size_t{1} << 17;
    static char buffer[chunk + 16];
    char* p = buffer;
    for (const int value : z) {
        // 書く前に空きを確かめるので、最後の値と空白は必ずbuffer内に残る。
        if (static_cast<std::size_t>(p - buffer) >= chunk) {
            const auto size = static_cast<std::size_t>(p - buffer);
            if (std::fwrite(buffer, 1, size, stdout) != size) return 1;
            p = buffer;
        }
        p = reinsirk::detail::verify_write_with_space(p, static_cast<std::uint32_t>(value));
    }
    if (p == buffer) *p++ = '\n';
    else p[-1] = '\n';
    const auto size = static_cast<std::size_t>(p - buffer);
    if (std::fwrite(buffer, 1, size, stdout) != size) return 1;
    return std::fflush(stdout) == 0 ? 0 : 1;
}
