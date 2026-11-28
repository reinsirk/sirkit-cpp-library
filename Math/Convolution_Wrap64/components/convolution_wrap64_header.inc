// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/convolution_wrap64.md
//
// API一覧
// convolution_mod_2_64(a,b): uint64_t列の畳み込みを各係数mod 2^64で返す。
// 空の列が片方でもあれば空。出力係数数N=a.size()+b.size()-1は2^21以下。
// O(N log(N+1))時間・O(N)領域。短い列は筆算、それ以外は5種類のNTTとCRT。
