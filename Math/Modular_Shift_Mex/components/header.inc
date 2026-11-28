// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/modular_shift_mex.md
// 参考（円環上の連続区間と床関数和による差分）: https://atcoder.jp/contests/abc465/editorial/22561
//
// API一覧
// ModularShiftMex(count,modulus,step,start=0): 空集合で初期化。uint32_tのcount/modulus、負のint64_tのstep/startも可。
// insert(value)/erase(value): [0,modulus)の値を追加/削除。集合が変わったときtrue。重複は保持しない。
// sum(): k=0..count-1で、集合の全要素をstart+step*kだけ法modulusでずらしたmexの総和。uint64_tで厳密。
// size()/clear(): 要素数O(1)/集合を空にするO(r+1)。移動元は同じ設定の空集合。自己代入は内容を保つ。
// 一直線に並べたときの占有区間数をrとして、追加削除はO(log(r+2)+log(modulus+1))時間・合計O(r+1)領域。
// 構築とsumはO(1)、コピーはO(r+1)。法0はinvalid_argument、範囲外の値はout_of_range。
