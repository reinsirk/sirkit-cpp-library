// SPDX-License-Identifier: CC0-1.0
// 参考: https://atcoder.jp/contests/awc0100/editorial/21308
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/persistent_weighted_union_find.md
//
// API一覧
// PersistentWeightedUnionFind<T,Add,Subtract,Shift>(n,zero=T{},add={},subtract={}): n頂点を独立に構築。
// 版を省略する操作は現在版を使う。Versionは非所有の版ハンドル。初期版はnullptr。version()/restore(v)はO(1)。
// root(i[,v]): 代表の頂点番号。size(i[,v]): 成分サイズ。same/issame(a,b[,v]): 同一成分か。
// root_with_weight(i[,v]): {代表, iと代表の差}。try_diff(a,b[,v]): bとaの差。別成分ならnullopt。
// unite_checked(a,b,d[,v]): bとaの差をdに制約し、Merged/AlreadyConsistent/Contradictionを返す。
// unite_checkedの後は、結合時は新しい版、それ以外は操作元の版が現在版になる。過去の版は保持される。
// B=2^Shift、L=1+log_B(n+1)。構築はO(B+L)。操作はO(log(n+1)L)、結合時にO(BL)の償却時間・追加領域。
// Tはコピー構築・代入可能な可換加法群。既定の符号付き整数の演算は中間値の桁あふれで例外。符号なし整数は型の幅で剰余。
// 版は同じ保存領域を所有するコピーが残る間有効。コピー同士の現在版は独立。移動元は空。
