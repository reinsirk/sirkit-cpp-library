// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjacent_swap_distance.md
//
// API一覧
// adjacent_swap_permutation(from,to,compare={}): from[i]に対応するtoの位置を添字iの順に返す。
// 同じ値の出現は先頭から順に対応させる。添字は0始まり。
// adjacent_swap_distance(from,to,compare={}): 隣接する2要素の交換でfromをtoへ変える最小回数。
// 変換できない場合はいずれもnullopt。compareで互いに小さくない値を同値と扱う。
// std::size・添字アクセスに対応する列で使用可。長さnはUINT32_MAX以下。O((n+1)log(n+2))時間・O(n+1)領域。
