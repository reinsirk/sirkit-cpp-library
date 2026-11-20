# 平面幾何（全機能版）

`Geometry/geometory.cpp`は点・ベクトル、直線・線分、多角形・凸包、円の操作をまとめたファイルです。通常の公開APIは`sirkit::geo`、`convex_hull_sorted`・`signed_triangle_area2`・`triangle_area`・`circle_intersection_kind`は`sirkit`直下にあります。共有ガードで同じ部品の再定義を防ぐため、各用途別ファイルと併用できます。

PointのValueは座標型T、Calcは計算型C、Realは浮動座標ならT・整数座標ならlong doubleです。

## API

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
std::vector<RealPoint> intersection(const Circle& c, const Line<long double>& l, Epsilon eps = {});
std::vector<RealPoint> intersection(const Circle& c, const Segment<long double>& s, Epsilon eps = {});
CircleIntersection intersection(const Circle& a, const Circle& b, Epsilon eps = {});
```

直線・線分・円周の交差種別と近似交点を返します。

**制約:** 有限座標・非負有限の許容幅。直線の方向は非零、円の半径は非負。対象ファイルに含む型を渡す。

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

多角形の閉じた辺の総延長を返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(N)時間。

### geo::contains

```text
template<class T, class C> Location contains(const std::vector<Point<T, C>>& poly, const Point<T, C>& p, Epsilon eps = {});
Location contains(const Circle& c, const RealPoint& p, Epsilon eps = {});
```

点が円盤・多角形の外部・境界・内部のどこにあるかをLocationで返します。

**制約:** 円盤は非負の有限半径、多角形は単純多角形。点は有限。

**計算量:** 円盤O(1)、多角形は頂点数NにO(N)。

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

辞書順整列済みのgeo::Pointから反時計回りの凸包を返す。

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

### circle_intersection_kind

```text
geo::CircleIntersectionKind circle_intersection_kind(const geo::Circle& a, const geo::Circle& b, geo::Epsilon eps = {});
```

円周同士の交差種別だけを返します。

**制約:** 共通の型・入力条件を満たすこと。

**計算量:** O(1)。


## 機能群


全公開操作の引数・戻り値・前提は、このページのファイル別API一覧で確認できます。必要な機能だけを使う場合は、用途別ファイルのガイドも参照してください。

```cpp
using namespace sirkit;
std::vector<geo::PointI> points{{0, 0}, {4, 0}, {0, 3}};
auto hull = geo::convex_hull(points);
auto area = geo::area(hull);                         // 6
geo::Segment<std::int64_t> side{{0, 0}, {4, 0}};
auto d = geo::distance(geo::PointI{2, 3}, side);      // 3
geo::Circle circle{{0, 0}, 5};
auto position = geo::contains(circle, geo::RealPoint{1, 1}); // Inside
```

## 共通の前提

点の座標は有限で、座標演算の結果は座標型`T`、整数の差・積・和は計算型`C`に収まることが必要です。整数の既定計算型は対応GCC／Clangでは128ビット拡張、標準代替では`std::int64_t`です。交点・射影・重心などの座標は主に`long double`による近似です。

直線は相異なる2点で指定します。線分は両端を含み、零長も扱えます。円の半径は非負です。内外判定や重心を求めるときは単純多角形、凸切断では頂点を周回順に並べた凸多角形を渡します。浮動判定の`Epsilon{abs,rel}`は有限の非負値で指定します。

型・関数名の重なりは引数で区別します。例えば`contains`は多角形と円盤の内外、`intersection`は直線・線分・円周の交点を扱います。返却される交点列・凸包・切断後の多角形は戻り値が所有します。

[完全な使用例](../../Examples/geometry_example.cpp)と[詳細文書の一覧](../../Geometry/README.md)を参照してください。
