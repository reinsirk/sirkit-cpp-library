# 多角形と三角形の面積

`Geometry/Polygon.cpp`を貼り付けて使う。`geo::signed_area2(polygon)`は符号付き面積の2倍を計算型`C`で返す。頂点が反時計回りなら正となる。`geo::area(polygon)`は面積の絶対値を`long double`で返す。先頭頂点から多角形を三角形に分けて計算し、時間はO(N)、追加領域はO(1)。隣り合う三角形で頂点の座標差を再利用し、符号付き和の加算順は保つ。頂点数が0、1、2の場合、面積は0。自己交差する入力の幾何学的な領域面積は保証しない。

三点が既にある場合は`triangle_area(a,b,c)`で面積、`signed_triangle_area2(a,b,c)`で符号付き面積の2倍を求める。時間はO(1)で、配列や動的確保は不要。退化した三角形の面積は0となる。

座標は有限で、差・積・外積の減算・多角形での累積和が`C`に収まることを前提とする。整数の既定計算型と標準代替は三点の向きと共通。浮動小数点座標では近似値となる。

```cpp
geo::PointI a{0,0}, b{4,0}, c{0,3};
auto twice = signed_triangle_area2(a,b,c); // 12
auto area = triangle_area(a,b,c);         // 6
```

Verifyは[C - 直訴](https://atcoder.jp/contests/abc002/tasks/abc002_3)。一般多角形・空や退化・符号付き面積・追加APIは独立ローカル検査で補う。
