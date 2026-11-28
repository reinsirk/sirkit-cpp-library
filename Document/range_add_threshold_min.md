# 固定キーで絞り込む区間最小値

各位置に固定キーと更新できる値を持ちます。値を区間加算しながら、指定したキー以上の位置から最小値を選べます。たとえばキーが `{2, 4, 3}`、値が `{10, 20, 5}` のとき、キー3以上に絞った最小値は5です。

## `RangeAddThresholdMin<Key>`

```cpp
RangeAddThresholdMin();
RangeAddThresholdMin(std::span<const Key> keys, std::span<const std::int64_t> values,
                     std::size_t block_width = 0);
```

キーと値の列をコピーします。キーには全順序が必要です。列の長さが異なると `std::invalid_argument` を送出します。ブロック幅 $B$ の既定値はおよそ $\sqrt n$ で、0を渡した場合も同じです。比較とキーのコピーを定数時間とすると、構築は $O(n\log(B+1)+1)$ 時間・$O(n+1)$ 領域です。

## `size`

列の長さを $O(1)$ 時間で返します。

## `empty`

空の列かどうかを $O(1)$ 時間で返します。

## `get`

位置を指定して現在の値を $O(1)$ 時間で返します。

## `add`

```cpp
void add(std::size_t left, std::size_t right, std::int64_t delta);
```

半開区間 `[left, right)` の全要素に `delta` を足します。$O(B+n/B+1)$ 時間です。更新後の各値は `std::int64_t` の範囲内とします。検査が有効なら、範囲を超えると `std::overflow_error` を送出し、更新前の値を保ちます。

## `minimum_at_least`

```cpp
std::optional<std::int64_t> minimum_at_least(const Key &threshold) const;
std::optional<std::int64_t> minimum_at_least(std::size_t left, std::size_t right,
                                             const Key &threshold) const;
```

キーが `threshold` 以上の位置から最小の値を返します。該当する位置がなければ `std::nullopt` です。全体の照会は $O((n/B+1)\log(B+1))$ 時間、半開区間の照会は $O(B+(n/B+1)\log(B+1))$ 時間です。

## 使用例

```cpp
const std::array keys{2, 4, 3};
const std::array<std::int64_t, 3> values{10, 20, 5};
sirkit::RangeAddThresholdMin<int> range(keys, values);
assert(range.minimum_at_least(3) == 5);
range.add(1, 3, 7);
assert(range.minimum_at_least(3) == 12);
```

位置の添字は0始まりです。検査が有効なら、範囲外の位置・区間には `std::out_of_range` を送出します。

[実装](../DataStructure/Range_Add_Threshold_Min.cpp)・[使用例の全体](../Examples/range_add_threshold_min_example.cpp)・[Verify](../Verify/current/submissions/range_add_threshold_min.cpp)
