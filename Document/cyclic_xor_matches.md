# 共通XORを含む巡回一致

整数列を巡回させ、さらに全要素に同じ値でXORを取って、別の列と一致する回転を列挙します。

## `cyclic_xor_matches`

```cpp
std::vector<CyclicXorMatch> cyclic_xor_matches(
    std::span<const std::uint64_t> source, std::span<const std::uint64_t> target);
```

列の長さを $n$ として、すべての添字 `i` で `(source[(i + shift) % n] ^ mask) == target[i]` を満たす組を返します。結果は `shift` の昇順です。隣接要素どうしのXORで共通の `mask` を相殺し、その列の一致を検索します。$O(n+1)$ 時間・領域です。

2列は同じ長さで、$0\le n\le \mathrm{INT\_MAX}/2$ とします。空なら空の結果です。検査が有効なら長さの不一致に `std::invalid_argument`、長さ上限の超過には `std::length_error` を送出します。

## `CyclicXorMatch`

`shift` は `source` での0始まりの回転開始位置、`mask` は全要素に適用するXOR値です。

## 使用例

```cpp
const std::array<std::uint64_t, 3> source{0, 2, 1}, target{1, 2, 3};
const auto matches = sirkit::cyclic_xor_matches(source, target);
assert(matches.size() == 1 && matches[0].shift == 1 && matches[0].mask == 3);
```

[実装](../String/Cyclic_Xor_Matches.cpp)・[使用例の全体](../Examples/cyclic_xor_matches_example.cpp)・[Verify](../Verify/current/submissions/cyclic_xor_matches.cpp)
