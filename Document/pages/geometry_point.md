# 点とベクトル

`Geometry/Point.cpp`は、`sirkit::geo`の点・ベクトルと三点の向きの判定を提供します。直線・線分・多角形・円の型はこのファイルには含みません。[全機能版の使用例](../../Examples/geometry_example.cpp)でも点の操作を確認できます。

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

自身とpの座標が完全に一致するかを返します。

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

整数の符号は厳密に判定し、実数の符号は許容幅を使って判定します。

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


`Point<T,C>`の`T`は符号付き整数または浮動小数点の座標型、`C`は内積・外積などの計算型です。`PointI`は`std::int64_t`、`PointD`は`double`、`RealPoint`は`long double`の座標を使います。


浮動小数点座標では除算・ラジアン回転・単位化・許容幅による`almost_equal`も使えます。全操作の引数・戻り値・制約はこのページのファイル別API一覧にまとめています。

```cpp
using namespace sirkit;
geo::PointI a{3, 4}, b{0, 0};
auto squared = a.dist2(b);                    // 25
long double length = geo::distance(a, b);     // 5
auto turned = a.rotated90();                 // {-4, 3}
auto turn = geo::orientation(b, a, turned);  // 1
```

## 前提

入力座標は有限で、座標演算の結果は`T`、差・積・和は`C`に収まることが必要です。整数の既定計算型は対応GCC／Clangでは128ビット拡張、標準代替では`std::int64_t`です。拡張型を使う場合も整数の全域で全演算を保証するものではありません。

比較演算子は完全一致・辞書順です。`Epsilon{abs,rel}`を受ける判定だけが許容幅を使います。浮動小数点の零除算と零ベクトルの単位化は、検査を省く設定でも拒否します。

詳しくは[点の全操作](../geometry_point.md)・[三点の向き](../geometry_orientation.md)を参照してください。
