# 各文字列の順序を保つ辞書順最小マージ

`lexicographic_string_shuffle(words)` は、毎回どれかの文字列の先頭から1文字を取り、すべての文字を使うマージのうち辞書順最小の結果を返します。空文字列も渡せます。辞書順は符号なしバイト値の順序です。

```cpp
std::array<std::string_view, 2> words{"b", "ba"};
auto merged = sirkit::lexicographic_string_shuffle(words); // "bab"
```

残りの接尾辞は終端をすべての文字より大きい記号として比較します。この例で `"ba"` の先頭を選ぶ理由は、すぐ次に `a` を選べるためです。接尾辞配列で順位を求め、ヒープで最小の候補を選びます。

文字列の本数を $n$、総長を $L$ とすると、時間は $O(n+L\log(n+1))$、領域は $O(L+1)$ です。総長と空でない文字列の本数の和は `INT_MAX-2` 以下にします。

[使用例](../Examples/lexicographic_string_shuffle_example.cpp) ／ [Verify](../Verify/current/submissions/lexicographic_string_shuffle.cpp)
