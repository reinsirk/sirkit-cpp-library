# ガウス整数の最大公約数

整数 $a,b$ を使って $a+bi$ と表す数をガウス整数と呼びます。`gaussian_gcd` は2つのガウス整数の最大公約数を求めます。

```cpp
auto g = sirkit::gaussian_gcd({5, 1}, {1, 3});  // 1+i
```

## `gaussian_gcd(a, b)`

```text
GaussianInteger gaussian_gcd(GaussianInteger a, GaussianInteger b);
```

入力の各成分は $[-2147483647,2147483647]$ とします。通常版で範囲外なら `std::out_of_range` を送出します。

最大公約数は $\pm1,\pm i$ 倍の違いを除いて一意です。結果が非零なら実部が正で虚部が非負のものを返します。両方が `0` なら結果も `0` です。

商の実部と虚部をそれぞれ最も近い整数に丸め、ユークリッドの互除法を行います。成分の絶対値の最大を $M$ として $O(\log(M+2))$ 時間、$O(1)$ 領域です。

## `GaussianInteger`

```text
struct GaussianInteger {
    std::int64_t real = 0;
    std::int64_t imag = 0;
};
```

`real` と `imag` が実部と虚部です。

## `operator==`

```text
friend constexpr bool operator==(const GaussianInteger &left,
                                  const GaussianInteger &right) noexcept;
```

実部と虚部の両方を $O(1)$ で比較します。

[使用例](../Examples/gaussian_integer_gcd_example.cpp) ／ [Verify](../Verify/current/submissions/gcd_of_gaussian_integers.cpp) ／ [数学的参考](https://kconrad.math.uconn.edu/blurbs/ugradnumthy/Zinotes.pdf)
