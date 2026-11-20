# Slope Trick

`SlopeTrick`は、絶対値などを足して作る区分線形凸関数と、その最小値・最小点を管理します。折点は整数座標、傾きの変化量は整数で、初期値はf(x)=0です。[使用例](../Examples/slope_trick_example.cpp)を参照してください。

以下、Kは保持する折点の総数です。Argminはoptional<std::int64_t>のlower・upperを持ちます。

## add_left_hinge

```text
void add_left_hinge(std::int64_t a);
```

max(a-x,0)を現在の関数へ加えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数Kに償却O(log(K+1))。

## add_right_hinge

```text
void add_right_hinge(std::int64_t a);
```

max(x-a,0)を加えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数Kに償却O(log(K+1))。

## add_abs

```text
void add_abs(std::int64_t a);
```

abs(x-a)を加えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数Kに償却O(log(K+1))。

## add_constant

```text
void add_constant(std::int64_t value);
```

定数valueを加えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** O(1)。

## min_value

```text
std::int64_t min_value() const noexcept;
```

最小値を返します。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** O(1)。

## argmin

```text
Argmin argmin() const;
```

最小点の閉区間Argmin{lower,upper}を返します。optionalの下端なしは負の無限大、上端なしは正の無限大です。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** O(1)。

## shift

```text
void shift(std::int64_t a);
void shift(std::int64_t a,std::int64_t b);
```

1引数ではf(x)=旧f(x-a)、2引数ではf(x)=min_{x-b<=y<=x-a}旧f(y)へ置き換えます。

**制約:** 2引数形はa<=b。座標・途中の差と和はint64_tに収まること。

**計算量:** O(1)。

## clear_right

```text
void clear_right() noexcept;
```

f(x)=min_{y<=x}旧f(y)へ置き換えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数KにO(K+1)。

## clear_left

```text
void clear_left() noexcept;
```

f(x)=min_{y>=x}旧f(y)へ置き換えます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数KにO(K+1)。

## eval

```text
std::int64_t eval(std::int64_t x) const;
```

関数を壊さずxでの値を返します。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 折点数KにO(K+1)。

## merge

```text
void merge(const SlopeTrick& other);
```

otherの関数を加えます。otherは保持し、自己mergeもできます。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 自分K個・相手M個にO(K+M log(K+M+1))時間・O(K+M+1)追加領域。

## size

```text
std::size_t size() const noexcept;
```

左右の折点の総数を返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

折点のない定数関数ならtrueを返します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。

## reserve

```text
void reserve(std::size_t left,std::size_t right);
```

左右のヒープに指定容量を予約します。

**制約:** 保存する座標・途中の差と和はint64_tの範囲内。無限端はargminのoptionalで表します。

**計算量:** 再確保時O(K+1)。

## clear

```text
void clear() noexcept;
```

関数をf(x)=0へ戻し、領域を解放します。

**制約:** 共通の前提を満たすこと。

**計算量:** 折点数KにO(K+1)。

## swap

```text
void swap(SlopeTrick& other) noexcept;
friend void swap(SlopeTrick& a,SlopeTrick& b) noexcept;
```

内容と設定を交換します。

**制約:** 共通の前提を満たすこと。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

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

## コンストラクタ

```text
SlopeTrick()=default;
SlopeTrick(const SlopeTrick&)=default;
SlopeTrick(SlopeTrick&& other) noexcept;
```

f(x)=0の関数を構築します。コピーは独立、ムーブ元はf(x)=0です。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** 空の構築・ムーブ構築はO(1)、コピーは保持要素数に比例。

## operator=

```text
SlopeTrick& operator=(const SlopeTrick& other);
SlopeTrick& operator=(SlopeTrick&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。
