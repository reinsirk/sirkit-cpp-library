// SPDX-License-Identifier: CC0-1.0
// 参考（木上のMo法）: https://atcoder.jp/contests/abc477/editorial/25886?lang=ja
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_mo.md
//
// API一覧
// TreeMo(n,edges,root=0): 頂点0..n-1の無向木。各辺を入力順に1回ずつ渡す。
// size(): 頂点数、O(1)。構築時間・領域はB=ceil(n/32)としてO(n+B log(B+1))。
// run(queries,add,erase,answer,block_size=0): (始点,終点)列の各パス上の頂点をMo順に集計。
// run_edges(...): 辺を集計。addとeraseの引数は入力順の辺番号。頂点版では頂点番号。
// answer(照会番号)は集計がパスに一致したとき呼ぶ。外部集計は開始時に空、正常終了時も空。
// 各パス上の順序は保持しない。n頂点q照会で、幅bなら入れ替えO(qb+n(n/b+1)+q)回。
// ほかに照会の整列O(q log(q+1))時間と作業領域O(n+q)。幅0で自動。
// callback中に本体を変更しない。callbackの例外では外部集計を巻き戻さない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
