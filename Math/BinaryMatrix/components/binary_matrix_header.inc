// SPDX-License-Identifier: CC0-1.0
// 参考: https://nyaannyaan.github.io/library/matrix/f2-matrix.hpp
// GF(2)の掃き出し法・機能選定を参考に独立実装。内部bit kernelはDynamicBitsetと同一guardで共有。
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/binary_matrix.md
// API一覧
// BinaryMatrix(rows=0,cols=0): 全0行列。O(rows*ceil(cols/64))時間/領域。GF(2)上の要素0/1を扱う。
// rows()/cols()/empty(): 行数/列数/要素が0個か。row_words(row): 列0を最下位bitとするconst word span。
// test(row,col)/operator()(row,col): bool取得。set(row,col,value=true)/flip(row,col): 要素変更、*thisを返す。
// swap_rows(a,b), xor_rows(target,source,first_col=0): 行交換/first_col以降だけの行XOR。自己XOR可。
// transpose(): 転置。multiply(other)/operator*: 積。operator*=は自己参照可。pow(uint64_t): 正方行列の累乗。
// identity(n): 単位行列。+= / + / ^= / ^は同形行列のGF(2)和。==は形と全要素の一致。
// rank(): 階数。determinant(): 正方行列の行列式bool。0x0の行列式は1。
// inverse(): optional<BinaryMatrix>、非正則ならnullopt。0x0は0x0単位行列を返す。
// solve(span<const uint8_t> rhs): Ax=rhsのoptional<LinearSolution>、不能ならnullopt。rhsは行数個の0/1。
// LinearSolution {particular,kernel,pivot_columns}: particularは列数個のuint8_t特解、
//   kernelは各行が核の基底となる(列数-rank)x列数の行列。任意解は特解と基底行の線形結合。
//   pivot_columnsはRREFのpivot列。0行/0列も可。0行では全変数が自由、0列ではrhsが全0の場合のみ可。
// reset(rows=0,cols=0)/clear(): 再初期化/0x0。copyは独立所有・代入強保証、move元は0x0で再利用可。
// swapはO(1)。set/test/flip/形照会はO(1)、行操作はO(ceil(cols/64))、転置O(rows*cols)。
// AxBの積はO(1+B.rows*B.cols + A.rows*ceil(B.cols/64) + A.rows*B.cols*ceil(A.cols/64))。
// 累乗は行列積O(log(k+1))回。内側0次元でも結果の全0領域は確保する。
// rank/inverseの消去は最悪O(1+rows*cols + rows*min(rows,cols)*ceil(cols/64))時間。元行列を変更しない。
// solveは上記にO(rows+cols+cols*(cols-rank))時間を追加、核は(cols-rank)*ceil(cols/64)語。
// LinearSolutionも独立copy/代入強保証、move元の全フィールドは空。swapはO(1)。
// 確保失敗時は元の内容/viewを維持。viewは成功したreset/clear/代入/所有権移動まで有効。
// 番号・形・rhs長/参照したrhsの0/1検査はSKIP_RUNTIME_CHECKSで除外。容量保護・数学的0判定は維持。
// 0列のsolveは非0のrhsを見つけた時点で終了し、それ以降のrhsは検査しない。
// SIRKIT_DISABLE_SIMDは共有bit kernelの追加CPU判定/専用実装を除く。消去/転置に独自SIMDは使わない。
