# 座標圧縮

離れた値を順序が保たれる $0$ 始まりの添字に置き換えます。登録値は `std::vector<T>` または `std::span<const T>` で渡します。[本体](../Algorithm/Coordinate_Compression.cpp)

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `CoordinateCompression(values, compare={})` | 比較順に並べて等価な値をまとめる。既定は昇順 | $O(n\log(n+1))$ |
| `find(x)` | 等価な登録値の添字を返す。なければ `std::nullopt` | $O(\log(u+1))$ |
| `lower_bound(x)` | `x` の挿入位置を返す | $O(\log(u+1))$ |
| `upper_bound(x)` | `x` と等価な値があれば、その直後の位置を返す | $O(\log(u+1))$ |
| `value(i)`, `values()` | 添字 `i` の値への参照、または全登録値の読み取り専用ビュー | $O(1)$ |
| `size()`, `empty()` | 圧縮後の値の種類数、空判定 | $O(1)$ |

$n$ は入力の長さ、$u$ は異なる値の数です。`lower_bound` と `upper_bound` は該当する位置がなければ `size()`。比較には `const` で呼び出せる狭義弱順序を使い、同値が複数あればいずれか1つを残します。コピー間で登録値を共有し、ビューは登録値を所有するオブジェクトの寿命内で使います。

```cpp
const sirkit::CoordinateCompression coordinates(std::vector{100, -5, 100, 42});
assert(coordinates.find(100) == 2);
assert(!coordinates.find(50) && coordinates.lower_bound(50) == 2);
assert(coordinates.value(1) == 42);
```

[実行できる使用例](../Examples/coordinate_compression_example.cpp)・[AtCoder Verify](../Verify/current/submissions/coordinate_compression.cpp)
