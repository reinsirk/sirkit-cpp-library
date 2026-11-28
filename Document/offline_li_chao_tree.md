# 削除できるオフライン Li Chao 木

直線や区間付きの直線を追加・削除し、各時点の最小値をまとめて求めます。[本体](../DataStructure/Offline_Li_Chao_Tree.cpp)

## `OfflineLiChaoTree<X, Function = OfflineLiChaoLine<X>>`

既定の関数型は傾きと切片を `{a, b}` で受け取ります。座標 `X` は全順序を持ち、コピー可能な型です。自作の `Function` は `const` で呼び出せて、返り値はムーブ構築と `<` による比較ができる型にします。関数がコピーできない場合は、関数と座標のムーブ構築が `noexcept` である必要があります。

任意の2関数が両方とも有効な範囲で、比較結果が座標順に変わる回数は高々1回です。整数では登録した座標での途中計算も型の範囲に収めてください。

| 操作 | 内容 | 時間 |
| --- | --- | --- |
| `add_line(f)` | 全域の関数を追加し、削除用IDを返す | 償却 $O(1)$ |
| `add_segment(l, r, f)` | $l\leq x<r$ の範囲で有効な関数を追加し、IDを返す | 償却 $O(1)$ |
| `erase(id)` | 発行元で有効な関数を削除する | $O(1)$ |
| `query(x)` | その時点の照会を登録し、回答の添字を返す | 償却 $O(1)$ |
| `solve()` | 照会順に最小値を返す。候補が空なら `std::nullopt` | 下記 |

照会数 $Q$、異なる照会座標数 $X$、全域・区間追加数 $A,S$ とすると、`solve` は $O(Q\log(Q+1)+(A+S)\log(X+1)+\log(Q+1)(A\log(X+1)+S\log^2(X+1)))$ 時間です。`solve` の後も操作を追記して再実行できます。

```cpp
sirkit::OfflineLiChaoTree<int> lines;
const auto id = lines.add_line({2, 1});
lines.add_segment(0, 2, {0, 4});
lines.query(1);
lines.erase(id);
lines.query(1);
const auto answer = lines.solve();  // 3, 4
```

[実行できる使用例](../Examples/offline_li_chao_tree_example.cpp)・[AtCoder Verify](../Verify/current/submissions/offline_li_chao_tree_coins.cpp)
