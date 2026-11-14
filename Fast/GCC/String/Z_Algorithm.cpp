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


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_Z_ALGORITHM_IMPL_DEFINED
#define SIRKIT_Z_ALGORITHM_IMPL_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
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
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
inline std::vector<int> z_algorithm(const std::string& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
}
template<class Sequence>
    requires requires(const Sequence& s) { s.size(); s[0] == s[0]; }
std::vector<int> z_algorithm(const Sequence& s) {
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::z_algorithm_impl(s);
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

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
