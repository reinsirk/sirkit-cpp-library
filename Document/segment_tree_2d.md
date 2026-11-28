# 二次元セグメント木

表の一点を更新し、長方形の和・最小値・最大値などを取得します。`op` には結合的で可換な演算を、`identity` にはその単位元を渡します。演算と要素のコピーは定数時間とします。

## SegmentTree2D

```text
SegmentTree2D(std::size_t height, std::size_t width, T identity, Op op);
SegmentTree2D(std::size_t height, std::size_t width, std::span<const T> row_major, T identity, Op op);
```

単位元の表、または行を順に並べた `height * width` 要素から構築します。検査有効時は要素数が寸法と合わなければ `std::invalid_argument` を送出します。必要な領域を表せない場合は `std::length_error` です。`H` 行 `W` 列で時間・領域ともに `O(HW+1)` です。

## set

```text
void set(std::size_t row, std::size_t col, T value);
```

範囲内の一点を代入します。更新中の演算と代入は例外を送出しないことが前提です。`O(log(H+1) log(W+1))` 時間です。

## get

```text
T get(std::size_t row, std::size_t col) const;
```

範囲内の一点の値を返します。`O(1)` 時間です。

## prod

```text
T prod(std::size_t top, std::size_t left, std::size_t bottom, std::size_t right) const;
```

半開矩形 `[top,bottom) × [left,right)` を集約します。空矩形には単位元を返します。`O(log(H+1) log(W+1))` 時間です。

## all_prod

```text
T all_prod() const;
```

全要素を集約します。表が空なら単位元を返します。`O(1)` 時間です。

`height()`、`width()`、`empty()` は行数、列数、要素がないかを返します。各 `O(1)` です。検査有効時は範囲外の点や矩形に `std::out_of_range` を送出します。

## 使用例

```cpp
sirkit::SegmentTree2D table(3, 4, 0, [](int a, int b) noexcept { return a + b; });
table.set(1, 2, 5);
table.set(2, 3, 7);
assert(table.prod(0, 0, 2, 3) == 5);
assert(table.all_prod() == 12);
```

[完全な使用例](../Examples/segment_tree_2d_example.cpp)
