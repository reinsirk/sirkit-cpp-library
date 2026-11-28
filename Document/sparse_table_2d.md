# 二次元スパーステーブル

更新のない表を前処理し、長方形内の最小値・最大値・最大公約数などを `O(1)` で求めます。演算 `op` は二項演算で、結合的かつ可換、さらに同じ値を重ねても値が変わらないことが前提です。

## SparseTable2D

```text
SparseTable2D(const std::vector<std::vector<T>> &grid, Op op);
SparseTable2D(std::size_t height, std::size_t width, std::span<const T> row_major, Op op);
```

各行の長さが同じ配列、または全行を順に連結した `height * width` 要素から構築します。検査有効時は形状の不一致で `std::invalid_argument` を送出します。表に必要な要素数を格納できない場合は `std::length_error` です。

`H` 行 `W` 列で、演算および要素のコピーが定数時間なら、時間は `O(H+1+HW log(H+1) log(W+1))`、領域は `O(1+HW log(H+1) log(W+1))` です。

## prod

```text
T prod(std::size_t top, std::size_t left, std::size_t bottom, std::size_t right) const;
```

半開矩形 `[top,bottom) × [left,right)` の集約値を返します。矩形は非空で全体の範囲内が前提です。検査有効時は違反すると `std::out_of_range` を送出します。`O(1)` 時間です。

## 大きさ

- `height()` は行数を返します。`O(1)` 時間です。
- `width()` は列数を返します。`O(1)` 時間です。
- `empty()` は要素がないかを判定します。`O(1)` 時間です。

## 使用例

```cpp
const std::vector<std::vector<int>> grid{{4, 2, 8}, {6, 5, 1}};
const sirkit::SparseTable2D table(grid, [](int a, int b) { return std::min(a, b); });
assert(table.prod(0, 0, 2, 2) == 2);
```

[完全な使用例](../Examples/sparse_table_2d_example.cpp)
