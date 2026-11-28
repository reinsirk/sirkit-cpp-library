# 円周上の弦の交差照会

登録した弦のうち、照会した弦と内側で交差するものを数えます。円周に沿った端点の順序があれば使えます。

## `count_chord_crossings`

```cpp
std::vector<int> count_chord_crossings(
    std::span<const std::pair<std::int64_t, std::int64_t>> chords,
    std::span<const std::pair<std::int64_t, std::int64_t>> queries);
```

円周を一周する順に端点へ数を割り振ります。各弦はその数の組で、組の中の順序は任意です。4端点が円周上で交互に現れる場合に交差と数え、端点だけを共有する弦や同じ点を結ぶ弦は含めません。同じ登録弦が複数あれば、その本数分を数えます。

結果は照会した順に返ります。登録数を $M$、照会数を $Q$ として、$O(1+(M+Q)\log(M+Q+1))$ 時間・$O(M+Q+1)$ 領域です。登録数は `INT_MAX` 以下です。

## 使用例

```cpp
using Chord = std::pair<std::int64_t, std::int64_t>;
const std::array chords{Chord{2, 4}, Chord{6, 8}};
const std::array queries{Chord{3, 7}, Chord{4, 7}};
const auto counts = sirkit::count_chord_crossings(chords, queries);
const std::vector<int> expected{2, 1};
assert(counts == expected);
```

[実装](../Geometry/Chord_Crossing_Queries.cpp)・[使用例の全体](../Examples/chord_crossing_queries_example.cpp)・[Verify](../Verify/current/submissions/chord_crossing_queries.cpp)
