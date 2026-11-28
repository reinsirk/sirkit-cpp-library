# DynamicWaveletMatrix

列の途中に値を挿入・削除しながら、区間内で小さい方から何番目か、ある値が何個あるかを調べます。更新後も列の並び順を保ちます。

## 使い方

```cpp
std::vector<std::uint32_t> a{8, 2, 5};
sirkit::DynamicWaveletMatrix<std::uint32_t> wm(a);
wm.insert(1, 3); // [8, 3, 2, 5]
assert(wm.kth_smallest(0, 3, 1) == 3);
assert(wm.count_less(0, 4, 5) == 2);
wm.set(0, 3); // [3, 3, 2, 5]
assert(wm.count(0, 4, 3) == 2);
[[maybe_unused]] const auto removed = wm.erase(2);
assert(removed == 2);
assert(wm.access(2) == 5);
```

[使用例](../Examples/dynamic_wavelet_matrix_example.cpp)

値は符号なし整数型です。値域が分かっているときは、`DynamicWaveletMatrix<std::uint32_t, 20>` のようにビット幅を指定できます。区間は半開区間 `[l,r)` で、位置と順位は0始まりです。

## 計算量

列の長さを `n`、ビット幅を `B` とすると、構築は `O(B(n+1))`、更新・検索は乱数優先度を使う平衡木で期待償却 `O(B log(n+2))` です。領域は、それまでの最大長を `N` として `O(B(1+N/64))` ワードです。更新のない列では [WaveletMatrix](wavelet_matrix.md) が照会を `O(B)` で処理します。
