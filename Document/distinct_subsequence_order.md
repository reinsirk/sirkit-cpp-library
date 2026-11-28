# 相異なる部分列の辞書順

元のバイト列から一部を除いて作れる、空でない部分列を調べます。元の位置の選び方が違っても、同じ並びは1種類です。辞書順は符号なしバイトの値で比較します。

## `DistinctSubsequenceOrder`

```text
explicit DistinctSubsequenceOrder(std::string_view text)
```

長さ $n$ のバイト列を $O(n+1)$ 時間・領域で索引化します。コピーは索引を共有します。

## `size`

長さを $O(1)$ 時間で返します。

## `count_distinct`

空でない部分列の種類数を $O(1)$ 時間で取得します。

## `kth_smallest`

```text
std::optional<std::string> kth_smallest(std::uint64_t rank) const
```

1始まりの `rank` 番目の部分列を返します。範囲外なら `std::nullopt` です。

## `rank_of`

```text
std::optional<std::uint64_t> rank_of(std::string_view pattern) const
```

`pattern` の辞書順順位を取得します。空文字列や作れない部分列なら `std::nullopt` です。

## `prefix_count`

```text
std::uint64_t prefix_count(std::string_view prefix) const
```

`prefix` で始まる部分列の種類数を数えます。空の `prefix` なら `count_distinct()` と同じ値です。

種類数と `rank_of` は `UINT64_MAX` で頭打ちになります。`kth_smallest` は指定できるすべての順位に対して正確です。異なるバイトの種類数を $\sigma$、クエリの入力または出力の長さを $k$ とすると、順位と復元は $O(\sigma k\log(n+2)+1)$、接頭辞の件数は $O(k\log(n+2)+1)$ 時間です。

## 使用例

```cpp
const sirkit::DistinctSubsequenceOrder order("aba");
assert(order.count_distinct() == 6);
assert(order.kth_smallest(4) == "aba");
assert(order.rank_of("ba") == 6);
assert(order.prefix_count("a") == 4);
```

[実装](../String/Distinct_Subsequence_Order.cpp)・[使用例の全体](../Examples/distinct_subsequence_order_example.cpp)
