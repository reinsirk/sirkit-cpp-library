# Floor Sum の重み付き和と二乗和

$q_i=\lfloor(ai+b)/m\rfloor$ の和に加えて、添字で重み付けした和や二乗和も同時に求めます。

## `floor_sum_moments`

```text
struct FloorSumMoments {
    std::uint64_t sum;
    std::uint64_t index_weighted;
    std::uint64_t pairs;
};
FloorSumMoments floor_sum_moments(std::uint32_t n, std::uint32_t m,
                                 std::int64_t a, std::int64_t b)
```

$i=0,\ldots,n-1$ での $\sum q_i$、$\sum i q_i$、$\sum q_i(q_i-1)/2$ を返します。どの値も法 $2^{64}$ の剰余です。`a` と `b` には負数も渡せます。`m=0` は `std::invalid_argument` です。`n=0` なら全て 0 を返します。$O(\log(m+1))$ 時間・領域です。

## `sum_of_squares`

```text
std::uint64_t sum_of_squares() const noexcept
```

$\sum q_i^2$ を法 $2^{64}$ で返します。$O(1)$ 時間です。

```cpp
const auto result = sirkit::floor_sum_moments(4, 7, 2, 4); // q={0,0,1,1}
const auto squared = result.sum_of_squares(); // 2
```

[使用例](../Examples/floor_sum_moments_example.cpp) ／ [Verify](../Verify/current/submissions/floor_sum_moments_abc402_g.cpp) ／ [通常のFloor Sum](floor_sum.md)
