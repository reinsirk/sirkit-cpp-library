// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dag_grundy.md
//
// API一覧
// dag_grundy_numbers(moves): 各頂点から選べる手を有向辺とし、DAG上の不偏ゲームのGrundy数を返す。
//   moves[v]は0始まりの移動先の列。動けない頂点は0。閉路があればnullopt。
//   独立した局面を合わせるなら、局面のGrundy数のXORが0のとき手番側が負け。
//   構築・計算O(n+m)時間、追加O(n)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#if !defined(SIRKIT_DISABLE_SIMD) && (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#endif


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
