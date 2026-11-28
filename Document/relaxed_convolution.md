# オンライン畳み込み

数列 $A,B$ の係数を先頭から1組ずつ追加し、そのたびに畳み込みの次の係数を得ます。直前の出力を次の入力の計算に使う場合にも使えます。

## 構築

```text
template <std::uint32_t Mod = 998244353, std::uint32_t Root = 3>
class RelaxedConvolution;
```

空の列から始めます。`Mod` と原始根 `Root` の条件は NTT と同じです。構築は $O(1)$ 時間です。

## `push`

```text
std::uint32_t push(std::uint32_t a, std::uint32_t b)
```

次の添字 $i$ に2つの値を追加し、$\sum_{j=0}^i A_j B_{i-j}$ を `Mod` で割った余りを返します。入力を $n$ 組追加する合計は $O(n\log^2(n+1))$ 時間、領域は $O(n)$ です。

## `size`

```text
std::size_t size() const noexcept
```

現在の組数を $O(1)$ 時間で返します。

## `max_size`

```text
static constexpr std::size_t max_size() noexcept
```

追加できる組数の上限を $O(1)$ 時間で返します。これを超える `push` は `std::length_error` です。

## `clear`

```text
void clear() noexcept
```

空の列に戻します。確保した領域は次回の入力で再利用できます。

```cpp
sirkit::RelaxedConvolution c;
auto first = c.push(3, 5);  // 15
auto second = c.push(4, 6); // 3*6 + 4*5 = 38
```

[使用例](../Examples/relaxed_convolution_example.cpp) ／ [Verify](../Verify/current/submissions/relaxed_convolution_abc315_ex.cpp) ／ [同梱NTT](ntt.md)
