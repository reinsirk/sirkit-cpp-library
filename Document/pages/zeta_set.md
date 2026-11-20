# 集合のゼータ／メビウス変換

添字は集合をビットで表します。ゼータ変換は部分集合または上位集合の値の総和を求めます。`subset_zeta_transform`は`f[S]=sum_{T⊆S}f[T]`、`superset_zeta_transform`は`f[S]=sum_{T⊇S}f[T]`へ入力配列を直接更新します。対応する`mobius`操作で元へ戻せます。

変換・畳み込みのnは配列長です。疎変換のmは要素数、rは各keyの素因数を重複込みで数えた総数です。

## API

### superset_zeta_transform

```text
template <class T> void superset_zeta_transform(std::vector<T>& f);
```

各集合の値を上位集合の総和へ更新します。

**制約:** 空または2冪長。値型は可換な加算に対応。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### superset_mobius_transform

```text
template <class T> void superset_mobius_transform(std::vector<T>& f);
```

上位集合ゼータ変換を元へ戻します。

**制約:** 空または2冪長。値型は加法逆元を持つ。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### subset_zeta_transform

```text
template <class T> void subset_zeta_transform(std::vector<T>& f);
```

各集合の値を部分集合の総和へ更新します。

**制約:** 空または2冪長。値型は可換な加算に対応。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。

### subset_mobius_transform

```text
template <class T> void subset_mobius_transform(std::vector<T>& f);
```

部分集合ゼータ変換を元へ戻します。

**制約:** 空または2冪長。値型は加法逆元を持つ。

**計算量:** O(1+n log(n+1))時間・O(1)追加領域。


次の例では利用側で`<cstdint>`も読み込みます。

```cpp
std::vector<std::int64_t> a{1,2,3,4};
sirkit::subset_zeta_transform(a);    // {1,3,4,10}。
sirkit::subset_mobius_transform(a);  // {1,2,3,4}。
```

入力は空配列または長さn=2^kの配列です。ゼータ変換には加算、逆変換には減算が必要です。各操作はO(1+n log(n+1))時間・O(1)追加領域です。入力を直接変更するため、値演算が例外を送出した場合には途中の状態が残ることがあります。

`std::vector<bool>`は対象外です。符号付き整数の途中の和・差は型の範囲内とし、符号なし整数は型の幅の剰余として計算します。浮動小数点には丸め誤差があります。入力検査を省いても長さと演算範囲の前提を満たしてください。

このファイルは4つの集合変換を提供します。AND／OR畳み込みが必要なら、変換も同梱する[Bitwise Convolution](../../Math/Zeta/Bitwise_Convolution.cpp)を使えます。

[完全な使用例](../../Examples/zeta_set_example.cpp)・[変換の共通説明](../zeta_mobius.md)
