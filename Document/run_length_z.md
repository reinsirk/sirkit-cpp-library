# 連長圧縮された文字列のZ値

長い連続区間をまとめたまま、ある位置からの接尾辞と元の文字列の共通接頭辞長を求めます。

## `RunLengthZ`

```text
Run{byte, count}
explicit RunLengthZ(std::span<const Run> input)
```

`Run` は `RunLengthZ::Run` の略です。`byte` が `count` 回続く区間を指定します。構築時に0回の区間を飛ばし、同じバイトの隣り合う区間を結合します。展開後の長さは `UINT64_MAX` 以下、結合後の区間数は `INT_MAX` 以下です。

入力区間数を $n$ として、構築は $O(n+1)$ 時間・領域です。

## `size`

```text
std::uint64_t size() const noexcept
```

展開後の長さを $O(1)$ 時間で返します。

## `prefix_at`

```text
std::uint64_t prefix_at(std::uint64_t position) const
```

展開後の0始まりの位置から始まる接尾辞について、元の文字列と先頭から一致する長さを返します。`position <= size()` が前提で、末尾位置なら0です。$O(\log(n+2))$ 時間です。

## `sum_prefix_lengths`

```text
std::uint32_t sum_prefix_lengths(std::uint32_t modulus = 1'000'000'007) const
```

展開後の各開始位置の `prefix_at` の総和を、正の法 `modulus` で割った余りとして返します。$O(n+1)$ 時間・$O(1)$ 追加領域です。

## 使用例

```cpp
using Run = sirkit::RunLengthZ::Run;
const sirkit::RunLengthZ index(std::array{Run{'a', 2}, Run{'b', 1}, Run{'a', 2}}); // aabaa
assert(index.prefix_at(3) == 2);
assert(index.sum_prefix_lengths() == 9);
```

[実装](../String/Run_Length_Z.cpp)・[使用例の全体](../Examples/run_length_z_example.cpp)・[同梱Z Algorithm](z_algorithm.md)
