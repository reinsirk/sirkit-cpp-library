# 4点から作れる台形と平行四辺形の数

`count_trapezoids(points)` は、取り出した4点が面積の正の台形を作る組数を `trapezoids` に、平行四辺形を作る組数を `parallelograms` に返します。平行四辺形や長方形は、両方の個数に含まれます。

```cpp
std::array<sirkit::TrapezoidPoint, 4> square{{{0, 0}, {2, 0}, {2, 2}, {0, 2}}};
auto counts = sirkit::count_trapezoids(square); // {1, 1}
```

点は相異なる `pair<int32_t, int32_t>` で、3点以上が同一直線にあっても構いません。異なる直線上の平行な辺を数え、2組の平行辺を持つ平行四辺形の重複を引きます。点数 $n$ が92,682以下で、時間は $O(n^2\log(n+1))$、領域は $O(n^2+1)$ です。

[使用例](../Examples/trapezoid_count_example.cpp) ／ [Verify](../Verify/current/submissions/trapezoid_count.cpp)
