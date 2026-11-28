# 数直線上で近い順に点を探す

固定した座標の列をもとに、指定座標から何番目に近い点かを調べます。同じ座標にある点も別々に数えます。

## `NearestValueIndex`

```cpp
NearestValueIndex();
explicit NearestValueIndex(std::span<const std::int64_t> values);
```

長さ $n$ の座標列をコピーして整列します。構築は $O(n\log(n+1)+1)$ 時間・$O(n+1)$ 領域です。

## `size`

点の数を $O(1)$ 時間で返します。

## `empty`

点がないかを $O(1)$ 時間で返します。

## `kth_nearest`

```cpp
std::optional<NearestValue> kth_nearest(std::int64_t center, std::size_t rank) const;
```

`center` から近い順に並べ、1始まりの `rank` 番目を返します。同距離なら座標が小さい方を先に数えます。範囲外の順位では `std::nullopt` です。$O(\log(n+1)+1)$ 時間で、中心の左右にある2つの距離列を二分探索します。

## `NearestValue`

`value` は点の座標、`distance` は中心からの距離です。座標は `std::int64_t`、距離は両端の差も扱える `std::uint64_t` です。

## `count_within`

```cpp
std::size_t count_within(std::int64_t center, std::uint64_t radius) const;
```

`center` からの距離が `radius` 以下の点を $O(\log(n+1)+1)$ 時間で数えます。

## 使用例

```cpp
const std::array<std::int64_t, 4> values{-3, -1, 5, 6};
const sirkit::NearestValueIndex points(values);
const auto third = points.kth_nearest(-2, 3);
assert(third && third->distance == 7);
assert(points.count_within(-2, 1) == 2);
```

[実装](../DataStructure/Nearest_Value_Index.cpp)・[使用例の全体](../Examples/nearest_value_index_example.cpp)・[Verify](../Verify/current/submissions/nearest_value_index.cpp)
