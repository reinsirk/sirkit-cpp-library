// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/matrix_pencil_determinant.md
//
// API一覧
// matrix_pencil_determinant<Mod=998244353>(constant,linear): det(constant+x*linear)の係数を定数項から返す。
// 両入力は同じサイズnの正方行列。負のint64_t係数も指定可。Modはuint32_tの素数。
// 返り値はn+1個のuint32_t。行列式が恒等的に0なら全項0、空行列なら{1}。
// 線形係数行列が特異でも可。形状不一致はinvalid_argument。O(n³+n log Mod+1)時間・O(n²+1)領域。
