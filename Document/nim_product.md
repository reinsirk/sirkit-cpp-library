# ニム積

64ビット整数を、加法を XOR とする有限体 $\mathbb{F}_{2^{64}}$ の要素として掛け合わせます。

```cpp
auto a = sirkit::nim_product(2, 2);  // 3
auto b = sirkit::nim_product(2, 3);  // 1
```

## `nim_product(a, b)`

```text
std::uint64_t nim_product(std::uint64_t a, std::uint64_t b);
```

2つの `std::uint64_t` を受け取り、ニム積を `std::uint64_t` で返します。`0` との積は `0`、`1` との積は元の値です。

$k$ が2の冪のとき、$F=2^k$ と $x<F$ のニム積は通常の積 $Fx$ です。$F$ 同士のニム積は $F\mathbin{\mathtt{xor}}(F/2)$ です。この関係で数を上下半分に分けて計算します。最初の呼び出しで69 KiBの表を作り、以後は各積を定数時間で求めます。

[使用例](../Examples/nim_product_example.cpp) ／ [Verify](../Verify/current/submissions/nim_product_64.cpp) ／ [数学的参考](https://www.cambridge.org/core/books/abs/games-of-no-chance/impartial-games/FAC3E206D7C0A86C60615EF8F669EABB)
