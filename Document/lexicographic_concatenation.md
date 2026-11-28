# 並べ替えた文字列の辞書順連結

`lexicographic_concatenation_order(words)` は、全ての文字列を一度ずつ連結した結果が辞書順最小となる添字の並びを返します。`second_lexicographic_concatenation(words)` の戻り値は、全順列の連結結果を辞書順に並べたときの2番目です。同じ結果もそれぞれ順位に含め、入力が2本未満の場合は `nullopt` を返します。

```cpp
std::array<std::string_view, 3> words{"b", "aab", "a"};
auto order = sirkit::lexicographic_concatenation_order(words); // {2, 1, 0}
auto second = sirkit::second_lexicographic_concatenation(words); // "aabab"
```

`x+y` と `y+x` の辞書順で並べ替えます。接尾辞の共通接頭辞長を前処理するので、比較のたびに連結した文字列を作る必要はありません。空文字列も扱い、順序は符号なしバイト値で比較します。

文字列の本数を $n$、総長を $L$ とすると、両関数とも $O(n\log(n+1)+L)$ 時間、$O(n+L+1)$ 領域です。総長は `INT_MAX-2` 以下にします。

[使用例](../Examples/lexicographic_concatenation_example.cpp) ／ [Verify](../Verify/current/submissions/lexicographic_concatenation_second.cpp) ／ [並べ替えの原理](https://info.atcoder.jp/entry/algorithm_lectures/lex_min_string_concatenation)
