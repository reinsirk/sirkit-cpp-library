# 接頭辞関数とKMP文字列検索

各接頭辞について接頭辞と接尾辞の一致長を求め、その情報を使ってパターンを線形時間で検索できます。

```cpp
auto border = sirkit::prefix_function("ababa");
auto matches = sirkit::find_occurrences("ababa", "aba");
```

## `prefix_function`

```text
std::vector<std::size_t> prefix_function(std::string_view text);
```

`result[i]` は `text[0,i+1)` の真の接頭辞でもある接尾辞の最大長です。時間・領域ともに $O(n)$ です。

## `find_occurrences`

```text
std::vector<std::size_t> find_occurrences(std::string_view text, std::string_view pattern);
```

重複も含め、一致の開始位置を昇順で返します。パターンが空なら、末尾を含むすべての境界を返します。文字列長を $n,m$、一致数を $k$ として $O(n+m+1)$ 時間・$O(m+k)$ 領域です。

[使用例](../Examples/prefix_function_example.cpp) ／ [Verify](../Verify/current/submissions/kmp_string_matching.cpp)
