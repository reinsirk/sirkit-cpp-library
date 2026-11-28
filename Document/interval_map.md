# 同値区間で管理する配列

同じ値が連続する部分を一つの区間にまとめます。区間代入の後は隣の同値区間も結合するため、変更後の区間の端をそのまま調べられます。

## `IntervalMap`

```cpp
IntervalMap();
IntervalMap(std::size_t n, const T &value);
explicit IntervalMap(std::span<const T> values);
```

空列と一様な列は $O(1)$、初期列からの構築は長さを $n$ として $O(n+1)$ 時間です。型 `T` はコピー構築でき、`==` が同値関係を表すとします。領域は保有する区間数に比例します。コピーは独立し、ムーブ元は空の列です。

以下で位置は0始まり、区間は半開です。検査が有効な場合、範囲外の位置や区間に対して `std::out_of_range` を送出します。

## `size`

列の長さを $O(1)$ 時間で返します。

## `empty`

列が空か $O(1)$ 時間で返します。

## `run_count`

極大な同値区間の数を $O(1)$ 時間で返します。

## `get`

```cpp
const T &get(std::size_t i) const;
```

位置 `i` の値を参照します。区間数を $R$ として $O(\log(R+1))$ 時間です。返した参照を保持する場合は、更新や代入に先立って値をコピーしてください。

## `run_at`

```cpp
Run run_at(std::size_t i) const;
```

`i` を含む最大の同値区間を `Run{left, right, value}` で返します。返る値はコピーです。$O(\log(R+1))$ 時間です。

## `assign`

```cpp
void assign(std::size_t left, std::size_t right, const T &value);
```

`[left, right)` を `value` に置き換えます。消える境界数を $K$ として $O(\log(R+1)+K)$ 時間です。空区間なら状態を保ちます。比較や確保が失敗しても元の列を保ちます。

## `runs`

```cpp
std::vector<Run> runs() const;
```

全区間を左端順にコピーして返します。$O(R+1)$ 時間・領域です。

## 使用例

```cpp
sirkit::IntervalMap<int> values(8, 0);
values.assign(1, 4, 7);
values.assign(4, 6, 7);
const auto run = values.run_at(5);
assert(run.left == 1 && run.right == 6 && run.value == 7);
```

[実装](../DataStructure/Interval_Map.cpp)・[使用例の全体](../Examples/interval_map_example.cpp)・[塗り替えのVerify](../Verify/current/submissions/interval_map_paint.cpp)・[区間代入のVerify](../Verify/current/submissions/interval_map_range_assign.cpp)
