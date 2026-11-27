# 更新付き区間の過半数

値の置き換えと区間の問い合わせを、指定した順番に処理します。ある値が区間の半分を超えて現れたとき、その値を返します。

## `RangeMajorityOperation`

```text
template <class T>
struct RangeMajoritySet { std::size_t position; T value; };

struct RangeMajorityQuery { std::size_t left, right; };

template <class T>
using RangeMajorityOperation = std::variant<RangeMajoritySet<T>, RangeMajorityQuery>;
```

`RangeMajoritySet<T>` は位置 `position` の値を `value` に置き換えます。`RangeMajorityQuery` は半開区間 `[left,right)` を指定します。添字は0から数えます。

## `range_majority_queries`

```text
template <class T>
std::vector<std::optional<T>> range_majority_queries(
    const std::vector<T> &values,
    std::span<const std::type_identity_t<RangeMajorityOperation<T>>> operations);
```

最初の値と全操作を渡すと、問い合わせの答えが順に返ります。空区間や過半数がない区間の答えは `std::nullopt` です。入力の `values` は保ちます。

`T` はコピーでき、`<` と `==` が同じ全順序を表す型を使います。要素数を $n$、操作数を $q$、更新数を $u$ とすると、$n+u$ は $2^{32}-1$ 以下です。値の比較とコピーを定数時間と数えると、$O((n+q)\log(n+u+2))$ 時間・$O(n+q)$ 領域です。

## 使用例

```cpp
const std::vector values{2, 1, 2, 4};
const std::vector<sirkit::RangeMajorityOperation<int>> operations{
    sirkit::RangeMajorityQuery{0, 4},
    sirkit::RangeMajoritySet<int>{1, 2},
    sirkit::RangeMajorityQuery{0, 4},
};
const auto answers = sirkit::range_majority_queries(values, operations);
// answers は {std::nullopt, 2}
```

[使用例](../Examples/range_majority_example.cpp) ／ [Verify](../Verify/current/submissions/majority_voting.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/majority_voting)
