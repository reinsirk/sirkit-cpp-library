# 最大長方形

ヒストグラムに収まる長方形や、指定した条件を満たすマスだけからなる長方形のうち、面積が最大のものを求めます。

| API | 戻り値・計算量 |
|---|---|
| `largest_histogram_rectangle(heights)` | 各棒の幅は1です。高さは `std::uint64_t` で、`{left,right,height,area}` を返します。$O(n)$ 時間・領域。 |
| `largest_grid_rectangle(rows, cols, accepted)` | `accepted(row,col)` が真のマスから最大長方形を求め、`{top,bottom,left,right,area}` を返します。`accepted` は行優先で各マスに1回ずつ呼ばれます。$O(HW)$ 時間、$O(W)$ 領域。 |

座標は0始まりで、`right` と `bottom` は範囲に含めません。最大面積が0なら全フィールドが0です。面積が `std::uint64_t` を超える場合は `std::overflow_error` です。

```cpp
const std::array<std::uint64_t, 7> heights{2, 1, 4, 5, 1, 3, 3};
const auto r = sirkit::largest_histogram_rectangle(heights);
// [2, 4)、高さ4、面積8。
```

[使用例](../Examples/largest_rectangle_example.cpp) ／ [ヒストグラムのVerify](../Verify/current/submissions/largest_histogram_rectangle_abc189_c.cpp) ／ [グリッドのVerify](../Verify/current/submissions/largest_grid_rectangle_dpl_3_b.cpp)
