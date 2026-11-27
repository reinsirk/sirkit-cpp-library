# 最長共通部分列

二つの文字列に共通する部分列のうち、最長のものを1つ復元します。

```cpp
auto answer = sirkit::longest_common_subsequence("abcde", "ace");
assert(answer == "ace");
```

## `longest_common_subsequence`

```text
std::string longest_common_subsequence(std::string_view a, std::string_view b);
```

各文字列から順序を保って文字を選び、最長の共通部分列を返します。同じ長さの答えが複数あればどれか1つを返します。文字列の長さを $n,m$ として $O(nm+1)$ 時間・$O(\min(n,m)+\log(\max(n,m)+1))$ 領域です。

[使用例](../Examples/longest_common_subsequence_example.cpp) ／ [Verify](../Verify/current/submissions/lcs_sequence.cpp)
