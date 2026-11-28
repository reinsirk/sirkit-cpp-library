# 各項に上下限がある整数の和

各整数範囲から1個ずつ選ぶ方法を、合計ごとに数えます。配る個数に上限がある場合や、各変数の上下限が決まった整数方程式に使えます。

## `bounded_composition_counts(bounds, max_sum, modulus)`

`bounds` は `BoundedCompositionPart{lower, upper}` の列です。両端を含みます。下限がすべて0なら、`std::size_t` の上限列だけを渡せます。

戻り値 `counts[s]` は合計が `s` となる選び方の数を `modulus` で割った余りです。`s=0` から `max_sum` まで返します。範囲を1つも与えないときは、合計0の方法が1通りです。

`modulus` は正の `std::uint32_t` で、既定は `998244353` です。範囲が $n$ 個、`max_sum=K` なら、$O((n+1)(K+1))$ 時間、$O(K+1)$ 領域です。

```cpp
const std::array<sirkit::BoundedCompositionPart, 2> bounds{{{1, 2}, {0, 2}}};
const auto counts = sirkit::bounded_composition_counts(bounds, 4);
// 合計0から4の順で {0, 1, 2, 2, 1}。
```

[使用例](../Examples/bounded_compositions_example.cpp) ／ [Verify](../Verify/current/submissions/bounded_composition_counts_dp_m.cpp)
