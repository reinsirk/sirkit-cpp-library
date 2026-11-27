# 素数法での k 乗根

素数 $p$ に対し、$x^k\equiv y\pmod p$ を満たす $x$ を1つ求めます。

```cpp
const auto found = sirkit::modular_kth_root(3, 8, 13);   // 例 2
const auto missing = sirkit::modular_kth_root(2, 2, 13); // nullopt
```

## `modular_kth_root(k, y, p)`

```text
std::optional<std::uint32_t> modular_kth_root(std::uint64_t k, std::uint64_t y, std::uint32_t p);
```

根は `0 <= x < p` の範囲で返ります。根がない場合は `std::nullopt`、複数ある場合はそのうち1つです。`y` は法で縮約し、$0^0=1$ とします。$p=2$ を含む素数を渡してください。

## 計算量

$d=\gcd(k,p-1)$ を試し割りし、$d$ の各素数冪に対して根を抽出します。離散対数が要るのは $\ell^2\mid p-1$ を満たす素数 $\ell$ の位数部分群です。1桁につき $O(\log p + \sqrt\ell\log(\ell+1))$ 時間で、最大 $v_\ell(p-1)-v_\ell(d)$ 桁を求めます。表に使う枠は $O(\sqrt\ell)$ 個で、32ビット法では256枠以内です。部分群の生成元は小さい数から試し、$R$ 個試したときの時間は $O(R\log p)$ です。試し割り用の素数表は初回に作り、以降も使います。

平方根専用の [ModSqrt](modular_arithmetic.md#modsqrt) は64ビット素数にも対応します。[離散対数](discrete_logarithm.md) は底を固定して指数を求める関数です。

[使用例](../Examples/modular_kth_root_example.cpp) ／ [Verify](../Verify/current/submissions/kth_root_mod.cpp) ／ [Library Checker](https://judge.yosupo.jp/problem/kth_root_mod) ／ [根抽出の原論文](https://www.cs.cmu.edu/~glmiller/Publications/AMM77.pdf) ／ [素数冪での離散対数の原論文](https://www-ee.stanford.edu/~hellman/publications/28.pdf)
