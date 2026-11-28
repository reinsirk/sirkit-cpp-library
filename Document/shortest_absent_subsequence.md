# 含まれない最短の部分列

指定した文字集合で作れる列のうち、元の列に部分列として現れない最短のものを求めます。

## `shortest_absent_subsequence`

```text
std::string shortest_absent_subsequence(text, alphabet = "abcdefghijklmnopqrstuvwxyz")
```

最短の候補が複数あれば、符号なしバイトの辞書順で最小のものを返します。`alphabet` の文字は順不同で、重複していても同じ文字として扱います。空の集合は指定できません。

## `shortest_absent_subsequence_length`

```text
std::size_t shortest_absent_subsequence_length(text, alphabet = "abcdefghijklmnopqrstuvwxyz")
```

同じ条件で最短長だけを返します。元の列の長さを $n$、`alphabet` の長さを $a$ とすると、いずれも $O(n+a+1)$ 時間です。追加領域は復元ありで $O(n+1)$、長さだけなら $O(1)$ です。

## 使用例

```cpp
assert(sirkit::shortest_absent_subsequence("abba", "ab") == "aaa");
assert(sirkit::shortest_absent_subsequence_length("abba", "ab") == 3);
```

[参考論文](https://fi.episciences.org/12396)・[実装](../String/Shortest_Absent_Subsequence.cpp)・[使用例の全体](../Examples/shortest_absent_subsequence_example.cpp)
