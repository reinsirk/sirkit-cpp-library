// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/adjugate_matrix.md
//
// API一覧
// MatrixAdjugate(matrix): 正方行列の余因子行列を転置した随伴行列を返す。特異行列にも対応。
// Matrix<T>の係数Tは正確な体。0×0は空行列、1×1は{1}。入力は変更しない。
// n行n列でO(1+n³)時間・O(n²)追加領域。Tの体演算・コピーは各O(1)と数える。
