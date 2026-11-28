# 3文字の出現回数を比べる

指定した3文字について、空でない各部分文字列での出現回数を比べます。同じ文字列でも元の区間が異なれば別々に数えます。

## `ThreeSymbolFrequencyCounts`

`all_equal` は3文字の出現回数がすべて等しい区間、`all_distinct` はすべて異なる区間の個数です。`equal_pairs` は指定順の組 `(0,1)`、`(0,2)`、`(1,2)` ごとに、2文字の回数が等しい区間を数えます。

## `analyze_three_symbol_frequencies(text, symbols)`

`symbols` は相異なる3バイトで、既定は `A`、`B`、`C` です。指定外の文字も区間に含められます。列長は `INT32_MAX` 未満です。

接頭辞での頻度差が等しい位置を集計します。列長 $n$ に対して期待 $O(n+1)$ 時間、$O(n+1)$ 領域です。

## 使用例

```cpp
const auto result = sirkit::analyze_three_symbol_frequencies("AABBCC");
assert(result.all_distinct == 4);
assert(result.all_equal == 1);
```

[実装](../String/Three_Symbol_Frequency_Counts.cpp)・[使用例の全体](../Examples/three_symbol_frequency_counts_example.cpp)・[Verify](../Verify/current/submissions/three_symbol_frequency_counts.cpp)
