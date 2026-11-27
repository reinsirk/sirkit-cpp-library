# 静的な区間最頻値

固定の配列を前計算し、区間ごとに最頻値と出現回数を求めます。例えば出現回数が最大でも区間長の半分に満たないとき、その値を答えます。

要素数を $n$ とし、要素の比較とコピーを定数時間と数えます。

## `StaticRangeMode`

```text
template <std::totally_ordered T = std::int64_t>
    requires std::copyable<T>
class StaticRangeMode;

explicit StaticRangeMode(std::span<const T> input = {});
```

初期列を渡します。要素数は `INT_MAX` 以下です。空の列も扱えます。 $O(1+n\sqrt n)$ 時間・$O(n+1)$ 領域です。

## `query`

```text
std::optional<std::pair<T, int>> query(int left, int right) const;
```

0始まりの半開区間 `[left,right)` の最頻値と出現回数を返します。`0 <= left <= right <= n` を満たしてください。同率ならそのうち1つ、空区間なら `std::nullopt` を返します。 $O(1+\sqrt n)$ です。

## `size`

```text
int size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## 使用例

```cpp
const std::vector<int> a{9, 7, 1, 7, 2};
const sirkit::StaticRangeMode<int> mode(a);
const auto [value, count] = *mode.query(0, 5); // 7, 2
```

照会を先にまとめて並べ替える方法には [Mo](mo.md) もあります。

[使用例](../Examples/static_range_mode_example.cpp) ／ [Verify](../Verify/current/submissions/static_range_mode_query.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/static_range_mode_query)
