# スタンプで文字列を作る

何も書いていない列から始めて、同じスタンプで連続区間を何度でも上書きします。目標の文字列が作れるか調べ、可能なら押す順番を求めます。

## `pattern_stamping_order(target, stamp)`

返り値は `std::optional<std::vector<std::size_t>>` です。作れる場合はスタンプを押す位置を実行順に返します。各位置は0始まりです。作れない場合は `std::nullopt`、`target` が空なら空の操作列になります。

目標から操作を逆にたどり、まだ消していない文字がすべて合う位置を順に消します。目標長を $n$、スタンプ長を $m$ とすると、$O(1+nm)$ 時間、$O(n+1)$ 領域です。文字はバイト単位で比較します。

## 使用例

```cpp
constexpr std::string_view target = "ABAAB";
constexpr std::string_view stamp = "AB";
const auto order = sirkit::pattern_stamping_order(target, stamp);
assert(order);
std::string result(target.size(), ' ');
for (const auto position : *order)
{
    result.replace(position, stamp.size(), stamp);
}
assert(result == target);
```

[実装](../String/Pattern_Stamping.cpp)・[使用例の全体](../Examples/pattern_stamping_example.cpp)・[Verify](../Verify/current/submissions/pattern_stamping.cpp)
