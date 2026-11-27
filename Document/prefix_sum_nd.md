# 多次元累積和

`CumulativeSum2D<T>` は長方形、`CumulativeSum3D<T>` は直方体の和を扱います。点に値を入れるなら `add → build → query`、領域に値を足すなら `imos → build → imos_get` の順で使います。

```cpp
sirkit::CumulativeSum2D<long long> grid(3, 4);
grid.add(0, 1, 5);
grid.add(2, 3, 7);
grid.build();
assert(grid.query(0, 1, 3, 4) == 12); // 行 [0, 3)、列 [1, 4)
```

位置は0始まりです。領域は各軸で左端を含み、右端を含みません。計算量の `D` は次元数、`M` は各軸の長さに1を足した値の積です。

## 構築

```text
explicit CumulativeSum2D<T>(std::size_t h, std::size_t w);
explicit CumulativeSum3D<T>(std::size_t h, std::size_t w, std::size_t d);
explicit CumulativeSumND<T, D>(const Index &shape);
```

各軸の長さを指定します。時間・領域は `O(M+D)` です。長さ0の軸があれば、時間・領域は `O(D)` です。`T{}` が加法の単位元となり、中間結果が収まる型を選びます。

## `add`

```text
void add(std::size_t i, std::size_t j, const T &value);
```

`build` の前に、有効な座標の1点へ値を足します。時間は `O(D)` です。

## `build`

```text
void build();
```

加算をすべて済ませてから一度呼びます。時間は `O(DM)` で、長さ0の軸があれば `O(1)` です。

## `query`

```text
T query(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2) const;
```

`add` と `build` のあとで `[i1, i2) × [j1, j2)` の和を返します。右端には軸の長さまで指定でき、空領域の和は `T{}` です。時間は `O(2^D)` です。

## `imos`

```text
void imos(std::size_t i1, std::size_t j1, std::size_t i2, std::size_t j2, const T &value = T{1});
```

`build` の前に半開領域へ値を足します。値を省略すると1を足します。時間は `O(2^D)` です。

## `imos_get`

```text
T imos_get(std::size_t i, std::size_t j) const;
```

`imos` と `build` のあとで、有効な座標の1点の値を返します。時間は `O(D)` です。

## `shape`

```text
const Index &shape() const noexcept;
```

各軸の長さを `O(1)` で返します。

3次元の点は `(i, j, k)`、領域は `(i1, j1, k1, i2, j2, k2)` の順で指定します。`CumulativeSumND<T, D>`（`D >= 1`）では長さと座標を `Index = std::array<std::size_t, D>` で渡します。

[使用例](../Examples/prefix_sum_nd_example.cpp) · [2次元Verify](../Verify/current/submissions/prefix_sum_2d.cpp) · [3次元Verify](../Verify/current/submissions/prefix_sum_3d.cpp) · [参考にしたNyaanの二次元累積和](https://nyaannyaan.github.io/library/data-structure-2d/2d-cumulative-sum.hpp)
