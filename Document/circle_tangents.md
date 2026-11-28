# 円の接線

`geo::point_circle_tangents(c, p)` は点 `p` を通る接線を返します。点が円の内側・円周上・外側にあるとき、それぞれ 0・1・2 本です。

`geo::circle_tangents(a, b)` は 2 円の内・外共通接線を返します。同心同半径の場合は `std::nullopt` で、同心で半径が異なる場合は空の配列です。

~~~cpp
using namespace sirkit::geo;
Circle a{{0, 0}, 1}, b{{5, 0}, 2};
auto tangents = circle_tangents(a, b);
auto tangent = tangents->front();
auto contact = tangent.first; // a 上の接点
auto unit_normal = tangent.normal;
~~~

接線の方程式は `normal.dot(x - first) = 0` です。`second` は 2 番目の円上の接点、または指定点 `p` です。
半径は正で、演算結果が有限の範囲で使います。`Epsilon` の許容幅内では接触とみなし、接点を近似します。同心円の半径差が許容幅以内の場合も `std::nullopt` です。各操作は `O(1)` です。[AtCoder Algorithm Lecturesの直線・線分・円](https://info.atcoder.jp/entry/algorithm_lectures/line_segment_circle)
