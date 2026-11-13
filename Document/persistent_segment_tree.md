# 永続セグメント木

`PersistentSegmentTree<T,Op>`は、過去の列を残したまま点を更新します。各版をRootで指定し、古い版から分岐した更新もできます。区間は0始まりの半開区間[l,r)で、集約は左から右の順序を保ちます。[使用例](../Examples/persistent_segment_tree_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `(n,op,identity)` / `(values,op,identity)` | 全要素単位元の列 / vectorやspanの列で構築する。O(1) / O(N)。 |
| `empty_root()` / `latest_root()` | 全要素単位元の版 / 直近に成功したbuild・set・transformの版を返す。 |
| `build(values)` | 同じ長さの列のRootを追加する。過去の版は残る。O(N)。 |
| `set(root,i,x)` / `transform(root,i,fn)` | 指定した版の点をx / fn(現在値)へ置き換えた新しいRootを返す。 |
| `get(root,i)` / `prod(root,l,r)` | 指定した版の点 / 区間の値を返す。空区間はidentity。 |
| `all_prod(root)` | 指定した版の全体積をO(1)で返す。 |
| `max_right(root,l,pred)` / `min_left(root,r,pred)` | 条件を満たす最大右端 / 最小左端を返す。 |
| `size()` / `empty()` / `allocated_nodes()` | 列長 / 空か / 空の根を除く保持ノード数。O(1)。 |
| `reserve(nodes)` | 総ノード容量を予約する。既存Rootは有効。再確保時O(保持ノード数)。 |
| `reset(n)` / `clear()` | 全Rootを無効にし、単位元の列 / 長さ0へ戻す。 |
| `remap_from(original,root)` | 直接のコピー元のRootを、コピー先で使えるRootへO(1)で変換する。 |

```cpp
PersistentSegmentTree<std::int64_t,std::plus<std::int64_t>> tree(
    std::vector<std::int64_t>{2,4,1},{},0);
auto first = tree.latest_root();
auto second = tree.set(first,1,9);
auto third = tree.transform(first,2,[](auto x){return x+5;});
// all_prod(first)==7、all_prod(second)==12、prod(third,1,3)==10。
```

## Rootと所有権

Rootは小さな不変の値で、作成した木へ渡します。木の破棄・reset・clear後には使えません。コピー先で元のRootを直接使うこともできません。`copy.remap_from(original,root)`で変換できるのは、直接のコピー元にコピー時点で存在したRootだけです。コピーしたノード列は独立し、以後の更新は互いに影響しません。

moveでは既存Rootが移動先へ所属し、move元には破棄・代入だけを行います。swapはRootの所属も交換します。更新・構築で確保や演算・コピーが失敗した場合、追加ノードを取り消し、古い版とlatest_rootを保持します。コピー代入には強い例外保証があります。演算の外部状態は巻き戻さず、コールバックから同じ木を再入して変更しないでください。

## 前提と計算量

Opはconst呼出可能な結合演算、identityは左右の単位元です。T・Opはコピー可能、Tの破棄は例外を投げない型を使います。演算・コピーをO(1)として、点更新はO(log(N+1))時間・追加領域、点取得・区間積・境界探索はO(log(N+1))です。密な構築はN>0で2N-1ノード、点更新は高々1+ceil(log2(N))ノードを追加します。

境界探索の述語はpred(identity)がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。長さ0では空のRootだけを作れ、空区間積・全体積はidentityです。reset・clearには旧ノード数に比例する破棄費用がかかります。

根の所属・添字・区間・長さは既定で検査します。`SKIP_RUNTIME_CHECKS`は入力検査と所属確認用の状態を除きますが、Rootの所属と有効期間は利用側で守る必要があります。ノード数の内部上限は常に維持します。

数学的参考は[AAL](https://info.atcoder.jp/entry/algorithm_lectures/persistent_segment_tree)、機能上の参考は[Nyaan](https://nyaannyaan.github.io/library/segment-tree/persistent-segment-tree.hpp.html)です。実装は整数添字で所有するノード列として組み直しています。
