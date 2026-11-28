# 固定パターンが区間に現れる位置

同じパターンについて、文字列中の複数の区間から出現を探します。出現は区間に完全に収まるものを数えます。

## `PatternRangeOccurrences(text, pattern)`

KMP でパターンの開始位置を集め、位置ごとの累積個数を作ります。文字列長 $n$、パターン長 $m$ に対して $O(1+n+m)$ 時間・領域です。

## `size()`

文字列の長さです。$O(1)$ で取得できます。

## `pattern_size()`

パターンの長さです。$O(1)$ で取得できます。

## `count(l, r)`

0始まりの半開区間 `[l, r)` に収まる出現数を $O(1)$ で返します。重なる出現も対象です。空パターンは `l` から `r` までの各境界に現れます。

## `positions(l, r)`

`count` と同じ出現の開始位置を昇順の `std::span<const std::size_t>` で返します。$O(1)$ で取得でき、表を所有するオブジェクトが一つでも残る間は参照できます。

## 使用例

```cpp
const sirkit::PatternRangeOccurrences occurrence("ababa", "aba");
assert(occurrence.count(0, 5) == 2);
const auto starts = occurrence.positions(1, 5);
assert(starts.size() == 1 && starts[0] == 2);
```

[実装](../String/Pattern_Range_Occurrences.cpp)・[使用例の全体](../Examples/pattern_range_occurrences_example.cpp)・[Verify](../Verify/current/submissions/pattern_range_occurrences.cpp)
