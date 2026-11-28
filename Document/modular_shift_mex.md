# 等差にずらした集合の mex の総和

`ModularShiftMex(count, modulus, step, start=0)` の初期状態は空集合です。各 $k$ で集合を `start + step * k` ずらし、法 `modulus` で余りを取ります。`sum()` は、$0 \le k < \text{count}$ に対する mex（含まれない最小の非負整数）の総和を `uint64_t` で返します。

- `insert(x)` と `erase(x)` は、集合が変われば `true` を返します。同じ値を複数回入れても1個です。$0\le x<\text{modulus}$ が必要です。
- `count` と `modulus` は `uint32_t`、`step` と `start` は負の `int64_t` も指定できます。法0には `std::invalid_argument`、範囲外の値には `std::out_of_range` を投げます。
- 更新は $O(\log(r+2)+\log(m+1))$ 時間、`sum()` と `size()` は $O(1)$ 時間です。$r$ は直線上の連続区間数、$m$ は法。領域は $O(r+1)$ で、`clear()` は $O(r+1)$ 時間です。

法3で集合が `{0,2}`、初期値0、公差1、回数2なら、mex は順に1と2で総和は3です。

[使用例](../Examples/modular_shift_mex_example.cpp) ／ [Verify](../Verify/current/submissions/modular_shift_mex_abc465_g.cpp)
