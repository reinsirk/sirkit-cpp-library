# 重複を含む順列の辞書順の順位

`MultisetPermutation<T>(values)` は `values` を並べ替えて作れる異なる列を、辞書順に扱います。たとえば `[1,1,2]` では `[1,1,2]`、`[1,2,1]`、`[2,1,1]` の順です。 要素数は `int` の上限以下です。

- `size()` は元の要素数、`count()` は並べ替えの総数を返します。総数と順位には [BigInteger](big_integer.md) を使います。
- `rank(values)` は0始まりの順位を返します。要素と個数が元の多重集合と一致しない場合は `nullopt` です。
- `unrank(index)` は0始まりの順位から列を復元します。負の順位や総数以上の順位は `nullopt` です。空列は順位0の1通りです。
- 要素数 $n$、値の種類数 $s$、総数の桁数 $D$ として、構築は $O(n(\log(n+1)+D))$、順位と復元は $O(n(D+\log(s+1)))$ 時間です。保持領域は $O(s+D)$、`count()` のコピーは $O(D)$ です。

[使用例](../Examples/multiset_permutation_example.cpp) ／ [Verify（2種類）](../Verify/current/submissions/multiset_permutation_abc202_d.cpp) ／ [Verify（多種類）](../Verify/current/submissions/multiset_permutation_abc215_c.cpp)
