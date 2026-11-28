# 乗法的関数の累積和

`MultiplicativePrefixSums sieve(N)` は、$f(1)=1$ の乗法的関数について、$N$ 以下の累積和を計算します。$N$ は1以上の `uint64_t`。構築は $O(\sqrt N\log\log(N+2))$ 時間で、領域は $O(\sqrt N)$ です。

- `size()` は異なる商 $\lfloor N/i\rfloor$ の個数、`quotient(j)` は昇順の0始まりの第 $j$ 項を返します。どちらも $O(1)$ 時間。
- `prime_prefix_sums(weight, prefix)` は各商以下の素数に限った重みの和を返します。`weight` は $\operatorname{weight}(1)=1$ の完全乗法的な関数、`prefix(x)` は $1$ から $x$ までの重みの総和です。
- `prefix_sums(prime_sums, prime_power)` は各商までの $f$ の総和を返します。`prime_sums` は各商までの素数での $f$ の総和、`prime_power(p,e)` は $f(p^e)$ です。
- `prefix_sum(prime_sums, prime_power)` は $N$ までの総和を1個返します。`prime_sums` の内容は保たれます。

値型は加減乗算ができる可換環です。`T{}` を0、`T{1}` を1として扱い、符号なし整数の演算は型幅の法で折り返します。各演算とコールバックが $O(1)$ なら、`prime_prefix_sums` と `prefix_sums` は $O(N^{3/4}/\log(N+2))$ 時間。`prefix_sum` は調べる素因数の組の数に比例し、最悪 $O(N)$ 時間です。

たとえば $f$ を正の約数の個数とすると、素数での値は2、素数冪 $p^e$ での値は $e+1$。$N=10$ で `prefix_sum` が返す総和は27です。

[使用例](../Examples/multiplicative_prefix_sums_example.cpp) ／ [Verify](../Verify/current/submissions/sum_of_multiplicative_function.cpp) ／ [Verify Large](../Verify/current/submissions/sum_of_multiplicative_function_large.cpp)
