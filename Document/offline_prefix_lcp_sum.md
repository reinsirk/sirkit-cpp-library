# 文字追加と最長共通接頭辞長の総和

複数の文字列に対する末尾への追加と、最長共通接頭辞長の総和の照会をまとめて処理します。照会では指定した文字列と現在のすべての文字列を一つずつ比較したときの長さを合計します。

## 操作の指定

```text
PrefixLcpAppend{index, byte}
PrefixLcpQuery{index}
using PrefixLcpOperation = std::variant<PrefixLcpAppend, PrefixLcpQuery>
```

`index` は0始まりです。追加では指定位置の文字列に1バイトを加え、照会では現在の文字列を指定します。照会先自身の長さも総和に含めます。

## `offline_prefix_lcp_sum`

```text
std::vector<std::uint64_t> offline_prefix_lcp_sum(std::span<const std::string_view> initial,
                                                     std::span<const PrefixLcpOperation> operations)
```

各照会の結果を入力順で返します。空文字列や同じ内容の文字列を初期値に使えます。文字列数と作られるTrieの頂点数は `INT_MAX` 以下です。

初期文字列数を $N$、合計長を $S$、操作数を $Q$、Trieの頂点数を $M$ とします。バイト値の種類数を固定として $O(N+S+(Q+1)\log(M+1))$ 時間・$O(N+M+Q)$ 領域です。

## 使用例

```cpp
const std::array<std::string_view, 3> words{"a", "ab", "b"};
const std::array<sirkit::PrefixLcpOperation, 3> operations{
    sirkit::PrefixLcpQuery{0}, sirkit::PrefixLcpAppend{0, 'b'}, sirkit::PrefixLcpQuery{1},
};
const auto result = sirkit::offline_prefix_lcp_sum(words, operations); // {2, 4}
```

[実装](../String/Offline_Prefix_Lcp_Sum.cpp)・[使用例の全体](../Examples/offline_prefix_lcp_sum_example.cpp)・[同梱Fenwick木](fenwick_tree.md)
