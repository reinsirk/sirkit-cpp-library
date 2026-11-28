# 一点更新と最長同値区間

配列の要素を一つずつ変えながら、ある区間に同じ値が何個連続しているかを求めます。たとえば `{1, 1, 2, 2, 2, 3}` 全体で最長の連続長は3です。

## `RangeEqualRuns`

```cpp
RangeEqualRuns();
explicit RangeEqualRuns(std::span<const T> values);
```

長さ $n$ の配列を保持します。構築は $O(n+1)$ 時間・領域で、長さは `INT_MAX` 以下です。型 `T` はコピー構築でき、例外なしで交換できるものとし、`==` は同値関係を表す必要があります。要素のコピー・比較・交換をそれぞれ定数時間として計算量を示します。

## `EqualRunLengths`

`query` と `all` はこの型を返します。`length` は調べた要素数、`prefix` と `suffix` は左端・右端での同値連続長、`longest` は区間内で最も長い同値連続長です。空区間では全項目が0になります。

## `size`

配列の長さを $O(1)$ 時間で返します。

## `empty`

空の配列かどうかを $O(1)$ 時間で返します。

## `get`

```cpp
T get(std::size_t i) const;
```

位置 `i` の値を $O(1)$ 時間でコピーして返します。

## `set`

```cpp
void set(std::size_t i, T value);
```

位置 `i` の値を変更します。隣接する2箇所の等しさだけを更新するため、$O(\log(n+1))$ 時間です。コピーや比較が例外を投げた場合は元の状態を保ちます。

## `query`

```cpp
EqualRunLengths query(std::size_t left, std::size_t right) const;
```

半開区間 `[left, right)` を $O(\log(n+1))$ 時間で集計します。添字は0始まりです。検査が有効なら、`get`、`set`、`query` の範囲外に対して `std::out_of_range` を送出します。

## `all`

配列全体を $O(1)$ 時間で集計します。

## 使用例

```cpp
const std::array values{1, 1, 2, 2, 2, 3};
sirkit::RangeEqualRuns<int> runs(values);
assert(runs.all().longest == 3);
runs.set(5, 2);
assert(runs.query(0, 6).longest == 4);
```

[実装](../DataStructure/Range_Equal_Runs.cpp)・[使用例の全体](../Examples/range_equal_runs_example.cpp)・[Verify](../Verify/current/submissions/range_equal_runs.cpp)・[同梱のSegment Tree](segment_tree.md)
