// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/big_integer.md
//
// API一覧
// [BigInteger<Radix=10>]
// Radixは10か16。BigInteger(value=0): int64_tから構築。BigInteger(text): 符号付きの文字列から構築。
// to_string(): 先頭ゼロのない文字列。16進の英字は大文字。sign(): -1、0、1。
// +,-,*,+=,-=,*=、比較、単項-: 符号付きの整数演算。桁数Dについて比較・加減・変換はO(D)時間。
// 乗算は内部の4桁ブロック数A,BについてO(A*B)またはO((A+B)log(A+B))時間、O(A+B)領域。
// NTTが必要な場合の変換長は2^23以下。整数係数を2種類のNTTから復元する。
// divmod(divisor): 正のuint32_tで割った商とint64_tの余り。商は0へ丸める。O(D)時間・領域。
// divmod(BigInteger)、/,%,/=,%=: 非零のBigIntegerで割る。商は0へ丸め、余りは元の整数と同符号か0。
// 4桁ブロック数を被除数A、除数Bとして、除算はO(A log^2(B+1))時間・O(A+B)領域。NTTの制約は乗算と共通。
