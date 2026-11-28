# 巡回シフトの転倒数と隣接交換

現在の列と、左へ巡回シフトした列の転倒数を扱います。たとえば `{2, 1, 3}` の転倒数は1で、左へ0回、1回、2回回した場合は順に `{1, 1, 3}` です。

## `CyclicInversionStatistics`

```cpp
CyclicInversionStatistics counts(values);
```

長さが分かるランダムアクセス範囲から構築します。比較関数は既定で `std::less<>{}`、`i < j` かつ `less(values[j], values[i])` の組が転倒です。比較は狭義弱順序を定める必要があり、同値の組は数えません。比較を定数時間とすると、構築は $O(n\log(n+1)+1)$ 時間・$O(n+1)$ 領域です。

集計には `std::int64_t` を使います。長さが2,642,246を超える場合は `std::length_error` を送出します。

## `size`

列の長さを $O(1)$ 時間で返します。

## `empty`

空の列かどうかを $O(1)$ 時間で返します。

## `inversion_count`

現在の列の転倒数を $O(1)$ 時間で返します。空列では0です。

## `rotation_inversion_sum`

全 $n$ 回転の転倒数の和を $O(1)$ 時間で返します。同じ列になる回転も開始位置ごとに数えます。空列での和は0です。

## `left_rotation_counts`

左へ0回から $n-1$ 回回したときの転倒数を順に返します。$O(n+1)$ 時間・領域です。

## `swap_adjacent`

```cpp
void swap_adjacent(std::size_t position);
```

0始まりの位置 `position` と `position+1` を交換します。転倒数と全回転の和を含め、更新は $O(1)$ 時間です。検査が有効なら、範囲外の位置には `std::out_of_range` を送出します。

## 使用例

```cpp
const std::array values{2, 1, 3};
sirkit::CyclicInversionStatistics counts(values);
assert(counts.rotation_inversion_sum() == 5);
counts.swap_adjacent(0);
assert(counts.rotation_inversion_sum() == 4);
```

[実装](../Algorithm/Cyclic_Inversion_Statistics.cpp)・[使用例の全体](../Examples/cyclic_inversion_statistics_example.cpp)・[各回転の Verify](../Verify/current/submissions/cyclic_rotation_inversions.cpp)・[隣接交換の Verify](../Verify/current/submissions/cyclic_rotation_inversions_adjacent_swaps.cpp)
