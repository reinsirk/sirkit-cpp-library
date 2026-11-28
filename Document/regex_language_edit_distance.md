# 正規表現で指定した文字列間の編集距離

`regex_language_edit_distance(a, b)` は、2つのパターンに一致する文字列を1個ずつ選び、挿入・削除・置換の回数の合計を最小にします。

- 英小文字はその文字と一致します。
- `x?` は `x` が0個または1個、`x*` は0個以上を表します。

```cpp
auto distance = sirkit::regex_language_edit_distance("a*bcd", "aaaabd?"); // 1
```

入力の長さを $n, m$ とすると、時間は $O(nm+n+m+1)$、領域は $O(n+m+1)$ です。入力パターンを字句単位に分けてDPするため、一致する文字列を列挙せずに求められます。

[使用例](../Examples/regex_language_edit_distance_example.cpp) ／ [Verify](../Verify/current/submissions/regex_language_edit_distance.cpp)
