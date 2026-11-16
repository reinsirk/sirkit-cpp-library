# 最近点対

整数座標の点列から、最短距離の二乗と2点の添字を求めます。座標は各値が`std::int32_t`に収まる整数です。

## API

### nearest_two_points

```text
template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel> std::optional<NearestTwoPointsResult> nearest_two_points(Iterator first, Sentinel last);
template<std::input_iterator Iterator> std::optional<NearestTwoPointsResult> nearest_two_points(Iterator first, std::ptrdiff_t count);
template<class Point, class Allocator> std::optional<NearestTwoPointsResult> nearest_two_points(const std::vector<Point, Allocator>& points);
```

`[first,last)`から`optional<NearestTwoPointsResult>`を返す。未整列でよい。

**制約:** 座標はint32_t内。個数は非負で指定範囲を参照できる。

**計算量:** O(N log(N+1))時間・O(N)追加領域。

### closest_pair

```text
template<std::input_iterator Iterator> std::uint64_t closest_pair(Iterator first, int n);
```

`int`個数の列から二乗距離だけを`std::uint64_t`で返す。

**制約:** 座標はint32_t内、n>=0でn個を参照できる。

**計算量:** O(N log(N+1))時間・O(N)追加領域。

### nearest_two_points_sorted

```text
template<std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel> std::optional<NearestTwoPointsResult> nearest_two_points_sorted(Iterator first, Sentinel last);
template<std::input_iterator Iterator> std::optional<NearestTwoPointsResult> nearest_two_points_sorted(Iterator first, std::ptrdiff_t count);
template<class Point, class Allocator> std::optional<NearestTwoPointsResult> nearest_two_points_sorted(const std::vector<Point, Allocator>& points);
```

`(x,y)`の辞書順で非減少の範囲を処理する。

**制約:** 座標はint32_t内、辞書順で非減少。個数は非負。

**計算量:** O(N log(N+1))時間・O(N)追加領域。


`P`は`std::pair<std::int32_t,std::int32_t>`です。ほかの整数pair-like型も使え、範囲版は単一パスiteratorに対応します。個数は非負で、その個数を参照できることが前提です。

結果の`distance_squared`は`std::uint64_t`、`first_index`・`second_index`は渡した列の0始まりの`std::size_t`で、前者が後者より小さくなります。同距離の対の選択は未規定です。sorted版も整列後の列の添字を返します。

## 計算量

N点にO(N log N)時間・O(N)追加領域です。入力を変更せず、結果は入力への参照を保持しません。

## 境界・例外

2点未満ではoptional版は`nullopt`、`closest_pair`は`INT32_MAX`を返します。最小二乗距離が`std::uint64_t`を超える場合は、検査省略時も`overflow_error`です。

既定では座標範囲、sorted版の整列順、負個数・逆順のsized範囲を検査します。`SKIP_RUNTIME_CHECKS`では利用側がこれらの前提を満たします。未読入力の検査や単一パスiteratorの巻戻しは保証しません。

```cpp
std::vector<P> points{{0,0}, {3,0}, {4,0}};
auto answer = nearest_two_points(points);
// distance_squared=1、first_index=1、second_index=2。
```

[完全な使用例](../Examples/nearest_two_points_example.cpp)
