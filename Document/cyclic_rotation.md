# 文字列の最小・最大巡回シフト

文字列を任意の位置で切り、後半、前半の順に並べた文字列を巡回シフトと呼びます。その中で辞書順が最小または最大になる開始位置を求めます。

## `min_cyclic_rotation_index`

```text
std::size_t min_cyclic_rotation_index(std::string_view text) noexcept
```

辞書順で最小の巡回シフトの開始位置を返します。開始位置は0始まりです。結果が同じなら最も小さい開始位置を選び、空文字列では0を返します。比較には符号なしバイトの値を使います。

長さを $n$ として $O(n+1)$ 時間、追加領域は $O(1)$ です。

## `max_cyclic_rotation_index`

```text
std::size_t max_cyclic_rotation_index(std::string_view text) noexcept
```

辞書順で最大の巡回シフトの開始位置を返します。添字、同順位、空文字列の扱いと計算量は最小版と共通です。

## 使用例

```cpp
std::string text = "aaba";
const auto start = sirkit::min_cyclic_rotation_index(text); // 3
const auto rotated = text.substr(start) + text.substr(0, start); // "aaab"
```

[実装](../String/Cyclic_Rotation.cpp)・[使用例の全体](../Examples/cyclic_rotation_example.cpp)
