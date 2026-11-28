# 文字列の周期と最短繰り返しブロック

文字列の各接頭辞について、周期や接頭辞と接尾辞の一致を調べます。たとえば `abababa` は `ab` を繰り返す途中で作れますが、`ab` を整数回だけ繰り返して作ることはできません。

## `StringPeriods`

```text
explicit StringPeriods(std::string_view text)
```

文字列をバイト列として前処理します。構築後は入力文字列の保存が不要です。長さを $n$ として $O(n+1)$ 時間・領域です。

## `size`

```text
std::size_t size() const noexcept
```

元の文字列のバイト数を $O(1)$ 時間で返します。

以下の関数は先頭 `length` バイトを対象とし、`length <= size()` が前提です。

## `least_period`

```text
std::size_t least_period(std::size_t length) const
```

すべての重なる位置で `text[i] == text[i + p]` が成り立つ最小の正の `p` を $O(1)$ 時間で返します。繰り返しの途中で終わる場合も周期とし、空の接頭辞では0を返します。

## `primitive_root_length`

```text
std::size_t primitive_root_length(std::size_t length) const
```

整数回の繰り返しで接頭辞全体を作れる最短のブロック長を $O(1)$ 時間で返します。1回でしか作れなければ `length`、空なら0を返します。

## `proper_borders`

```text
std::vector<std::size_t> proper_borders(std::size_t length) const
```

接頭辞かつ接尾辞になっている文字列の長さを降順で返します。0と `length` は含みません。返す個数を $k$ として $O(k+1)$ 時間・領域です。

## `periods`

```text
std::vector<std::size_t> periods(std::size_t length) const
```

正の周期を昇順で返します。空なら空の配列、非空なら `length` も含めます。返す個数を $k$ として $O(k+1)$ 時間・領域です。

## 使用例

```cpp
const sirkit::StringPeriods index("abababa");
assert(index.least_period(7) == 2);
assert(index.primitive_root_length(7) == 7);
assert(index.primitive_root_length(6) == 2);
// index.periods(7) は {2, 4, 6, 7}。
```

[実装](../String/String_Periods.cpp)・[使用例の全体](../Examples/string_periods_example.cpp)・[同梱する文字列検索のAPI](prefix_function.md)
