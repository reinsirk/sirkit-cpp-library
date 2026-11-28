# 0と1の区間統計

0と1からなる列に、区間反転と区間代入を行います。各ビットの個数や連続区間の長さを照会できます。[本体](../DataStructure/Binary_Range_Statistics.cpp)

| 操作 | 内容 |
| --- | --- |
| `BinaryRangeStatistics(bits)` / `(n)` | 文字列、または `n` 個の0から構築 |
| `flip(l, r)` | 半開区間の0と1を反転 |
| `assign(l, r, bit)` | 半開区間を指定ビットにする |
| `stats(l, r)` / `all_stats()` | 区間、または列全体の統計 |

結果の `ones` は1の個数です。`prefix[bit]`、`suffix[bit]`、`longest[bit]` は指定ビットだけからなる接頭辞、接尾辞、最長区間の長さを返します。

```cpp
sirkit::BinaryRangeStatistics bits("001011");
bits.flip(0, 2);
assert(bits.stats(0, 4).longest[1] == 3);
bits.assign(2, 6, false);
assert(bits.all_stats().ones == 2);
```

構築は時間・領域とも $O(n)$ です。区間操作は $O(\log(n+1))$、全体の統計は $O(1)$ で取得できます。

[実行できる使用例](../Examples/binary_range_statistics_example.cpp)・[AtCoder Verify](../Verify/current/submissions/binary_range_statistics.cpp)
