# 相異なる部分列の個数

要素の順序を保って取り出せる、空でない相異なる部分列を数えます。位置が異なっても、取り出した値の並びが同じなら1つと数えます。

```cpp
std::vector<int> sequence{1, 2, 1};
auto answer = sirkit::count_distinct_subsequences(sequence);  // 6
```

## `count_distinct_subsequences`

```text
template <class Sequence>
std::uint64_t count_distinct_subsequences(const Sequence &sequence,
                                          std::uint64_t modulus = 998244353);
```

整数を値型とする長さ既知のランダムアクセス範囲を渡します。`std::vector`、`std::span`、`std::array` に対応し、負数や型の最大値も使えます。空列の答えは0です。

戻り値は `std::uint64_t` の剰余です。法は1以上で、0の場合は `std::invalid_argument` を送出します。空部分列も含める場合は、結果に1を足して再度剰余を取ります。

左から順に要素を加え、直前に同じ値を使って作った部分列の重複を引きます。要素数を $N$ とすると、値の幅（最大値と最小値の差）が $N$ 未満なら $O(N)$ 時間、その他は座標圧縮を含めて $O(N\log(N+1))$ 時間です。追加領域は $O(N)$ です。

[使用例](../Examples/number_of_subsequences_example.cpp) ／ [Verify](../Verify/current/submissions/number_of_subsequences.cpp)
