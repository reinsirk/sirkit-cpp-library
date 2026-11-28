# 格子図形の回転・反転照合

平行移動を許して、二つの整数点の図形が一致する向きを列挙します。

## `lattice_shape_transforms`

```cpp
std::vector<LatticeOrientation> lattice_shape_transforms(
    std::span<const std::pair<std::int64_t, std::int64_t>> source,
    std::span<const std::pair<std::int64_t, std::int64_t>> target,
    bool allow_reflection = false);
```

図形は `(x, y)` の列で渡します。点の順序は任意で、同じ点が複数ある場合は個数も照合します。点数が異なると結果は空です。両方が空なら、許されたすべての向きが返ります。

点数を $n$ として $O(1+n\log(n+1))$ 時間、$O(n+1)$ 領域です。各座標は `std::int64_t` 全域を扱えます。

## `LatticeOrientation`

`kIdentity`、`kRotate90`、`kRotate180`、`kRotate270` の順で返ります。正の90度は `(x, y) → (-y, x)` です。

`allow_reflection` が真の場合は続けて `kReflect` と `kReflectThenRotate90`、`kReflectThenRotate180`、`kReflectThenRotate270` も試します。反転は第1座標の符号を変え、その後で回転します。

## 使用例

```cpp
using Point = std::pair<std::int64_t, std::int64_t>;
const std::array source{Point{0, 0}, Point{1, 0}, Point{0, 1}, Point{0, 2}};
const std::array target{Point{5, 3}, Point{5, 4}, Point{4, 3}, Point{3, 3}};
const auto matches = sirkit::lattice_shape_transforms(source, target);
assert(matches.size() == 1 && matches[0] == sirkit::LatticeOrientation::kRotate90);
```

[実装](../Geometry/Lattice_Shape_Transforms.cpp)・[使用例の全体](../Examples/lattice_shape_transforms_example.cpp)・[Verify](../Verify/current/submissions/lattice_shape_transforms.cpp)
