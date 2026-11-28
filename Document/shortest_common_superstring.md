# 最短共通超文字列

すべての入力文字列を連続した部分文字列として含む、最短の文字列を一つ求めます。

## `shortest_common_superstring`

```text
std::string shortest_common_superstring(std::span<const std::string_view> words)
```

空の文字列、重複、一つの入力が別の入力に含まれる場合も扱えます。文字列はバイト列です。最短のものが複数あるときはそのうちの一つを返します。入力は20個以下、長さの合計は `UINT32_MAX` 以下です。

入力数を $N$、長さの合計を $S$、空・重複・他の入力に含まれる文字列を除いた数を $K$ とします。$O(NS+K^2 2^K+1)$ 時間・$O(S+K 2^K+N^2+1)$ 領域です。KMP法で末尾と先頭の重なりを求め、部分集合DPで並べ方を決めます。

## 使用例

```cpp
const std::array<std::string_view, 3> words{"snuke", "kensho", "uk"};
const auto text = sirkit::shortest_common_superstring(words);
assert(text == "snukensho");
```

[実装](../String/Shortest_Common_Superstring.cpp)・[使用例の全体](../Examples/shortest_common_superstring_example.cpp)・[同梱する文字列検索のAPI](prefix_function.md)
