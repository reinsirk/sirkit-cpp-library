// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/set_series_composition.md
//
// API一覧
// set_series_compose(polynomial,series): 一変数多項式に集合冪級数を代入し、全ての集合について係数を返す。
// set_series_power_projection(series,weights,count): k=0..count-1のseries^kとweightsの内積を返す。
// seriesは2^n係数、0<=n<=20。weightsはseriesと同じ長さ。法998244353。
// Mは多項式の係数数またはcount。各O((n+1)^2*2^n+(n+1)*M)時間、O((n+1)*2^n+M)領域。
