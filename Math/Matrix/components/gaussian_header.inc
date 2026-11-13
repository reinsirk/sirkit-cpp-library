// SPDX-License-Identifier: CC0-1.0
// 参考（用途・Verify問題）: https://nyaannyaan.github.io/library/matrix/matrix.hpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix.md
//
// API一覧
// 以下の消去は正確な体Tが前提。Mint等は利用側が用意する。整数型・一般の合成数法・浮動小数点は対象外。
// GaussianElimination(a,reduced=false): 値で受け取った行列を行基本変形。元のlvalueは変更せず結果を返す。
// GaussianEliminationResult<T>: matrixはピボット1の階段形、pivot_columns/pivotsは列と正規化前の値、row_swapsは交換数。
// reduced=trueなら各ピボットの上も消去する。rank()はピボット数をO(1)で返す。
// rows=m,cols=nとし消去はO(m*n*min(m,n))時間・O(m*n)所有領域。0行・0列・矩形・特異も可。
// MatrixRank(a): 体上の階数。MatrixDeterminant(a): 正方行列の行列式、0x0はT{1}。消去と同計算量。
// MatrixInverse(a): 正方行列の逆行列をoptionalで返す。特異はnullopt、0x0は空の単位行列。O(n³)時間・O(n²)領域。
// SolveLinearSystem(a,b): a*x=bの一解と核の基底をoptionalで返す。不能ならnullopt、b.size()==a.rows()が前提。
// LinearSystemResult<T>: particularは長さnの一解、kernelは各行が核基底（列数n）、pivot_columnsはピボット列。
// SolveはO(m*(n+1)*(1+min(m,n))+n²)時間・O(m*(n+1)+n²)領域。0行では全空間、0列ではbが零のときだけ空の一解。
// 非零ピボットの逆数が1を作らなければdomain_error。これは係数型が体であること全体を検証するものではない。
// 0判定・非可逆ピボットの保護・内部容量上限はSKIP_RUNTIME_CHECKSでも維持する。
