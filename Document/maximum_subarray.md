# 最大部分和

連続部分列やグリッド内の長方形から、要素の和が最大になる範囲を求めます。負の値だけでも、要素を1つ以上選びます。

| API | 戻り値・計算量 |
|---|---|
| `maximum_subarray_sum<Sum>(values)` | `{left, right, sum}`。$O(n)$ 時間、$O(1)$ 領域。 |
| `maximum_submatrix_sum<Sum>(rows, cols, value_at)` | `{top, bottom, left, right, sum}`。`value_at(i,j)` はマスの値。$O(HW\min(H,W))$ 時間、$O(\max(H,W))$ 領域。 |

座標は0始まりで、`right` と `bottom` は範囲に含めません。入力に要素がなければ `std::nullopt` を返します。`Sum` の既定は `std::int64_t` です。`Sum{}` は0で、比較と加算は定数時間、途中の和も `Sum` に収まることが前提です。

```cpp
const std::vector<int> values{-5, 4, -1, 6, -9};
const auto best = sirkit::maximum_subarray_sum(values);
// [1,4) を選び、和は9。
```

[使用例](../Examples/maximum_subarray_example.cpp) ／ [Verify](../Verify/current/submissions/maximum_submatrix_awc0097_d.cpp)
