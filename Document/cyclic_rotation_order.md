# 巡回シフトの辞書順

文字列の先頭を末尾へ繰り返し移して得られる列を比べます。たとえば `baba` の開始位置0と2はともに `baba`、位置1と3は `abab` です。比較には符号なしバイト順を使います。

## `cyclic_rotation_ranks`

```cpp
std::vector<int> cyclic_rotation_ranks(std::string_view text);
std::vector<int> cyclic_rotation_ranks(std::span<const std::string_view> texts);
```

各回転の辞書順順位を0始まりで返します。同じ回転には同じ順位を割り当て、順位は詰めます。1文字列では `ranks[j]` が開始位置 `j` の順位です。長さ $n$ の文字列が $m$ 個なら、`ranks[i*n+j]` が `texts[i]` の開始位置 `j` に対応します。

1文字列では $O(n+1)$ 時間・領域、複数では $O(mn+m+1)$ 時間・$O(mn+1)$ 領域です。文字列ごとの長さが異なる場合は `std::invalid_argument` を送出し、連結長 `2*m*n` が `INT_MAX-2` を超える場合は `std::length_error` を送出します。

## `cyclic_rotation_order`

```cpp
std::vector<int> cyclic_rotation_order(std::string_view text);
```

開始位置を回転の辞書順に並べます。同じ回転では開始位置が小さい順です。$O(n+1)$ 時間・領域です。

## `count_cyclic_rotation_pairs`

```cpp
std::uint64_t count_cyclic_rotation_pairs(std::string_view first, std::string_view second);
```

同じ長さの2文字列について、`first` の回転が `second` の回転以下である開始位置対を数えます。$O(n+1)$ 時間・領域です。空文字列に開始位置はなく、順位列・並び順は空、対の個数は0です。

## 使用例

```cpp
const std::vector<int> order{1, 3, 0, 2};
assert(sirkit::cyclic_rotation_order("baba") == order);
assert(sirkit::count_cyclic_rotation_pairs("adb", "cab") == 4);
```

[実装](../String/Cyclic_Rotation_Order.cpp)・[使用例の全体](../Examples/cyclic_rotation_order_example.cpp)・[Verify](../Verify/current/submissions/cyclic_rotation_pairs.cpp)・[最小・最大の回転だけを求める場合](cyclic_rotation.md)
