# 静的区間最小値

更新のない配列に対し、区間の最小値とその位置を定数時間で取得できます。同じ最小値が複数あれば左端に近い位置を返します。

## `StaticRangeMinimum`

```text
template<class T, class Compare = std::less<>>
class StaticRangeMinimum;
explicit StaticRangeMinimum(std::vector<T> values, Compare compare = Compare{})
```

配列を所有し、長さを $n$ として $O(n+1)$ 時間・領域で構築します。`compare(a,b)` は `a` を `b` より優先するときに真となる狭義弱順序です。比較は定数時間とします。例えば `std::greater<>` を指定すると最大値を求めます。コピー構築は前処理結果を共有するため $O(1)$ 時間です。

## `size`

```text
std::size_t size() const noexcept
```

配列の長さを $O(1)$ 時間で返します。

## `argmin`

```text
std::size_t argmin(std::size_t left, std::size_t right) const
```

半開区間 `[left, right)` で最小値が最初に現れる0始まりの添字を $O(1)$ 時間で返します。`left < right <= size()` が前提です。

## `min`

```text
const_reference min(std::size_t left, std::size_t right) const
```

`argmin` の位置にある要素を $O(1)$ 時間で返します。通常は保持している要素への参照、`T=bool` のときは値です。参照は前処理結果を共有するオブジェクトがある間有効です。

## 使用例

```cpp
const sirkit::StaticRangeMinimum<int> index({5, 2, 3, 2, 8});
assert(index.argmin(1, 5) == 1);
assert(index.min(2, 5) == 2);
```

[実装](../DataStructure/Static_Range_Minimum.cpp)・[使用例の全体](../Examples/static_range_minimum_example.cpp)
