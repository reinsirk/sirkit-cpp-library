# Lyndon 分解

Lyndon 語は、空でない真の接尾辞のどれよりも辞書順で小さい文字列です。文字列を辞書順が非増加になるよう Lyndon 語に分ける方法は一意です。

```cpp
const auto cuts = sirkit::lyndon_factorization("baba"); // {0, 1, 3, 4}
```

## `lyndon_factorization`

```text
std::vector<std::size_t> lyndon_factorization(std::string_view text);
```

隣り合う境界 `cuts[i]`、`cuts[i+1]` が1つの因子の半開区間を表します。先頭は0、末尾は文字列の長さです。空文字列では `{0}` を返します。文字は `unsigned char` の値で比較します。

文字列長を `N` として、Duval 法で `O(N)` 時間。出力以外の追加領域は `O(1)` です。

[使用例](../Examples/lyndon_factorization_example.cpp) ／ [Verify](../Verify/current/submissions/lyndon_factorization.cpp) ／ [Duval の原論文](https://doi.org/10.1016/0196-6774(83)90017-2)
