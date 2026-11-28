# KdTreePointExtractor

平面上の点をkd木に入れ、閉円や閉長方形に入る点を取り出します。同じ点を二度取り出さないので、近くの未訪問点を探すBFSに使えます。

```cpp
sirkit::KdTreePointExtractor points({{0, 1}, {2, 2}, {0, 1}});
std::vector<std::size_t> ids;
points.take_in_circle({0, 0}, 1, ids); // ID 0と2を取り出す。順序は未規定
points.take_in_rectangle({2, 1}, {3, 3}, ids); // ID 1を末尾へ追加
points.size(); // 0
```

[使用例](../Examples/kd_tree_point_extractor_example.cpp)

## `KdTreePointExtractor`

```text
explicit KdTreePointExtractor(const std::vector<Point> &points)
```

`Point` は `std::array<std::int32_t, 2>` です。入力順に0始まりのIDを付けます。座標が等しい点にも別のIDを付けます。点数を `n` とすると構築は平均 `O(n log(n+1))` 時間、領域は `O(n)` です。

## `take_in_circle`

```text
void take_in_circle(Point center, std::uint32_t radius, std::vector<std::size_t> &output)
```

円周と円の内部にある点を取り除き、IDを `output` の末尾に加えます。順序は未規定です。最悪 `O(n)` 時間です。

## `take_in_rectangle`

```text
void take_in_rectangle(Point lower, Point upper, std::vector<std::size_t> &output)
```

各座標が下端と上端の間にある点を、境界も含めて取り出します。各軸で `lower <= upper` が必要です。最悪 `O(n)` 時間です。

出力先の空きが残りの点数より少ない場合は、取り出す数を調べて必要な容量を確保します。再確保時は既存要素の移動時間もかかります。

## `size`

```text
std::size_t size() const
```

未取得の点数を `O(1)` で返します。
