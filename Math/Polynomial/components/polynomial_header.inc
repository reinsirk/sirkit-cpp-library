// SPDX-License-Identifier: CC0-1.0
// 参考（積木による多点評価・補間の数式とVerify問題。所有構造・評価再帰は独自実装）:
// https://nyaannyaan.github.io/library/fps/multipoint-evaluation.hpp.html
// https://judge.yosupo.jp/problem/polynomial_interpolation
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/polynomial.md
//
// API一覧
// ProductTree(): 空の点列。ProductTree(points)/assign(points): 整数vectorを998244353へ正規化し積木を構築。
// 構築O(1+M(n)log(n+1))、保持O(n log(n+1))、M(n)=O(n log(n+1))。n<=max_points()=2^22-1。
// size()/empty()/points(): 点数/空か/正規化した点列のconst参照。O(1)。コピーは独立、別オブジェクトへの移動元は空。assign/コピー代入は失敗時に元を保つ。
// swap(other): 点列と積木をO(1)で交換する。
// polynomial(): 積Π(x-point)のFormalPowerSeriesをコピーして返す。空点列では{1}。O(n)。
// evaluate(poly): FormalPowerSeriesまたは整数係数vectorを各点で評価しvector<uint32_t>を返す。重複点も可。
// evaluateは係数数d<=2^22、O(1+M(max(n,d))log(n+2)+n+d)、追加O(n+d)。固定点列で積木を再利用する。
// interpolate(values): 各点の整数値vectorから次数n未満のFormalPowerSeriesを返す。出力はn係数（末尾0を保持）。
// 補間には法で相異なる点が必要。重複は数式上の分母0として検査省略時もdomain_error。O(1+M(n)log(n+1))、追加O(n log(n+1))。
// MultipointEvaluate(poly,points)/PolynomialInterpolate(points,values): 一時積木を使う簡易版。
// TaylorShift(poly,shift): f(x+shift)のFormalPowerSeries。係数数は入力と同じ。O(1+M(d))時間・O(d)追加、d<=2^22。
// TaylorShiftはFormalPowerSeries/整数vectorに対応。全て法998244353、Mint不要。容量/数学的0判定は検査省略時も維持。
