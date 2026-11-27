# 回文の最小分割

文字列を連続した回文に分割し、その個数を最小にします。

```cpp
const auto cuts = sirkit::min_palindromic_partition("noonabbad");
// cuts = {0, 4, 8, 9}。分割は "noon", "abba", "d"。
```

## `min_palindromic_partition`

```text
std::vector<int> min_palindromic_partition(std::string_view text);
```

`std::string_view` を受け取り、最少個数で分割した境界の列を `std::vector<int>` で返します。各部分は `[cuts[i], cuts[i+1])`、個数は `cuts.size() - 1` です。空文字列の結果は `{0}`。最適な分割が複数あれば、そのうち一つを返します。

NUL を含む任意のバイトを扱い、境界もバイト単位です。長さが `INT_MAX - 1` を超えると `std::length_error` を送出します。

回文木で等差数列をなす長さの接尾回文をまとめ、接頭辞ごとの最小個数と直前の境界を求めます。長さ $N$ に対して $O(N\log(N+1))$ 時間、$O(N)$ 領域です。

[使用例](../Examples/palindromic_factorization_example.cpp) ／ [Verify](../Verify/current/submissions/palindromic_factorization_local.cpp) ／ [独立参照テスト](../Verify/current/tests/palindromic_factorization_reference_test.cpp) ／ [原論文](https://arxiv.org/abs/1506.04862)
