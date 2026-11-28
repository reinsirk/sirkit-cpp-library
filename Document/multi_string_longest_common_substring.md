# 複数の文字列で共通する最長部分文字列

すべての文字列に連続して現れる最長の文字列と、各文字列での開始位置を求めます。

## `longest_common_substring`

```text
LongestCommonSubstring longest_common_substring(std::span<const std::string_view> texts)
```

戻り値の `length` は最長部分文字列の長さ、`starts[i]` は `texts[i]` での0始まりの開始位置です。最長の文字列が複数ある場合はいずれかを返します。

共通する文字列が空なら長さと開始位置は0です。入力が1本ならその全体、0本なら開始位置も空です。文字はバイトとして扱います。

空でない文字列を2本以上渡す場合、文字列長の合計 $N$ と本数 $k$ の和は `INT_MAX-1` 未満です。計算量は $O(N+k+1)$ 時間・領域です。

## 使用例

```cpp
const std::array<std::string_view, 3> texts{"banana", "kanata", "panama"};
const auto common = sirkit::longest_common_substring(texts);
assert(common.length == 3);
assert(texts[0].substr(common.starts[0], common.length) == "ana");
```

[実装](../String/Multi_String_Longest_Common_Substring.cpp)・[使用例の全体](../Examples/multi_string_longest_common_substring_example.cpp)・[同梱する接尾辞配列のAPI](suffix_array.md)
