# 永続アフィン数列

区間の和を求めながら、区間の加算・アフィン変換・反転や、区間のコピーを行えます。更新は新しい数列を返すので、保存しておいた数列から区間を取り出せます。[本体](../DataStructure/Persistent_Affine_Sequence.cpp)

| 操作 | 内容 |
| --- | --- |
| `sum(l, r)` / `total()` | 半開区間の和、列全体の和 |
| `affine(l, r, a, b)` / `reverse(l, r)` | 各要素を `a*x+b` に変更、または区間を反転 |
| `slice(l, r)` / `split(k)` | 区間を取り出す、または位置 `k` で前後に分ける |
| `copy_from(l, r, other)` | 別の列の同じ添字の区間をコピー |
| `replace(l, r, other)` / `concat(other)` | 別の列で置換、または連結。置換で長さを変更してもよい |

```cpp
const std::array<long long, 5> values{1, 2, 3, 4, 5};
const sirkit::PersistentAffineSequence<long long> before(values);
const auto after = before.affine(1, 3, 1, 10);
const auto copied = after.replace(2, 4, before.slice(0, 2));
assert(copied.total() == 21);
assert(before.total() == 15);
```

構築は時間・領域とも $O(n)$、コピーと全体和は $O(1)$ です。区間和は $O(\log(n+1))$ でノードを追加しません。更新・分割・連結は $O(\log(n+m+1))$ 時間、同数の新規ノードを使います。$m$ は連結・置換する列の長さです。整数型では途中の積と和も型の範囲に収めてください。

[実行できる使用例](../Examples/persistent_affine_sequence_example.cpp)・[Library Checker Verify](../Verify/current/submissions/persistent_affine_sequence.cpp)・[AtCoder Verify](../Verify/current/submissions/persistent_affine_sequence_range_copy.cpp)
