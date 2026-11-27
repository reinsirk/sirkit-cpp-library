# 素数を法とする原始根と乗法位数

素数を法とする最小の原始根と、各整数の乗法位数を求めます。

```cpp
const sirkit::PrimeMultiplicativeGroup group(17);
auto g = group.primitive_root();  // 3
auto order = group.order(4);     // 4
```

## `PrimeMultiplicativeGroup`

```text
explicit PrimeMultiplicativeGroup(std::uint64_t prime);
```

素数 $p$ を法とする乗法群を作り、$p-1$ の因数分解を後続の操作で再利用します。素数以外は `std::domain_error`。素因数 $r$ の分離に期待 $O(\sqrt r)$ 回の剰余乗算を使うPollard–Rho法です。

## `modulus`

```text
std::uint64_t modulus() const noexcept;
```

法 $p$ を $O(1)$ で返します。

## `order`

```text
std::optional<std::uint64_t> order(std::uint64_t a) const noexcept;
```

$a^k \equiv 1 \pmod p$ となる最小の正整数 $k$ を返します。$a$ が $p$ の倍数なら `std::nullopt`。$O(\log^2 p)$ 回の剰余乗算です。

## `is_generator`

```text
bool is_generator(std::uint64_t a) const noexcept;
```

$a$ の位数が $p-1$ なら真を返します。$p-1$ の異なる素因数の数を $d$ とすると $O(d\log p)$ 回の剰余乗算です。

## `PrimeMultiplicativeGroup::primitive_root`

```text
std::uint64_t primitive_root() const noexcept;
```

法 $p$ の最小の原始根を返します。$p-1$ の異なる素因数の数を $d$、試す候補の数を $g$ とすると $O(gd\log p)$ 回の剰余乗算です。

## `primitive_root`

```text
std::uint64_t primitive_root(std::uint64_t prime);
```

乗法群を構築して、法 `prime` の最小の原始根を返します。因数分解と原始根の探索にかかる時間は上記の各操作と同じです。

## 同梱の64ビット因数分解

`PollardRho64` と、因数分解・約数・トーティエント関数などの自由関数も使えます。[PollardRho64と自由関数のAPI](pollard_rho.md#api)

[使用例](../Examples/primitive_root_example.cpp) ／ [Verify](../Verify/current/submissions/primitive_root.cpp)
