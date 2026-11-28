# PersistentLiChaoTree

直線やx座標を制限した線分を追加し、指定したx座標での最小値を求めます。追加するたびに新しい版ができ、以前の版から分岐できます。

```cpp
sirkit::PersistentLiChaoTree tree({0, 1, 3});
const auto a = tree.add_line({}, {2, 5});
const auto b = tree.add_segment(a, 0, 3, {0, 2});
tree.get(a, 1); // 7
tree.get(b, 1); // 2
tree.get(b, 3); // 11
```

[使用例](../Examples/persistent_li_chao_example.cpp)

## `PersistentLiChaoTree`

```text
PersistentLiChaoTree<T = std::int64_t>
explicit PersistentLiChaoTree(std::vector<T> coordinates)
```

照会する全x座標を渡します。重複した座標はまとめます。入力の座標数を `m`、重複を除いた数を `n` とすると、構築は `O(m log(m+1))` 時間と `O(m)` 領域です。

`Version{}` は空の版です。更新後の版は作成した木で使います。木を破棄したり、別の木を代入したりすると、それまでの版は使えなくなります。

`Line{slope, intercept}` は `y = slope * x + intercept` を表します。型 `T` は符号付き整数で、掛け算と足し算の結果がともに `T` に収まるようにします。

## `add_line`

```text
Version add_line(Version version, Line line)
```

全登録座標で有効な直線を追加し、新しい版を返します。償却 `O(log(n+1))` 時間と追加領域を使います。

## `add_segment`

```text
Version add_segment(Version version, T left, T right, Line line)
```

`left <= x < right` の登録座標だけで有効な直線を追加します。`left <= right` が必要です。償却 `O(log²(n+1))` 時間と追加領域を使います。

## `get`

```text
std::optional<T> get(Version version, T x) const
```

指定した版の `x` における最小値を `O(log(n+1))` 時間で返します。登録外のx座標や、有効な直線がない座標には `std::nullopt` を返します。
