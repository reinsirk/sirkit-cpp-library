# 1点更新・区間出現回数

整数列の値を1つずつ置き換えながら、区間内で指定値が何回出るかを求めます。初期列にない値も代入できます。

要素数を $n$ とし、添字は0から数えます。入力検査が有効な場合、範囲外の添字・区間には `std::out_of_range` を送出します。

## `PointSetRangeFrequency`

```text
template <std::integral T = std::int64_t>
class PointSetRangeFrequency;

explicit PointSetRangeFrequency(std::span<const T> values);
```

整数の初期列を渡します。空の列も扱えます。要素数は $2^{32}-2$ 以下です。 $O(1+n\log(n+1))$ 時間・$O(n+1)$ 領域です。

## `size`

```text
std::size_t size() const noexcept;
```

要素数を返します。 $O(1)$ です。

## `get`

```text
T get(std::size_t i) const;
```

`i < n` の位置の値を返します。 $O(1)$ です。

## `set`

```text
void set(std::size_t i, T value);
```

`i < n` の位置を `value` に置き換えます。 $O(1+\log(n+1))$ です。

## `count`

```text
std::size_t count(std::size_t left, std::size_t right, T value) const;
```

`left <= right <= n` を満たす区間 `[left,right)` で `value` の出現回数を返します。空区間の答えは0です。 $O(1+\log(n+1))$ です。

## 使用例

```cpp
const std::vector<int> values{4, 7, 4, 2};
sirkit::PointSetRangeFrequency<int> frequency(values);
frequency.count(1, 4, 4); // 1
frequency.set(3, 4);
frequency.count(1, 4, 4); // 2
frequency.get(3);        // 4
```

[使用例](../Examples/point_set_range_frequency_example.cpp) ／ [Verify](../Verify/current/submissions/point_set_range_frequency.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/point_set_range_frequency)
