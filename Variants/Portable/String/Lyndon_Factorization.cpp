// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: String/Lyndon_Factorization.cpp
// 参考（Duval法）: https://doi.org/10.1016/0196-6774(83)90017-2
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lyndon_factorization.md
//
// API一覧
// lyndon_factorization(text): Lyndon語への辞書順非増加な分解。各区間の境界を昇順で返す。
// 文字はunsigned charとして比較。空文字でも{0}を返す。O(N)時間・出力以外O(1)領域。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <string_view>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LYNDON_FACTORIZATION_DEFINED
#define SIRKIT_LYNDON_FACTORIZATION_DEFINED

inline std::vector<std::size_t> lyndon_factorization(std::string_view text)
{
    std::vector<std::size_t> boundaries{0};
    std::size_t begin = 0;
    while (begin < text.size())
    {
        std::size_t scan = begin + 1;
        std::size_t compare = begin;
        // 同じ Lyndon 語の繰り返しとして読める最長接頭辞を探す。
        for (; scan < text.size(); ++scan)
        {
            const auto current = static_cast<unsigned char>(text[compare]);
            const auto next = static_cast<unsigned char>(text[scan]);
            if (current > next)
            {
                break;
            }
            compare = current < next ? begin : compare + 1;
        }
        const auto period = scan - compare;
        // scan までに読み切れた因子を一括で出す。
        do
        {
            begin += period;
            boundaries.push_back(begin);
        } while (begin <= compare);
    }
    return boundaries;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
