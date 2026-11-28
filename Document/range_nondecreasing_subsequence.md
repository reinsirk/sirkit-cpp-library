# 一点更新と区間最長広義単調増加部分列

取りうる値が少ない列で、一点代入と区間照会を扱います。値が等しい要素も続けて選べます。たとえば `{2, 0, 1, 1, 2}` から `{0, 1, 1, 2}` を選ぶと長さは4です。

## `RangeNondecreasingSubsequence<A>`

```cpp
RangeNondecreasingSubsequence();
explicit RangeNondecreasingSubsequence(std::span<const int> symbols);
```

長さ $n$ の列を保持します。値は `0` 以上 `A` 未満で、`1 <= A <= 256`、`n <= INT_MAX` が必要です。構築は $O(A^3(n+1))$ 時間、保管は $O(A^2(n+1))$ 領域なので、小さい `A` に向きます。

## `size`

列の長さを $O(1)$ 時間で返します。

## `empty`

空の列かどうかを $O(1)$ 時間で返します。

## `set`

```cpp
void set(int position, int symbol);
```

位置 `position` の値を `symbol` に代入します。$O(A^3\log(n+1))$ 時間です。

## `longest`

```cpp
int longest(int left, int right) const;
int longest(int left, int right, int low, int high) const;
```

半開区間 `[left, right)` の最長広義単調増加部分列の長さを返します。4引数版では `low` 以上 `high` 以下の値から選びます。どちらも $O(A^3\log(n+1))$ 時間で、空区間の長さは0です。

## `replacement_count`

```cpp
int replacement_count(int left, int right) const;
```

半開区間 `[left, right)` を広義単調増加列にする最少置換回数です。区間長から `longest(left, right)` を引きます。$O(A^3\log(n+1))$ 時間です。

## `all`

列全体の最長広義単調増加部分列の長さを $O(A^2)$ 時間で返します。

## 使用例

```cpp
const std::array symbols{2, 0, 1, 1, 2};
sirkit::RangeNondecreasingSubsequence<3> range(symbols);
assert(range.replacement_count(0, 5) == 1);
range.set(0, 0);
assert(range.all() == 5);
```

位置の添字は0始まりです。検査を有効にすると、範囲外の位置・区間・値には `std::out_of_range`、`low > high` には `std::invalid_argument` を送出します。

[実装](../DataStructure/Range_Nondecreasing_Subsequence.cpp)・[使用例の全体](../Examples/range_nondecreasing_subsequence_example.cpp)・[Verify](../Verify/current/submissions/range_nondecreasing_subsequence.cpp)
