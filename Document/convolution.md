# 任意法・整数畳み込み

[CRTConvolution](../Math/Convolution.cpp)は、整数列a,bの畳み込み`c[k]=sum(a[i]*b[k-i])`を求めます。任意の32ビット正整数法、または厳密なint64_tの結果を扱い、Mintは不要です。入力は変更せず、片方が空なら空列を返します。[使用例](../Examples/convolution_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `modular(a,b,mod)` | 各入力を法で正規化し、畳み込みをvector<uint32_t>で返す。1<=mod<=UINT32_MAX。 |
| `modular_square(a,mod)` | aとaの畳み込み。大きい入力では順変換を1回にする。 |
| `integer(a,b)` / `integer_square(a)` | int64_t列の厳密な畳み込み / 自己畳み込みをvector<int64_t>で返す。 |
| `prepare(length)` | 0または対応する2冪長の根を準備する。 |
| `max_length()` / `roots_capacity()` / `clear_cache()` | 結果長上限2^23 / 保持する根の総数 / 根を解放する。 |
| `ConvolutionAnyMod(a,b,mod)` / `ConvolutionInt64(a,b)` | 一時オブジェクトを使う簡易版。 |

```cpp
CRTConvolution engine;
const std::vector<int> a{1,2}, b{3,4};
auto modular = engine.modular(a,b,7); // {3,3,1}。
const std::vector<std::int64_t> x{-1,2};
auto exact = engine.integer_square(x); // {1,-4,4}。
```

modularは符号付き・符号なし64ビット以下の整数入力を受け付け、INT64_MIN・UINT64_MAXも正規化できます。法1は全0、法0は常に例外です。**integerでは全ての最終係数がint64_tに収まることを利用側で保証します。** 中間積・部分和が大きくても、最終的に相殺される場合を扱います。

## 方式と計算量

3つの素数を法としてNTTを行い、中国剰余定理で結果を復元します。任意法版では正規化した係数と結果長の上限により一意性を確保します。指定法が同梱NTTの素数なら、そのNTTだけを使います。

短い方が32要素以下ならO(nm)、それ以外はL=bit_ceil(n+m−1)としてO(L log L)時間・O(L)追加領域です。結果長は2^23以下です。同じengineを再利用すると根の表も再利用でき、コピーの表は独立です。prepareの新規準備・保持量はO(length)、clear_cacheは保持数に比例します。

厳密整数版は、復元値がint64_tに収まらなければ検査省略時も例外です。ただし前提に違反する真の係数が3素数の積を超えると、小さな別の整数に合同となる場合があり、**値域違反を全て検出する保証はありません**。

GCC/Clangの128ビット整数拡張があれば厳密復元に使います。拡張型は標準C++の型ではありません。`SIRKIT_CONVOLUTION_DISABLE_INT128`または`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準の64ビット2語の代替経路を選べます。同梱NTTの追加SIMD・CPU判定は`SIRKIT_DISABLE_SIMD`で除外できます。容量・長さ・法0の保護は常に維持します。

Verifyは[Convolution (Mod 1,000,000,007)](https://judge.yosupo.jp/problem/convolution_mod_1000000007)と[AtCoder ABC392 G](https://atcoder.jp/contests/abc392/tasks/abc392_g)。数学的参考は[中国剰余定理](https://info.atcoder.jp/entry/algorithm_lectures/chinese_remainder_theorem)です。負数・相殺・極値・所有権は独立ローカル参照で補います。
