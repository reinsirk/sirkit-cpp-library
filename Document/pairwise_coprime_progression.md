# どの2項も互いに素な等差数列

- `max_pairwise_coprime_progression_length(step)` は、正の公差 `step` で作れる最大の長さを返します。
- `pairwise_coprime_progression_start(length, step)` は、その長さを実現する正の初項を `BigInteger<10>` で返します。実現できなければ `nullopt` です。

たとえば公差6の最大長は9です。長さ3なら初項7が返り、数列は `(7,13,19)` になります。

公差は正の `uint64_t` です。公差を割らない最小の素数を $p$ とすると、最大長は $2p-1$ になります。64ビットでは $p\le53$ なので、両方とも時間・領域は $O(1)$ です。

[使用例](../Examples/pairwise_coprime_progression_example.cpp) ／ [Verify](../Verify/current/submissions/pairwise_coprime_progression_yuki3832.cpp)
