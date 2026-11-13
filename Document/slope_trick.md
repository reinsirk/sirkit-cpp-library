# Slope Trick

`SlopeTrick`は、絶対値などを足して作る区分線形凸関数と、その最小値・最小点を管理します。折点は整数座標、傾きの変化量は整数で、初期値はf(x)=0です。[使用例](../Examples/slope_trick_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `add_left_hinge(a)` / `add_right_hinge(a)` | max(a-x,0) / max(x-a,0)を足す。 |
| `add_abs(a)` / `add_constant(c)` | abs(x-a) / 定数cを足す。 |
| `min_value()` / `argmin()` | 最小値 / 最小点の閉区間Argmin{lower,upper}を返す。O(1)。 |
| `shift(a)` | 新f(x)=旧f(x-a)。グラフを右へa移す。O(1)。 |
| `shift(a,b)` | 新f(x)=min_{x-b<=y<=x-a}旧f(y)。a<=bが前提。O(1)。 |
| `clear_right()` / `clear_left()` | min_{y<=x}旧f(y) / min_{y>=x}旧f(y)へ置き換える。 |
| `eval(x)` | 関数を壊さずxでの値を返す。O(K+1)。 |
| `merge(other)` | otherの関数を加える。otherを保ち、自己mergeも可能。 |
| `size()` / `empty()` | 折点数 / 折点のない定数関数か。O(1)。 |
| `reserve(left,right)` / `clear()` / `swap(other)` | 左右の容量予約 / f=0へ戻して解放 / O(1)で交換する。 |

```cpp
SlopeTrick f;
f.add_abs(2); f.add_abs(8); f.add_constant(3);
auto interval = f.argmin(); // lower=2、upper=8。
auto minimum = f.min_value(); // 9。
auto value = f.eval(5); // 9。
f.shift(-1,2); // 最小点の区間を左へ1、右へ2広げる。
```

argminの両端はoptional<int64_t>です。lowerなしは負の無限大、upperなしは正の無限大を表し、INT64_MIN/MAXを番兵にしません。関数は最小値が有限なものを扱います。

## 計算量と数値の前提

Kは折点数です。折点追加は償却O(log(K+1))、定数追加はO(1)、領域はO(K+1)。重みwの絶対値はw回加えます。clear・片側の削除・破棄はO(K+1)、reserveは再確保時O(K+1)です。

mergeは自分のコピーへ加算してから交換します。自分K個・相手M個としてO(K+M log(K+M+1))時間・O(K+M+1)追加領域で、小さい側だけを破壊的に挿入する計算量ではありません。

座標・平行移動量・保存する座標差・途中の差と和はint64_tの範囲に収めます。範囲外はoverflow_errorで、最終結果が相殺で収まっても途中の差が範囲外なら例外です。shiftは全折点を調べないため、範囲外になった実座標は後で参照した時に例外となる場合があります。

コピーは独立し、コピー代入は強い例外保証を持ちます。移動はnoexcept・O(1)、移動元はf=0です。全操作は確保・整数範囲の失敗時に関数を保ちますが、容量だけ変わる場合があります。

GCC/Clangの128ビット整数拡張を使える場合は範囲判定に使用し、`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準C++へ切り替えます。`SKIP_RUNTIME_CHECKS`が除くのはshift(a,b)のa<=b検査だけで、容量・整数範囲の保護は維持します。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/data-structure/slope-trick.hpp.html)と[Luzhiled](https://ei1333.github.io/library/structure/others/slope-trick.hpp.html)です。実装は独立に組み立てています。
