# 整数ベクトルの偏角ソート

`Geometry/angle_sort.cpp`を貼り付けて使います。点は`std::pair<std::int64_t,std::int64_t>`の`{x,y}`で表し、各座標の全域に対応します。関数と比較器は`namespace sirkit`内に定義されます。

## API

### angle_cross_sign

```text
constexpr int angle_cross_sign( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

外積の符号を-1/0/1で返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### arccmp_less

```text
constexpr bool arccmp_less( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

偏角が厳密に小さいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### arccmp_g

```text
constexpr bool arccmp_g( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

偏角が厳密に大きいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### arccmp_less_eq

```text
constexpr bool arccmp_less_eq( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

偏角が小さいか等しいかを返します。

**制約:** std::sortの比較器に使用しない。

**計算量:** O(1)。

### arccmp_g_eq

```text
constexpr bool arccmp_g_eq( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

偏角が大きいか等しいかを返します。

**制約:** std::sortの比較器に使用しない。

**計算量:** O(1)。

### same_angle

```text
constexpr bool same_angle( const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) noexcept;
```

同じ方向ならtrueを返します。零ベクトルは零ベクトルとのみ同方向です。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### AngleLess::operator()

```text
constexpr bool operator()(const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) const noexcept;
```

偏角の厳密な昇順の比較結果を返します。

**制約:** 座標はint64_tのpair。

**計算量:** O(1)。

### AngleGreater::operator()

```text
constexpr bool operator()(const std::pair<std::int64_t, std::int64_t>& a, const std::pair<std::int64_t, std::int64_t>& b) const noexcept;
```

偏角の厳密な降順の比較結果を返します。

**制約:** 座標はint64_tのpair。

**計算量:** O(1)。

### angle_sort

```text
void angle_sort(std::vector<std::pair<std::int64_t, std::int64_t>>& p, bool descending = false);
```

点列を偏角順に並べ替えます。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N log(N+1))時間。添字版はO(N)追加領域。

### angle_sort_indices

```text
std::vector<std::size_t> angle_sort_indices( const std::vector<std::pair<std::int64_t, std::int64_t>>& p, bool descending = false);
```

入力を変えず偏角順の添字を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N log(N+1))時間。添字版はO(N)追加領域。


偏角は+x方向から反時計回りの`[0,2π)`です。昇順では零ベクトルを先頭、降順では末尾に置きます。同じ方向の長さによる順序は定めません。零ベクトルは零ベクトルとのみ同方向です。


比較と方向判定はO(1)、ソートはO(N log N)の上界です。添字列の作成はO(N)の追加領域を使い、同じ偏角の点同士の順序は保証しません。`arccmp_less_eq`と`arccmp_g_eq`は厳密な比較ではないため、`std::sort`の比較器には使えません。

整数の積を安全な範囲で比較し、必要な場合だけ広い演算を使います。GCC/Clangの128ビット整数拡張が利用できる環境では、その拡張を使います。標準C++の型ではありません。`SIRKIT_FORCE_STANDARD_ARITHMETIC`を定義するか拡張が利用できない場合は、符号と符号なし整数の分割積による標準C++の経路へ切り替わります。

大きい列では、整列済みなら確認の走査だけで完了します。少数の乱れは移動回数を制限した挿入処理で直し、短い整列済みの末尾は定数領域で前半とマージします。途中で打ち切った場合も点の多重集合を保ち、半平面分割と`std::sort`へ進みます。整列済み入力はO(N)時間です。

添字版も半平面と零を先に分け、全座標が32bit符号付き整数に収まる場合は64bitの積で比較します。並べ替えるのは添字配列だけで、入力とO(N)の追加領域という前提を保ちます。

有効な座標型の全域を扱うため入力検査は不要で、`SKIP_RUNTIME_CHECKS`によって動作は変わりません。

```cpp
using namespace sirkit;

std::vector<std::pair<std::int64_t, std::int64_t>> points{
    {0, -1}, {1, 0}, {0, 0}, {0, 1}
};
angle_sort(points); // {0,0}, {1,0}, {0,1}, {0,-1}
auto order = angle_sort_indices(points, true); // 入力を変えず降順の添字を得る
```
