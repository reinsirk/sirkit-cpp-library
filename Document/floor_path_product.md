# Floor Sum のモノイド積

$y_i=\lfloor(ai+b)/m\rfloor$ でできる階段を左から進み、横と縦の1マスに対応する値を順に掛けます。文字列の連結、行列の積、写像の合成など、順序で結果が変わる演算にも使えます。

## `floor_path_product`

```text
template <class T, class Combine>
T floor_path_product(std::uint32_t n, std::uint32_t m, std::uint64_t a, std::uint64_t b,
                     const T &horizontal, const T &vertical, const T &identity, const Combine &combine)
```

最初に `vertical` を $y_0$ 回掛けます。続けて $i=0,\ldots,n-1$ の順に `horizontal` を1回、`vertical` を $y_{i+1}-y_i$ 回掛けます。`combine(left,right)` には結合則が必要です。`identity` は単位元です。

`m` は正、`a` と `b` は非負です。`m=0` は `std::invalid_argument` です。`n=0` なら最初の縦移動だけを集計します。`combine` は $O(\log(n+1)+\log(m+1)+\log(a+1)+\log(b+1))$ 回です。領域は $O(\log(m+1))$ 個の `T` です。

```cpp
const auto join = [](const std::string &s, const std::string &t) { return s + t; };
const auto path = sirkit::floor_path_product(3, 5, 2, 4, std::string("X"), std::string("Y"), std::string(), join);
// "XYXXY"
```

[使用例](../Examples/floor_path_product_example.cpp) ／ [Verify](../Verify/current/submissions/floor_path_product_abc429_g.cpp) ／ [参考解説](https://atcoder.jp/contests/arc183/editorial/10768)
