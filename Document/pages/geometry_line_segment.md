# 直線・線分の交差と距離

`Geometry/Line_Segment.cpp`は、`sirkit::geo`の直線・閉線分の交差、射影、最近点、距離を提供します。点・ベクトルと三点の向きの判定も含むため、このファイルだけで使えます。多角形と円の操作は別ファイルです。

PointのValueは座標型T、Calcは計算型C、Realは浮動座標ならT・整数座標ならlong doubleです。

## API

### geo::predicate_sign

```text
template<class T, class C> int predicate_sign(C value, C scale, Epsilon eps = {});
```

整数は厳密、実数は許容幅で値の符号を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::orientation

```text
template<class T, class C> int orientation(const Point<T, C>& a, const Point<T, C>& b, const Point<T, C>& c, Epsilon eps = {});
```

三点の向きを左折1・直進0・右折-1で返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::to_real

```text
template<class T, class C> RealPoint to_real(const Point<T, C>& p);
```

long double座標の点へ変換します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::distance

```text
template<class T, class C> typename Point<T, C>::Real distance(const Point<T, C>& p, const Point<T, C>& q);
template<class T, class C> long double distance(const Point<T, C>& p, const Line<T, C>& l);
template<class T, class C> long double distance(const Point<T, C>& p, const Segment<T, C>& s);
template<class T, class C> long double distance(const Segment<T, C>& s, const Segment<T, C>& t, Epsilon eps = {});
```

2点、点と直線・線分、または線分同士の距離を返します。

**制約:** 直線の2点は相異なること。零長線分は可。計算型で中間値を表せること。

**計算量:** O(1)。

### geo::on_segment

```text
template<class T, class C> bool on_segment(const Point<T, C>& p, const Segment<T, C>& s, Epsilon eps = {});
```

端点を含めて点が閉線分上にあればtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::intersects

```text
template<class T, class C> bool intersects(const Segment<T, C>& s, const Segment<T, C>& t, Epsilon eps = {});
```

閉線分が交差すればtrueを返します。端点接触と重なりを含みます。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::projection

```text
template<class T, class C> RealPoint projection(const Point<T, C>& p, const Line<T, C>& l);
```

直線への射影を返します。

**制約:** 直線の方向はlong doubleへの変換後も非零。中間値は計算型内。

**計算量:** O(1)。

### geo::reflection

```text
template<class T, class C> RealPoint reflection(const Point<T, C>& p, const Line<T, C>& l);
```

直線に関する対称点を返します。

**制約:** 直線の方向はlong doubleへの変換後も非零。中間値は計算型内。

**計算量:** O(1)。

### geo::closest_point

```text
template<class T, class C> RealPoint closest_point(const Point<T, C>& p, const Segment<T, C>& s);
```

閉線分上の最近点を返します。

**制約:** 有限座標の閉線分。零長なら端点を返す。

**計算量:** O(1)。

### geo::intersection

```text
template<class T, class C> LineIntersection intersection(const Line<T, C>& l, const Line<T, C>& m, Epsilon eps = {});
template<class T, class C> SegmentIntersection intersection(const Segment<T, C>& s, const Segment<T, C>& t, Epsilon eps = {});
```

交差種別と近似交点を返します。

**制約:** 有限座標・非負有限の許容幅。直線の方向は非零、円の半径は非負。対象ファイルに含む型を渡す。

**計算量:** O(1)。

### geo::Epsilon::tolerance

```text
constexpr long double tolerance(long double scale) const;
```

abs + rel * abs(scale)を返します。

**制約:** absとrelは非負有限（既定は各1e-12L）。scaleは有限。

**計算量:** O(1)。

### geo::Point::operator==

```text
constexpr bool operator==(const Point& p) const;
```

値と形が等しいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator!=

```text
constexpr bool operator!=(const Point& p) const;
```

等しくなければtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator<

```text
constexpr bool operator<(const Point& p) const;
```

辞書順で小さいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator>

```text
constexpr bool operator>(const Point& p) const;
```

辞書順で大きいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator<=

```text
constexpr bool operator<=(const Point& p) const;
```

辞書順で小さいか等しいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator>=

```text
constexpr bool operator>=(const Point& p) const;
```

辞書順で大きいか等しいかを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator+

```text
constexpr Point operator+(const Point& p) const;
```

対応する座標を加えた点を返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::operator-

```text
constexpr Point operator-(const Point& p) const;
constexpr Point operator-() const;
```

差を返します。単項版は符号を反転します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::operator+=

```text
constexpr Point& operator+=(const Point& p);
```

加算して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator-=

```text
constexpr Point& operator-=(const Point& p);
```

減算して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator*

```text
constexpr Point operator*(T k) const;
friend constexpr Point operator*(T k, const Point& p);
constexpr C operator*(const Point& p) const;
```

