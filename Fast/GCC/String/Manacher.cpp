// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: String/Manacher.cpp
// 参考（回文半径の出力形式とVerify先。実装は独自構成）:
// https://judge.yosupo.jp/problem/enumerate_palindromes
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/manacher.md
//
// API一覧
// PalindromeRadii(s)/assign(s): 列の奇数/偶数長回文の半径をO(n)時間・領域で構築。
// odd[i]: 中心iの奇数回文の半径（中心を含む）。even[i]: i-1とiの間の偶数回文の半分の長さ。
// size()/empty(): 元の列長/空かを返す。is_palindrome(l,r): 半開区間[l,r)をO(1)で判定、空区間はtrue。
// longest(): 最長回文の半開区間を返す。同長なら最小の左端、空なら{0,0}。O(n)。
// count(): 非空の連続回文区間の総数をuint64_tで返す。O(n)。
// PalindromicLengths(s): 文字上/文字間の2n-1中心の最長回文長を順に返す。空なら{}、O(n)。
// size()/添字アクセス/同値関係の==が使える列が前提。各比較O(1)、列長はINT_MAX未満。元の列は保持しない。
// 検査省略時もis_palindromeの添字は0<=l<=r<=size()が必要。公開半径配列は変更しないこと。

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_PALINDROME_RADII_DEFINED
#define SIRKIT_PALINDROME_RADII_DEFINED
class PalindromeRadii {
public:
    std::vector<int> odd, even;
    PalindromeRadii() = default;
    template <class Sequence>
    explicit PalindromeRadii(const Sequence& s) { assign(s); }

    template <class Sequence>
    void assign(const Sequence& s) {
        if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("PalindromeRadii: sequence too long");
        const int n = static_cast<int>(s.size());
        std::vector<int> next_odd(s.size()), next_even(s.size());
        int left = 0, right = -1;
        for (int i = 0; i < n; ++i) {
            int radius = i <= right ? std::min(right - i + 1, next_odd[left + (right - i)]) : 1;
            while (radius <= i && radius < n - i && s[i - radius] == s[i + radius]) ++radius;
            next_odd[i] = radius;
            if (i + radius - 1 > right) { left = i - radius + 1; right = i + radius - 1; }
        }
        left = 0; right = -1;
        for (int i = 0; i < n; ++i) {
            int radius = i <= right ? std::min(right - i + 1, next_even[left + (right - i) + 1]) : 0;
            while (radius < i && radius < n - i && s[i - radius - 1] == s[i + radius]) ++radius;
            next_even[i] = radius;
            if (i + radius - 1 > right) { left = i - radius; right = i + radius - 1; }
        }
        odd.swap(next_odd); even.swap(next_even);
    }

    [[nodiscard]] int size() const noexcept { return static_cast<int>(odd.size()); }
    [[nodiscard]] bool empty() const noexcept { return odd.empty(); }
    [[nodiscard]] bool is_palindrome(int l, int r) const {
        const int length = r - l;
        if (length == 0) return true;
        const int center = l + length / 2;
        return length & 1 ? odd[center] >= length / 2 + 1 : even[center] >= length / 2;
    }
    [[nodiscard]] std::pair<int, int> longest() const noexcept {
        std::pair<int, int> best{0, 0};
        const auto consider = [&](int l, int r) {
            if (r - l > best.second - best.first ||
                (r - l == best.second - best.first && l < best.first)) best = {l, r};
        };
        for (int i = 0; i < size(); ++i) {
            consider(i - odd[i] + 1, i + odd[i]);
            consider(i - even[i], i + even[i]);
        }
        return best;
    }
    [[nodiscard]] std::uint64_t count() const noexcept {
        std::uint64_t result = 0;
        for (int radius : odd) result += static_cast<std::uint64_t>(radius);
        for (int radius : even) result += static_cast<std::uint64_t>(radius);
        return result;
    }
};
#endif

#ifndef SIRKIT_PALINDROMIC_LENGTHS_DEFINED
#define SIRKIT_PALINDROMIC_LENGTHS_DEFINED
template <class Sequence>
std::vector<int> PalindromicLengths(const Sequence& s) {
    if (s.size() >= static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("PalindromicLengths: sequence too long");
    const int n = static_cast<int>(s.size());
    if (n == 0) return {};
    std::vector<int> result;
    const auto count = static_cast<std::size_t>(n);
    const auto capacity = result.max_size();
    if (count > capacity / 2 + capacity % 2)
        throw std::length_error("PalindromicLengths: result too long");
    result.resize(count + (count - 1));
    // n<INT_MAXより、2n-1はintと同幅の符号なし型に収まる。
    using Center = unsigned int;
    // centerは文字上と文字間を交互に表す。rightは既知回文の右端の2倍。
    Center middle = 0, right = 0, start = 0;
    const auto centers = static_cast<Center>(result.size());
    // 先頭の同値区間内では回文長が三角列になる。
    if (n > 1 && s[0] == s[1]) {
        int run = 2;
        while (run < n && s[0] == s[run]) ++run;
        const auto prefix_centers = Center{2} * static_cast<Center>(run) - 1;
        for (Center c = 0; c < static_cast<Center>(run); ++c) result[c] = static_cast<int>(c + 1);
        for (Center c = static_cast<Center>(run); c < prefix_centers; ++c) result[c] = static_cast<int>(prefix_centers - c);
        if (run == n) return result;
        middle = static_cast<Center>(run - 1);
        right = prefix_centers - 1;
        // 最初の不一致をまたぐ文字間の中心は0（零初期化済み）。
        start = prefix_centers + 1;
    }
    for (Center center = start; center < centers; ++center) {
        int length = (center & 1) == 0 ? 1 : 0;
        if (center < right) {
            const int available = static_cast<int>(right - center + 1);
            const int reflected = result[middle - (center - middle)];
            // 右端に届かない鏡像は、不一致まで確定している。
            if (reflected < available) { result[center] = reflected; continue; }
            length = available;
        }
        int l = static_cast<int>((center + 1 - static_cast<Center>(length)) / 2) - 1;
        int r = static_cast<int>((center + static_cast<Center>(length) + 1) / 2);
        while (l >= 0 && r < n && s[l] == s[r]) { --l; ++r; length += 2; }
        result[center] = length;
        const auto end = center + static_cast<Center>(length) - 1;
        if (end > right) { middle = center; right = end; }
    }
    return result;
}
#endif
