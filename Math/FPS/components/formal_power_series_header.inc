// SPDX-License-Identifier: CC0-1.0
// 参考（形式的冪級数の用途・Verify問題。Newton反復と所有構造は独自実装）:
// https://nyaannyaan.github.io/library/fps/ntt-friendly-fps.hpp.html
// https://nyaannyaan.github.io/library/ntt/ntt.hpp.html
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/formal_power_series.md
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/ntt.md
//
// API一覧
// FormalPowerSeries: 法998244353、Mint不要の形式的冪級数/多項式。係数はuint32_tの独立所有vector。
// ()/size_t n/vector<整数>/initializer_list<int64_t>: 空/n個の0/法へ正規化した係数で生成。コピーは独立。
// modulus()/max_precision(): 法/精度操作の上限をO(1)で返す。
// size()/empty()/coefficients(): 係数数/空か/係数vectorのconst参照。operator[](i)/set(i,value): 既存添字の読取/正規化した代入。
// get(i): 範囲外は0。resize(n): 末尾を0で拡張/切詰め。prefix(n): n係数へ0拡張/切詰めしたコピー。
// trim(): 末尾の0を除く。空vectorは零多項式。長さを含めて等値比較するため、精度の異なる0列は区別する。
// +,-,*,/,%, +=,-=,*=,/=,%=: 係数和/差/積/多項式商/剰余。和・差は長い方の長さ、積は空を含めば空。
// scale(value): 全係数を整数倍。derivative()/integral(): 微分/積分（積分定数0）。積分は係数数<998244353が必要。
// inverse(n)/log(n)/exp(n): x^n未満で逆数/形式log/形式exp。inverseは定数項!=0、logは=1、expは=0。
// pow(k,n): 非負uint64_t乗をx^nで切る。0^0=1、先頭0と大きいkを扱う。sqrt(n): 平方根かnullopt。
// sqrtは先頭非0次数が偶数かつ定数部分が平方剰余なら存在。根の先頭非0係数は2候補の小さい方、未確定末尾は0。
// inverse/log/exp/pow/sqrtの引数n省略は現在のsize()。n=0は常に空、精度nは2^22以下。
// divmod(divisor): 末尾0を除いた商/剰余のpair。0除算は検査省略時もdomain_error。
// 線形操作はO(n)。精度操作はM(n)=O(n log n)としてO(M(n))、冪乗は加えてO(log k)。作業領域O(n)。
// 積と商剰余のNTT長は2^23以下。短い積/除数では直接計算。長さ・数学的0判定は検査省略時も維持。
// clear_cache(): このスレッドのNTT根/整数逆数cacheを解放。cacheは同一スレッドで共有し、係数は共有しない。

