# 円と交点

`Geometry/Circle.cpp`は、円・円盤の内外判定・外接円・円周の交点を提供します。通常の操作は`sirkit::geo`内、`circle_intersection_kind`は`sirkit`直下です。点と向きの判定、`Line`・`Segment`・`on_segment`も同梱し、このファイルだけで円と直線・線分を組み合わせられます。多角形や直線同士の交点計算は別ファイルです。

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
```

2点間の距離を返します。

**制約:** 有限座標で、差と距離を計算型で表せること。

**計算量:** O(1)。

### geo::on_segment

```text
template<class T, class C> bool on_segment(const Point<T, C>& p, const Segment<T, C>& s, Epsilon eps = {});
```

端点を含めて点が閉線分上にあればtrueを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::circumcircle

```text
template<class T, class C> std::optional<Circle> circumcircle(const Point<T, C>& a, const Point<T, C>& b, const Point<T, C>& c, Epsilon eps = {});
```

3点を通る円をoptionalで返します。一直線・重複点ならnulloptです。

**制約:** 有限座標。一直線・重複点を許し、その場合はnullopt。

**計算量:** O(1)。

### geo::validate

```text
void validate(const Circle& c);
```

円の中心と半径を検査し、不正ならdomain_errorを送出します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### geo::contains

```text
Location contains(const Circle& c, const RealPoint& p, Epsilon eps = {});
```

点が円盤の外部・境界・内部のどこにあるかをLocationで返します。

**制約:** 円は非負の有限半径、点はRealPointの有限座標。

**計算量:** O(1)。

### geo::intersection

```text
std::vector<RealPoint> intersection(const Circle& c, const Line<long double>& l, Epsilon eps = {});
std::vector<RealPoint> intersection(const Circle& c, const Segment<long double>& s, Epsilon eps = {});
CircleIntersection intersection(const Circle& a, const Circle& b, Epsilon eps = {});
```

円周と直線・線分・円周の交点を返します。

**制約:** 有限座標・非負有限の許容幅。直線の方向は非零、円の半径は非負。対象ファイルに含む型を渡す。

**計算量:** O(1)。

### circle_intersection_kind

```text
geo::CircleIntersectionKind circle_intersection_kind(const geo::Circle& a, const geo::Circle& b, geo::Epsilon eps = {});
```

円周同士の交差種別だけを返します。

**制約:** 共通の型・入力条件を満たすこと。

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


`Circle{center,radius}`は有限の中心と非負の有限半径を持ち、半径0は点を表します。中心と返却交点には`RealPoint`（`long double`座標）を使います。円盤の`contains`には`RealPoint`、円との`intersection`には`Line<long double>`・`Segment<long double>`を渡します。整数の点は`to_real`で変換できます。


```cpp
using namespace sirkit;
geo::Circle a{{0, 0}, 5}, b{{6, 0}, 5};
auto kind = circle_intersection_kind(a, b);      // TwoPoints
auto hit = geo::intersection(a, b);             // 2交点
geo::Line<long double> axis{{-6, 0}, {6, 0}};
auto crossings = geo::intersection(a, axis);    // {-5,0}, {5,0}
auto disk = geo::contains(a, geo::RealPoint{0, 0}); // Inside
```

## 前提と戻り値

`CircleIntersection`は`kind`と`points`を所有します。種別は`None`・`Tangent`・`TwoPoints`・`Coincident`です。一致する正半径の円は`Coincident`で交点を格納せず、半径0で同じ位置の2円は`Tangent`と1交点です。円と直線／線分の交点は0・1・2点です。零長線分も扱えます。

許容幅は`Epsilon{abs,rel}`で指定します。交点は`long double`による近似であり、有限の入力でも途中の差・積などが計算型で表せることが必要です。`SKIP_RUNTIME_CHECKS`は円の入力検査を省きますが、直接呼ぶ`validate`は常に検査します。

詳しくは[円周同士の交差](../geometry_circle_intersection.md)・[同梱する点の操作](geometry_point.md)を参照してください。[全機能版の使用例](../../Examples/geometry_example.cpp)もあります。
