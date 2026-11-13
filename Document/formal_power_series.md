# 形式的冪級数と多項式

[FormalPowerSeries](../Math/Formal_Power_Series.cpp)は、法998244353の多項式を扱います。係数は定数項から順に並べ、Mintは不要です。「精度n」の操作はx^n以上を捨て、n個の係数を返します。[使用例](../Examples/formal_power_series_example.cpp)を参照してください。

## API

| API | 動作・前提 |
| --- | --- |
| `FormalPowerSeries(n)` / `(vector)` / `{...}` | n個の0 / 整数係数を法へ正規化して構築する。 |
| `size()` / `coefficients()` / `operator[]` / `get(i)` | 係数数 / const参照 / 既存係数 / 範囲外を0とする取得。 |
| `set(i,value)` / `resize(n)` / `prefix(n)` / `trim()` | 既存係数変更 / 0拡張・切詰め / n係数のコピー / 末尾0の除去。 |
| `+` / `-` / `*` / `scale(value)` | 係数の和 / 差 / 積 / 全係数の定数倍。 |
| `derivative()` / `integral()` | 微分 / 積分。積分定数0、積分前の係数数は法未満。 |
| `inverse(n)` / `log(n)` / `exp(n)` | 逆数 / 形式log / 形式exp。定数項は順に非0 / 1 / 0。 |
| `pow(k,n)` / `sqrt(n)` | 非負整数乗 / 平方根のoptional。不可能ならnullopt。 |
| `divmod(g)` / `/` / `%` | 多項式の商と剰余の組 / 商 / 剰余。0除算は常に例外。 |
| `clear_cache()` | このスレッドのNTT根・整数逆数の表を解放する。 |

```cpp
const FormalPowerSeries f{1,2,1}; // 1+2x+x²。
auto inverse = f.inverse(4); // {1,-2,3,-4}を法へ正規化した係数。
auto restored = f.log(4).exp(4); // f.prefix(4)と一致。
auto [quotient,remainder] = (f*f).divmod(f); // fと零多項式。
```

inverse・log・exp・pow・sqrtのnを省略すると、現在の係数数を精度にします。n=0は常に空列です。sqrtは先頭非0次数が偶数、かつ先頭係数が平方剰余なら存在します。先頭係数は2候補の小さい方を選び、精度内で未確定の末尾は0にします。powでは0^0=1です。

和・差は長い方の係数数、積は通常n+m−1個で、片方が空なら空です。商・剰余は末尾0を除いて返します。一方、通常の等値比較は係数数も比較するため、精度の異なる0列は区別します。

## 計算量・上限と所有

線形操作はO(n)、精度操作はM(n)=O(n log n)としてO(M(n))時間・O(n)作業領域です。powは加えてO(log k)がかかります。逆数はNewton反復で精度を倍にし、log・exp・平方根も畳み込みを組み合わせて求めます。

inverse・log・exp・pow・sqrtの精度は2^22以下、積・商剰余のNTT長は2^23以下です。容量・長さ・数学的な0判定は検査省略時も維持します。

係数は各個体が独立に所有します。NTT根と整数逆数の表だけを同一スレッドで再利用し、別スレッドとは共有しません。[同梱NTT](ntt.md)は共通ガードで単独版との併用もでき、[生成器](../Scripts/generate_formal_power_series.py)で同期します。VerifyはLibrary Checkerの形式的冪級数・多項式除算を使います。
