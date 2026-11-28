# 左端に一致する部分文字列の繰り返し削除

同じパターンが現れるたびに、左端の一致を消して前後をつなぎます。たとえば `AABCBCZ` から `ABC` を消すと `ABCZ` になり、もう一度消すと `Z` が残ります。

## `erase_pattern`

```cpp
std::string erase_pattern(std::string_view text, std::string_view pattern);
```

削除後の文字列を返します。一致が重なる場合も左端から消すため、`ababa` から `aba` を消した結果は `ba` です。

## `erase_pattern_inplace`

```cpp
std::size_t erase_pattern_inplace(std::string &text, std::string_view pattern);
```

`text` を削除後の内容に更新し、消した出現の個数を返します。`pattern` に `text` 内の部分文字列を指定できます。

どちらもテキストの長さ $n$、パターンの長さ $m$ に対し $O(n+m+1)$ 時間・領域です。比較はバイト単位で、空のパターンを渡すと `std::invalid_argument` を送出します。

## 使用例

```cpp
assert(sirkit::erase_pattern("ababa", "aba") == "ba");
std::string text = "AABCBCZ";
const auto removed = sirkit::erase_pattern_inplace(text, "ABC");
assert(text == "Z" && removed == 2);
```

[実装](../String/Pattern_Erasure.cpp)・[使用例の全体](../Examples/pattern_erasure_example.cpp)・[Verify](../Verify/current/submissions/pattern_erasure.cpp)
