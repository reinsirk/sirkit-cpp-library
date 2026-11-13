# 最近共通祖先

[LCA](../Graph/LCA.cpp)は無向森を前処理し、最近共通祖先・距離・祖先・パス上の頂点を返します。[使用例](../Examples/lca_example.cpp)を参照してください。距離は辺数です。

## 構築

隣接リスト`std::vector<std::vector<int>>`、または頂点数と無向辺列を渡します。辺列の型は`std::span<const std::pair<int,int>>`です。読み取り専用の配列参照なので、`std::vector<std::pair<int,int>>`をそのまま渡せます。各無向辺を1回だけ記録し、向きは問いません。どちらの入力も構築後には参照を保持しません。

頂点番号は`[0,N)`で、自己ループ・多重辺・閉路を含めません。隣接リストの対称性は利用側の前提です。指定した`root`の成分から処理し、残りは最小未訪問頂点を各成分の根にします。空の入力には`root=0`を指定します。

Nを頂点数、Eを隣接リストの要素数（辺列版では無向辺数の2倍）、B=ceil(N/32)、Hを最大深さとします。構築はO(N+E+B log(B+1))、保持領域はO(N+B log(B+1))、追加作業領域はO(N)です。辺列版はこれに加えてO(N+E)の一時領域で隣接データを作り、構築完了時に解放します。

## API

| API | 動作・戻り値 | 計算量 |
| --- | --- | --- |
| `LCA()` | 空の表を作る。 | O(1) |
| `LCA(g,root=0)` / `build(g,root=0)` | 隣接リストから構築 / 再構築する。 | 構築と同じ |
| `LCA(n,edges,root=0)` / `build(n,edges,root=0)` | 無向辺列から構築 / 再構築する。 | 構築と同じ |
| `size()/empty()/component_count()/root()` | 頂点数・空判定・成分数・指定根。空の根は-1。 | O(1) |
| `parent(v)/depth_of(v)/component_of(v)/root_of(v)` | 親・深さ・成分番号・成分根。根の親は-1。 | O(1) |
| `same_component(u,v)/is_ancestor(u,v)` | 同成分か / uがvの祖先か。自身も含む。 | O(1) |
| `get(u,v)/operator()(u,v)` | 最近共通祖先。異成分なら-1。 | O(1) |
| `get(u,v,new_root)` | new_rootを根としたLCA。3頂点の成分が揃わなければ-1。 | O(1) |
| `get_distance(u,v)` | パスの辺数。異成分なら-1。 | O(1) |
| `kth_ancestor(v,k)` | k個上の祖先。k=0ならv、根を越えると-1。 | O(log(H+2)) |
| `kth_on_path(u,v,k)` | uからvへのパスのk番目（0始まり）。異成分・範囲外なら-1。 | O(log(H+2)) |
| `order_index(v)` | 森全体でのDFS順添字。 | O(1) |
| `subtree_range(v)/subtree_size(v)` | DFS順の部分木半開区間 / 部分木の頂点数。 | O(1) |
| `clear()/swap(other)` | 領域を解放して空にする / 内容を交換する。 | O(N+B log(B+1)) / O(1) |

```cpp
std::vector<std::pair<int,int>> edges{{0,1},{0,2},{1,3},{1,4},{5,6}};
LCA tree(7, edges);                 // 0〜4と5〜6の二成分
int a = tree.get(3, 4);             // 1
int distance = tree.get_distance(3, 2); // 3
int middle = tree.kth_on_path(3, 2, 2); // 0
int disconnected = tree.get(0, 5); // -1
auto [first,last] = tree.subtree_range(1);
tree.build(7, edges, 4);           // 根を変更して再構築
```

## 前提と所有権

照会の頂点番号は表の範囲内で指定します。kは`std::uint64_t`で受け、`UINT64_MAX`も範囲外と判定します。根変更の`get`は答えだけを計算し、部分木の向きは変えません。部分木は構築時の根に対し、`first <= order_index(x) < last`を満たす頂点全体です。兄弟の訪問順は規定しません。

検査有効時は不正な根・頂点・辺列の端点を`out_of_range`、閉路・自己ループ・多重辺を`invalid_argument`で拒否します。辺列版は構築時に全端点を検査します。隣接リストの対称性は検査しません。`SKIP_RUNTIME_CHECKS`で入力検査を除いても、負のnと内部容量超過の`length_error`は維持します。

`build`とコピー代入は失敗時に元の内容を保持します。コピー後の再構築・破棄は互いに影響しません。頂点情報と親表は各オブジェクトが所有し、深い森の不変な照会表は内部共有します。ムーブ元は空で再利用でき、自己コピー・自己ムーブは内容を保持します。

深い森は32要素ブロックのRMQでLCAを求め、Ladder分解で祖先を探索します。浅い森は小さな祖先表を使います。Ladderの数学的参考は[Bender・Farach-Colton](https://www3.cs.stonybrook.edu/~bender/newpub/2004-level.pdf)です。実装の出典ではありません。

提出先は[Library Checker](https://judge.yosupo.jp/problem/lca)です。森・任意の根・追加API・所有権・例外保証は[独立テスト](../Verify/current/tests/lca_reference_test.cpp)で補い、詳しい検証・測定は[Audit](../Audit/)に置きます。
