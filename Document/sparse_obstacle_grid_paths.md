# 少数の障害物を避ける格子経路

大きなグリッドで、左上から右下へ右と下の移動だけで進む経路を数えます。グリッド全体を用意せず、壁の座標だけを使います。

## `count_sparse_grid_paths`

```cpp
template <typename Choose>
auto count_sparse_grid_paths(int height, int width,
                             std::span<const std::pair<int, int>> walls, Choose choose);
```

`walls` は0始まりの `(行, 列)` の列です。順不同で、同じ壁が複数あっても構いません。左上か右下に壁がある場合や、どちらかの寸法が0の場合、経路数は0です。

`choose(n, k)` は2個の `std::int64_t` を受け、二項係数を返す関数です。結果は `choose` と同じ型で、その型で0の構築、減算、乗算ができる必要があります。剰余がほしい場合は、二項係数も返却型の演算も同じ法で行います。

壁の数を $K$ として $O((K+1)^2)$ 時間・$O(K+1)$ 領域です。二項係数と返却型の演算は $O(1)$ とします。検査が有効なら、負の寸法は `std::invalid_argument`、グリッド外の壁は `std::out_of_range` になります。

## 使用例

```cpp
const auto choose = [](std::int64_t n, std::int64_t k)
{
    std::int64_t result = 1;
    for (std::int64_t i = 1; i <= k; ++i)
    {
        result = result * (n - i + 1) / i;
    }
    return result;
};
const std::array walls{std::pair{1, 1}};
const auto paths = sirkit::count_sparse_grid_paths(3, 3, walls, choose);
assert(paths == 2);
```

[実装](../Algorithm/Sparse_Obstacle_Grid_Paths.cpp)・[二項係数を含む使用例](../Examples/sparse_obstacle_grid_paths_example.cpp)・[Verify](../Verify/current/submissions/sparse_obstacle_grid_paths.cpp)
