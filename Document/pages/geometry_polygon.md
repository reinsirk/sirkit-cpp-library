# 多角形と凸包

`Geometry/Polygon.cpp`は、面積・周長・内外判定・凸包・面積重心・凸切断を提供します。通常の操作は`sirkit::geo`内、`convex_hull_sorted`・`signed_triangle_area2`・`triangle_area`は`sirkit`直下です。点と向きの判定、`Line`・`Segment`・`on_segment`も同梱します。直線同士の交点計算や円の操作は対応ファイルを併用してください。

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

### geo::signed_area2

```text
template<class T, class C> C signed_area2(const std::vector<Point<T, C>>& poly);
```

多角形の符号付き面積の2倍を計算型Cで返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N)時間。

### geo::area

```text
template<class T, class C> long double area(const std::vector<Point<T, C>>& poly);
```

多角形の面積の絶対値を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N)時間。

### geo::perimeter

```text
template<class T, class C> long double perimeter(const std::vector<Point<T, C>>& poly);
```

多角形を一周する辺の長さの合計を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N)時間。

### geo::contains

```text
template<class T, class C> Location contains(const std::vector<Point<T, C>>& poly, const Point<T, C>& p, Epsilon eps = {});
```

点が多角形の外部・境界・内部のどこにあるかをLocationで返します。

**制約:** 周回順の単純多角形と有限座標の点。

**計算量:** 頂点数NにO(N)。

### geo::convex_hull

```text
template<class T, class C> std::vector<Point<T, C>> convex_hull(std::vector<Point<T, C>> points, Epsilon eps = {});
```

辞書順最小点から反時計回りの凸包を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N log(N+1))時間・O(N)領域。

### convex_hull_sorted

```text
template<class T, class C> std::vector<geo::Point<T, C>> convex_hull_sorted(std::vector<geo::Point<T, C>> points, geo::Epsilon eps = {});
```

辞書順に整列した点列から凸包を求めます。

**制約:** 入力は座標の辞書順で非減少。

**計算量:** O(N)。

### geo::centroid

```text
template<class T, class C> std::optional<RealPoint> centroid(const std::vector<Point<T, C>>& poly, Epsilon eps = {});
```

多角形の面積重心をoptionalで返します。

**制約:** 単純多角形。面積が0ならnullopt。

**計算量:** O(N)時間。

### geo::convex_cut

```text
template<class T, class C> std::vector<RealPoint> convex_cut(const std::vector<Point<T, C>>& poly, const Line<T, C>& line, Epsilon eps = {});
```

凸多角形と直線の左側との共通部分を返します。

**制約:** 周回順の凸多角形と非零長の直線。

**計算量:** O(N)時間。

### signed_triangle_area2

```text
template<class T, class C> constexpr C signed_triangle_area2(const geo::Point<T, C>& a, const geo::Point<T, C>& b, const geo::Point<T, C>& c);
```

三角形の符号付き面積の2倍を計算型Cで返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。

### triangle_area

```text
template<class T, class C> long double triangle_area(const geo::Point<T, C>& a, const geo::Point<T, C>& b, const geo::Point<T, C>& c);
```

三角形の面積の絶対値を返します。

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


多角形は`std::vector<geo::Point<T,C>>`で、先頭点を末尾へ重複させる必要はありません。以下のNは入力の頂点数です。


```cpp
using namespace sirkit;
std::vector<geo::PointI> points{{0, 0}, {4, 0}, {0, 3}, {1, 1}};
auto hull = geo::convex_hull(points);
auto area = geo::area(hull);                        // 6
auto location = geo::contains(hull, geo::PointI{1, 1}); // Inside
auto center = geo::centroid(hull);                  // {4/3,1}
```

## 前提

整数の差・積・和が計算型`C`に収まる入力を渡します。`signed_area2`は反時計回りなら正、空列なら0を返します。`contains`と`centroid`には単純多角形を渡します。`convex_cut`には頂点を周回順に並べた凸多角形と、長さが零でない有向直線を渡します。

凸包は完全一致の重複点を除き、一直線の入力では両端だけを返します。`convex_hull_sorted`の入力は辞書順で非減少に並べておきます。重心は3点未満または面積0なら`nullopt`です。点の操作と浮動判定の許容幅は[Point](geometry_point.md)と共通です。

詳しくは[面積・三角形・重心](../geometry_area.md)・[内外判定](../geometry_point_location.md)・[凸包](../convex_hull.md)を参照してください。[全機能版の使用例](../../Examples/geometry_example.cpp)もあります。
