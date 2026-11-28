# ハミング距離でのパターン照合

テキスト内の各位置で、パターンと異なるバイトの数を求めます。テキストとパターンには任意のバイトを使えます。

## `hamming_distances`

```text
std::vector<std::size_t> hamming_distances(std::string_view text, std::string_view pattern)
```

0始まりの開始位置 `i` ごとに、そこからパターンと同じ長さを切り出したときの不一致数を返します。パターンが空なら末尾を含む全境界の値が0、テキストより長ければ結果は空です。

テキスト長を $n$、パターン長を $m$ とします。$1\le m\le n$ の場合は $n+m-1\le 2^{23}$ が前提です。$L$ を $n+m-1$ 以上で最小の2のべき乗、各バイトの出現数をそれぞれ $a_c,b_c$ とすると $O(n+m+\sum_c\min(a_cb_c,L\log L))$ 時間・$O(n+m+1)$ 領域です。頻度が低いバイトは位置の組を、高いバイトはNTTの積を使って一致数を数えます。

## `approximate_match_positions`

```text
std::vector<std::size_t> approximate_match_positions(std::string_view text, std::string_view pattern,
                                                      std::size_t max_mismatches)
```

不一致数が `max_mismatches` 以下の開始位置を昇順で返します。条件と計算量は `hamming_distances` と共通です。

## 使用例

```cpp
const auto distances = sirkit::hamming_distances("aaba", "ab");              // {1, 0, 2}
const auto starts = sirkit::approximate_match_positions("aaba", "ab", 1);   // {0, 1}
```

[実装](../String/Hamming_Distance_Matching.cpp)・[使用例の全体](../Examples/hamming_distance_matching_example.cpp)・[同梱NTT](ntt.md)
