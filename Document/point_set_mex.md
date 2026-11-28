# 一点更新できるmex

整数列の一要素を書き換えながら、列にない最小の非負整数を求めます。たとえば `{0, 0, 2}` のmexは1です。

## `PointSetMex`

```cpp
PointSetMex();
explicit PointSetMex(std::vector<T> values);
```

`T` は64ビット以下の整数型で、既定値は `int` です。引数なしで構築すると空の列になります。初期列を渡したときの構築は、長さを $n$ として $O(n+1)$ 時間・領域です。コピーは独立し、ムーブ元は空の列になります。

## `set`

```cpp
void set(std::size_t i, T value);
```

位置 `i` の値を置き換えます。更新は $O(1+\log_{64}(n+1))$ 時間です。負の値や $n$ より大きい値も保存でき、mexの値には影響しません。

## `get`

```cpp
T get(std::size_t i) const;
```

位置 `i` の現在値を $O(1)$ 時間で返します。`get` と `set` の位置は0始まりで、`i < size()` が前提です。検査が有効なら範囲外で `std::out_of_range` を送出します。

## `mex`

```cpp
std::size_t mex() const noexcept;
```

現在のmexを $O(1)$ 時間で返します。空の列なら0です。

## `size`

```cpp
std::size_t size() const noexcept;
```

列の長さを $O(1)$ 時間で返します。

## 使用例

```cpp
sirkit::PointSetMex<int> index({0, 0, 2, -1});
assert(index.mex() == 1);
index.set(1, 1);
assert(index.mex() == 3);
```

[実装](../DataStructure/Point_Set_Mex.cpp)・[使用例の全体](../Examples/point_set_mex_example.cpp)・[AtCoder Verify](../Verify/current/submissions/point_set_mex.cpp)・[yukicoder Verify](../Verify/current/submissions/point_set_mex_ranges.cpp)
