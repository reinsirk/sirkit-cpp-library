# 疎行列の行列式

法 $p=998244353$ で $N$ 行 $N$ 列の疎行列の行列式を求めます。要素を `(行, 列, 値)` の一覧で渡します。

```cpp
const std::vector<sirkit::SparseDeterminantEntry> entries{{0, 0, 2}, {0, 1, 7}, {1, 1, 3}};
const auto det = sirkit::sparse_determinant_mod998244353(2, entries);  // 6
```

## `sparse_determinant_mod998244353(n, entries, seed=0, attempts=4)`

```text
std::optional<std::uint32_t> sparse_determinant_mod998244353(
    std::size_t n, std::span<const SparseDeterminantEntry> entries,
    std::uint64_t seed = 0, int attempts = 4);
```

行と列は `0` 始まりで、同じ位置の値は法 $p$ で足し合わせます。`0×0` の行列式は `1` です。通常版では、添字が範囲外なら `std::out_of_range`、試行回数が負なら `std::invalid_argument` です。容量の上限を超えると `std::length_error` を送出します。

戻り値は `std::optional<std::uint32_t>` です。行列式が確定すれば値を返し、試行を使い切っても確定しなければ `std::nullopt` を返します。`seed` を指定すると試行を再現でき、別の `seed` で再試行できます。

Wiedemann 法で独立一様乱数を仮定すると、1試行の未確定確率は高々 $\frac{N(N-1)}{2(p-1)}+\frac{2N}{p}$ です。独立に $t$ 回試行した場合、未確定確率はこの上限の $t$ 乗以下です。実装はシード付きの疑似乱数を使います。入力の要素数を $K$ として、1試行は $O(N(N+K))$ 時間、領域は $O(N+K)$ です。

## `SparseDeterminantEntry`

```text
struct SparseDeterminantEntry {
    std::size_t row;
    std::size_t column;
    std::uint32_t value;
};
```

`row` と `column` は行・列の添字です。`value` は法998244353で正規化します。

## `BerlekampMassey`

```text
template <class T>
std::vector<T> BerlekampMassey(const std::vector<T> &sequence);
std::vector<std::uint32_t> BerlekampMassey(const std::vector<std::uint32_t> &sequence);
```

同梱の関数です。有限列に合う最短線形漸化式の係数を返します。`T` は正確な体を指定し、`std::uint32_t` 版は法998244353で計算します。長さ $L$ に対し $O(L^2)$ 時間・$O(L)$ 領域です。[係数の順序と使用例](pages/berlekamp_massey.md)

[使用例](../Examples/sparse_matrix_determinant_example.cpp) ／ [Verify](../Verify/current/submissions/sparse_matrix_det.cpp) ／ [対角行列による前処理の論文](https://kaltofen.math.ncsu.edu/bibliography/02/CEKSTV02.pdf)
