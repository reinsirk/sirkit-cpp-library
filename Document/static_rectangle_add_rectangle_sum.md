# 静的な長方形加算・長方形和

格子上の長方形に重みを加え、すべての加算を終えた後で長方形内の総和を求めます。1件の追加が照会に寄与する量は、2つの長方形の重なりの面積に重みを掛けた値です。

長方形は半開 `[left,right) × [bottom,top)` です。座標と重みは符号付き64ビット整数で、負の値も使えます。幅または高さが0の追加は寄与せず、空の照会の答えは0です。入力検査が有効な場合、左右または上下が逆の長方形には `std::invalid_argument` を送出します。

## `StaticRectangleAdd / StaticRectangleSumQuery`

```text
struct StaticRectangleAdd { std::int64_t left, bottom, right, top, weight; };
struct StaticRectangleSumQuery { std::int64_t left, bottom, right, top; };
```

`StaticRectangleAdd` は範囲内の整数座標ごとに重みを加え、`StaticRectangleSumQuery` は範囲内の和を照会します。

## `static_rectangle_add_rectangle_sum`

```text
template <std::uint32_t Mod = 998244353>
std::vector<std::uint32_t> static_rectangle_add_rectangle_sum(
    std::span<const StaticRectangleAdd> additions, std::span<const StaticRectangleSumQuery> queries);
```

全追加後の長方形和を照会順に返します。各値は法 `Mod` での非負剰余です。`Mod` は正の符号なし32ビット整数とし、追加数は $2^{31}-1$ 以下です。 追加数 $n$、照会数 $q$ に対して $O((n+q)\log(n+q+1))$ 時間・$O(n+q)$ 領域です。

## 使用例

```cpp
const std::vector<sirkit::StaticRectangleAdd> additions{{0, 0, 3, 2, 5}};
const std::vector<sirkit::StaticRectangleSumQuery> queries{{1, 0, 4, 2}, {3, 0, 4, 2}};
const auto answers = sirkit::static_rectangle_add_rectangle_sum(additions, queries);
// answers は {20, 0}
```

[使用例](../Examples/static_rectangle_add_rectangle_sum_example.cpp) ／ [Verify](../Verify/current/submissions/static_rectangle_add_rectangle_sum.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/static_rectangle_add_rectangle_sum)
