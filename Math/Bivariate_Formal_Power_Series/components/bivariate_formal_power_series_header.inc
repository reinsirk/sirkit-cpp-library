// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/bivariate_formal_power_series.md
//
// API一覧
// bivariate_fps_inverse(coefficients,rows,columns): 法998244353で2変数FPSの逆数を返す。
// x^iy^jの係数をi*columns+jに置く。入力・出力はrows*columns要素、積は2^21以下。
// 逆数はx^rowsとy^columnsで切り、定数項は非零。空の軸を含む場合の答えは空。
// N=rows*columnsとしてO(N log(N+1))時間・O(N)領域。
