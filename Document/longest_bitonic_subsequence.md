# 最長の山型部分列

元の順序を保って要素を選び、一つの頂点まで増加し、その後は減少する最長の部分列を復元します。増加・減少の片側だけでも構いません。

## `BitonicSubsequenceResult`

`indices` には選んだ要素の元の添字が入ります。`peak` は `indices` の中での頂点の位置なので、頂点の元の添字は `indices[peak]` です。両方の添字は0始まりです。

## `longest_bitonic_subsequence<Strict>(values)`

既定では狭義の増加と減少を求めます。`Strict=false` なら同じ値を続けて選べます。値はコピー構築・代入が可能で、`operator<` で比較できる型です。

左右から各要素を末尾とする最長増加部分列の長さを求め、最も長くつながる頂点を選びます。列長 $n$ に対して $O(1+n\log(n+1))$ 時間、$O(n+1)$ 領域です。空列には空の添字列を返します。

## 使用例

```cpp
const std::vector values{1, 4, 2, 5, 3, 2};
const auto result = sirkit::longest_bitonic_subsequence(values);
assert(result.indices.size() == 5);
assert(result.indices[result.peak] == 3);
```

[実装](../Algorithm/Longest_Bitonic_Subsequence.cpp)・[使用例の全体](../Examples/longest_bitonic_subsequence_example.cpp)・[Verify](../Verify/current/submissions/longest_bitonic_subsequence.cpp)
