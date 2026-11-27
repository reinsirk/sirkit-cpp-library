# 区間の最長増加部分列

固定された数列の任意の半開区間について、狭義の最長増加部分列の長さを求めます。同じ値を増加列で続けて選ぶことはできません。

```cpp
const sirkit::StaticRangeLIS lis(std::array{3, 1, 2, 2, 4});
auto answer = lis.length(1, 4);  // {1, 2, 2} のLIS長は2
```

## `StaticRangeLIS`

```text
template <class Sequence>
explicit StaticRangeLIS(const Sequence &values);
```

整数を値型とする長さ既知のランダムアクセス範囲を渡します。重複、負数、空列を扱えます。要素数を $N$ とすると前処理は $O(N\log^2(N+1))$ 時間・$O(N\log(N+1))$ 領域です。

## `length`

```text
std::size_t length(std::size_t left, std::size_t right) const;
```

添字は0始まりで、区間の右端を含みません。空区間の答えは0です。通常版では添字が範囲外、または `left > right` なら `std::out_of_range` を送出します。

部分列として選べない要素数を表す点を、値で分割した区間の単位Monge行列を合成して求めます。問い合わせでは点を数え、区間長から引きます。1回あたり $O(1+\log(N+1))$ 時間です。

[使用例](../Examples/static_range_lis_example.cpp) ／ [Verify](../Verify/current/submissions/static_range_lis_query.cpp) ／ [原論文](https://doi.org/10.4230/LIPIcs.ESA.2025.74)
