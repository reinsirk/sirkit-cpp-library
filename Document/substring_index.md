# 部分文字列の索引

固定した文字列で、パターンの出現回数を調べたり、辞書順で指定した順位の部分文字列を取得したりできます。

## `SubstringIndex`

```text
explicit SubstringIndex(std::string_view text)
```

文字列をコピーし、接尾辞配列と隣り合う接尾辞の共通接頭辞長から索引を作ります。長さは `INT_MAX-1` 未満です。バイトの値を符号なし整数として比較します。

文字列の長さを $n$ として $O(n+1)$ 時間・領域です。完成した索引をコピーすると同じデータを共有します。

## `distinct_substring_count`

```text
std::uint64_t distinct_substring_count() const noexcept
```

空でない部分文字列のうち、内容が異なるものの個数を返します。$O(1)$ 時間です。

## `kth_distinct_substring`

```text
std::optional<std::string_view> kth_distinct_substring(std::uint64_t rank) const noexcept
```

辞書順で `rank` 番目に小さい部分文字列を参照するビューを返します。順位は1始まりで、範囲外なら `std::nullopt` です。$O(1+\log(n+1))$ 時間・$O(1)$ 追加領域です。

## `occurrences`

```text
std::span<const int> occurrences(std::string_view pattern) const
```

一致の開始位置を0始まりで返します。並び順は接尾辞の辞書順です。空パターンには空接尾辞を除く全 $n$ 本が一致します。

パターンの長さを $m$ として $O(1+(m+1)\log(n+1))$ 時間・$O(1)$ 追加領域です。取得した部分文字列と一致位置の参照は、索引を共有する個体が生きている間に使えます。

## 使用例

```cpp
const sirkit::SubstringIndex index("banana");
assert(index.occurrences("ana").size() == 2);
assert(index.kth_distinct_substring(3).value() == "ana");
```

[実装](../String/Substring_Index.cpp)・[使用例の全体](../Examples/substring_index_example.cpp)・[同梱する接尾辞配列のAPI](suffix_array.md)