スカラー倍、または点同士の内積を返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。整数座標に浮動小数点スカラーは渡せません。

**計算量:** O(1)。

### geo::Point::operator*=

```text
constexpr Point& operator*=(T k);
```

スカラー倍で自身を更新します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。整数座標に浮動小数点スカラーは渡せません。

**計算量:** O(1)。

### geo::Point::operator/

```text
Point operator/(T k) const requires std::is_floating_point_v<T>;
```

商を返します。

**制約:** 浮動小数点座標で、スカラーは非零。

**計算量:** O(1)。

### geo::Point::operator/=

```text
Point& operator/=(T k) requires std::is_floating_point_v<T>;
```

除算して自身を更新します。

**制約:** 浮動小数点座標で、スカラーは非零。

**計算量:** O(1)。

### geo::Point::dot

```text
constexpr C dot(const Point& p) const;
```

内積を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::cross

```text
constexpr C cross(const Point& p) const;
```

外積を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::operator^

```text
constexpr C operator^(const Point& p) const;
```

外積を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::norm2

```text
constexpr C norm2() const;
```

長さの二乗を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::norm

```text
Real norm() const;
```

長さを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::dist2

```text
constexpr C dist2(const Point& p) const;
```

点までの距離の二乗を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::dist

```text
Real dist(const Point& p) const;
```

点までの距離を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::orient

```text
constexpr C orient(const Point& b, const Point& c) const;
```

この点を基準にした三点の外積を計算型Cで返します。

**制約:** 座標演算はT、差・積・和は計算型Cに収まること。

**計算量:** O(1)。

### geo::Point::ccw

```text
constexpr int ccw(const Point& b, const Point& c) const;
```

向きの符号を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::rotated90

```text
constexpr Point rotated90(int k = 1) const;
```

90度の整数倍だけ回転したコピーを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::rot90

```text
constexpr void rot90(int k = 1);
```

90度の整数倍だけ回転して自身を更新します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::arg

```text
Real arg() const;
```

偏角をラジアンで返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::rotated

```text
Point rotated(T angle) const requires std::is_floating_point_v<T>;
```

指定ラジアンだけ回転したコピーを返します。

**制約:** 浮動小数点座標、角度は有限。

**計算量:** O(1)。

### geo::Point::rot

```text
void rot(T angle) requires std::is_floating_point_v<T>;
```

指定ラジアンだけ回転して自身を更新します。

**制約:** 浮動小数点座標、角度は有限。

**計算量:** O(1)。

### geo::Point::normalized

```text
Point normalized() const requires std::is_floating_point_v<T>;
```

同じ方向の単位ベクトルを返します。

**制約:** 浮動小数点座標で零ベクトルではない。

**計算量:** O(1)。

### geo::Point::almost_equal

```text
bool almost_equal(const Point& p, Epsilon eps = {}) const requires std::is_floating_point_v<T>;
```

許容幅で各座標を比較します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::cast

```text
template<class U, class D = DefaultCalc<U>> constexpr Point<U, D> cast() const;
```

座標型と計算型を変えた点を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::arg_cmp

```text
static bool arg_cmp(const Point& a, const Point& b);
```

零点を先頭に偏角・距離順で比較します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::Point::operator<<

```text
friend std::ostream& operator<<(std::ostream& out, const Point& p);
```

ストリームへ値を出力します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


`Line<T,C>{a,b}`は相異なる2点で表す直線、`Segment<T,C>{a,b}`は両端を含む線分です。線分は零長でも使えます。


`LineIntersection`は`kind`と`optional<RealPoint>`の`point`、`SegmentIntersection`は`kind`と`points`を所有します。重なりの座標は元の座標で辞書順に選んだ両端点です。[全機能版の使用例](../../Examples/geometry_example.cpp)でも線分交差を確認できます。

```cpp
using namespace sirkit;
geo::Segment<std::int64_t> s{{0, 0}, {4, 0}};
geo::Segment<std::int64_t> t{{2, -1}, {2, 1}};
bool hit = geo::intersects(s, t);             // true
auto result = geo::intersection(s, t);        // Point、{2,0}
auto nearest = geo::closest_point(geo::PointI{2, 3}, s);
auto distance = geo::distance(geo::PointI{2, 3}, s); // 3
```

## 前提

点の座標型・計算型の前提は[Point](geometry_point.md)と同じです。整数の交差判定は、差・積などが計算型に収まる範囲で厳密です。返す交点・射影・最近点・距離は`long double`による近似です。

直線を受ける射影・対称点・距離・交点計算では、`long double`へ変換した後も方向が非零であることが必要です。零長線分の最近点はその端点、点との距離は端点までの距離です。

詳しくは[交差判定と戻り値](../geometry_segment_intersection.md)・[距離と近似計算](../geometry_point_segment_distance.md)を参照してください。
