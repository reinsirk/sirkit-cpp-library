# 二元拡大体の畳み込み

$\mathbb{F}_{2^{64}}$ の元を `std::uint64_t` で表し、畳み込みを計算します。第 $i$ ビットが $x^i$ の係数、和は XOR です。積は多項式として掛け、$x^{64}+x^4+x^3+x+1$ で割った余りを取ります。

| API | 戻り値・計算量 |
|---|---|
| `multiply_f2_64(a, b)` | 体の積。$O(1)$。 |
| `convolution_f2_64(a, b)` | 体を係数とする多項式の積。結果長以上の最小の2冪を $L$ として $O(L\log^2 L)$ 時間、$O(L)$ 領域。短い方が16要素以下なら $O(\lvert a\rvert\lvert b\rvert)$ 時間。 |

片方が空なら空列を返します。結果が $2^{21}$ 要素を超える場合は `std::length_error` です。GCC・Clang では対応する AMD64 のキャリーレス乗算命令を使います。`SIRKIT_DISABLE_SIMD` で標準C++経路を選べます。

```cpp
const std::array<std::uint64_t, 2> a{1, 2}, b{1, 3};
const auto c = sirkit::convolution_f2_64(a, b); // {1, 1, 6}
auto q = sirkit::multiply_f2_64(std::uint64_t{1} << 63, 2); // 27
```

[使用例](../Examples/convolution_f2_64_example.cpp) ／ [Verify](../Verify/current/submissions/convolution_f2_64.cpp) ／ [参考論文](https://arxiv.org/abs/1802.03932)
