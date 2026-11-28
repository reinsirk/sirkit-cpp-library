# 順列の辞書順とLehmer符号

`0` から `n-1` までの順列を辞書順に並べ、0始まりの順位と順列を相互に変換します。順位が64ビットを超える場合も、Lehmer符号なら各桁に分けて順位を表せます。

## 使い方

```cpp
std::vector<int> p{2, 0, 3, 1};
sirkit::permutation_rank(p);              // 13
sirkit::permutation_unrank(4, 13);        // [2, 0, 3, 1]
auto digits = sirkit::permutation_lehmer_code(p); // [2, 0, 1, 0]
sirkit::permutation_from_lehmer_code(digits);      // [2, 0, 3, 1]
```

[使用例](../Examples/permutation_order_example.cpp)

Lehmer符号の各桁は、その位置の値より小さく、右側にある値の個数です。順位が `std::uint64_t` に収まらない場合や、長さ `n` に対して順位が `n!` 以上の場合は、対応する変換が `std::nullopt` を返します。

## 計算量

長さを `n` とすると、どの操作も `O(n log(n+1)+1)` 時間と `O(n+1)` 領域を使います。
