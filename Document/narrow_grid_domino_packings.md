# 幅の狭い盤面のドミノ配置

`narrow_grid_domino_packings(h, w, mode, mod)` は、`h` 行 `w` 列の盤面に、重ならないようにドミノを置く方法を `mod` で数えます。縦向きと横向きに置けます。`h` は `0..5`、`w` は `std::uint64_t` です。

- `DominoPackingMode::kPerfect` は全マスを覆う配置を数えます。
- `DominoPackingMode::kMaximal` はドミノをこれ以上追加できない配置を数えます。隣接する未被覆マスがあれば、そこにドミノを追加できます。

```cpp
auto ways = sirkit::narrow_grid_domino_packings(3, 2, sirkit::DominoPackingMode::kMaximal); // 5
```

1列分の遷移を作り、行列の累乗で長い盤面も扱います。到達する状態数を $S$ とすると、時間 $O(S3^h+S^3\log(w+1))$、領域 $O(S^2+4^h)$ です。$S$ は完全配置で $2^h$、極大配置で $3^h$ 以下です。

[使用例](../Examples/narrow_grid_domino_packings_example.cpp) ／ [Verify](../Verify/current/submissions/narrow_grid_domino_packings.cpp)
