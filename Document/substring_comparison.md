# 部分文字列の一致・辞書順比較

固定した文字列の部分文字列を繰り返し比較するときに使います。接尾辞の最長共通接頭辞も求められます。

## `SubstringComparison`

```text
explicit SubstringComparison(std::string_view text)
```

バイト列を前処理します。構築後は入力文字列を破棄できます。長さ $n$ は `INT_MAX - 2` 以下で、構築は $O(n+1)$ 時間・領域です。コピー構築では前処理結果を共有します。

## `size`

```text
std::size_t size() const noexcept
```

元の文字列のバイト数を返します。

## `common_prefix`

```text
std::size_t common_prefix(std::size_t first, std::size_t second) const
```

0始まりの指定位置から末尾までの接尾辞を比較し、先頭から一致する長さを返します。`size()` は空の接尾辞を表します。

## `equal`

```text
bool equal(std::size_t first, std::size_t second, std::size_t length) const
```

指定した開始位置から `length` バイトが一致すれば `true` を返します。

## `compare`

```text
std::strong_ordering compare(std::size_t first, std::size_t first_length,
                             std::size_t second, std::size_t second_length) const
```

長さが異なる部分文字列も辞書順で比較できます。バイト値は符号なしの順序で、片方がもう片方の真の接頭辞なら短い方が先です。

照会には空区間も指定できます。開始位置と長さは元の文字列に収まることが前提で、どの照会も $O(1)$ 時間です。

## 使用例

```cpp
const sirkit::SubstringComparison index("banana");
assert(index.common_prefix(1, 3) == 3);  // "anana" と "ana"
assert(index.equal(1, 3, 3));            // "ana" 同士
assert(index.compare(0, 3, 1, 3) == std::strong_ordering::greater);
```

[実装](../String/Substring_Comparison.cpp)・[使用例の全体](../Examples/substring_comparison_example.cpp)・[接尾辞配列](suffix_array.md)・[静的区間最小値](static_range_minimum.md)
