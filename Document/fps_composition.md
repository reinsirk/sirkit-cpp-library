# 形式的冪級数の合成・逆合成・累乗の係数

法 $998244353$ で級数を合成できます。$f(g(x))=x$ となる $g$ を求める操作もあります。

## `fps_compose`

```text
FormalPowerSeries fps_compose(const FormalPowerSeries &outer,
                              const FormalPowerSeries &inner, std::size_t precision)
```

`outer(inner(x))` のうち、次数 `precision` 未満の係数を返します。`inner` の定数項は 0 とします。

## `fps_compositional_inverse`

```text
FormalPowerSeries fps_compositional_inverse(const FormalPowerSeries &value, std::size_t precision)
```

`value(g(x))` が次数 `precision` 未満で $x$ と一致する $g$ を返します。`value` の定数項は 0、一次係数は非零とします。

両方とも精度 $N$ は $2^{18}$ 以下です。$N=0$ では空を返します。時間は $O(N\log^2(N+1))$、領域は $O(N\log(N+1))$ です。

## `fps_power_projection`

```text
FormalPowerSeries fps_power_projection(const FormalPowerSeries &value,
                                       const FormalPowerSeries &weight,
                                       std::size_t degree, std::size_t count)
```

$k=0,\ldots,\texttt{count}-1$ の順に、$\mathrm{weight}(x)\cdot\mathrm{value}(x)^k$ の $x^{\mathrm{degree}}$ の係数を返します。`value` の定数項は任意です。`degree` は $2^{18}$ 未満、`count` は $2^{18}$ 以下です。$D=\texttt{degree}+1$、$C=\texttt{count}$ として $O(D\log^2(D+1)+C\log(C+1))$ 時間、$O(D+C)$ 領域です。

```cpp
const sirkit::FormalPowerSeries f{0, 1, -1};
const auto g = sirkit::fps_compositional_inverse(f, 5); // {0, 1, 1, 2, 5}
const auto x = sirkit::fps_compose(f, g, 5); // {0, 1, 0, 0, 0}
```

[使用例](../Examples/fps_composition_example.cpp) ／ [合成のVerify](../Verify/current/submissions/composition_of_formal_power_series_large.cpp) ／ [逆合成のVerify](../Verify/current/submissions/compositional_inverse_of_formal_power_series_large.cpp) ／ [累乗の係数のVerify](../Verify/current/submissions/fps_power_projection_abc439_g.cpp) ／ [同梱FPS](formal_power_series.md) ／ [Kinoshita–Liの論文](https://arxiv.org/abs/2404.05177)
