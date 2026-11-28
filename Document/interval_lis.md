# 区間から値を選ぶ最長増加部分列

各位置の値を整数の閉区間から選びます。位置の順序を保って狭義増加な部分列を作るとき、可能な最長の長さと、各長さで作れる最小の末尾値を求めます。

## `interval_lis_tails`

```text
LISInterval{left, right}
std::vector<std::int64_t> interval_lis_tails(std::span<const LISInterval> intervals)
```

各位置の値を `left` 以上 `right` 以下から選びます。端点は符号付き64ビット整数で、`left <= right` が前提です。区間の個数は `INT_MAX` 以下です。

結果の添字 `i` には長さ `i + 1` で作れる最小の末尾値が入ります。各末尾値は長さごとに独立に求めます。戻り値の長さが最長増加部分列の最大の長さです。入力が空なら空の配列を返します。

区間数を $n$ として、期待 $O((n+1)\log(n+2))$ 時間・$O(n+1)$ 領域です。

## 使用例

```cpp
const std::array intervals{
    sirkit::LISInterval{3, 5},
    sirkit::LISInterval{1, 4},
    sirkit::LISInterval{2, 7},
};
const auto tails = sirkit::interval_lis_tails(intervals); // {1, 2, 5}
```

[実装](../Algorithm/Interval_LIS.cpp)・[使用例の全体](../Examples/interval_lis_example.cpp)・[同梱する暗黙Treap](implicit_treap.md)
