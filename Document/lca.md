# 最近共通祖先

[LCA](../Graph/LCA.cpp)は無向森を前処理し、最近共通祖先・距離・祖先・パス上の頂点を返します。[使用例](../Examples/lca_example.cpp)を参照してください。距離は辺数です。

## 構築

隣接リスト`std::vector<std::vector<int>>`、または頂点数と無向辺列を渡します。辺列の型は`std::span<const std::pair<int,int>>`です。読み取り専用の配列参照なので、`std::vector<std::pair<int,int>>`をそのまま渡せます。各無向辺を1回だけ記録し、向きは問いません。どちらの入力も構築後には参照を保持しません。

頂点番号は`[0,N)`で、自己ループ・多重辺・閉路を含めません。隣接リストの対称性は利用側の前提です。指定した`root`の成分から処理し、残りは最小未訪問頂点を各成分の根にします。空の入力には`root=0`を指定します。

Nを頂点数、Eを隣接リストの要素数（辺列版では無向辺数の2倍）、B=ceil(N/32)、Hを最大深さとします。構築はO(N+E+B log(B+1))、保持領域はO(N+B log(B+1))、追加作業領域はO(N)です。辺列版はこれに加えてO(N+E)の一時領域で隣接データを作り、構築完了時に解放します。

## API

### LCA

```text
LCA() noexcept;
explicit LCA(const std::vector<std::vector<int>>& graph, int root = 0);
LCA(int n, std::span<const std::pair<int,int>> edges, int root = 0);
LCA(const LCA& other);
LCA(LCA&& other) noexcept;
```

空の表、隣接リスト、または無向辺列から構築します。コピー後の再構築は互いに影響しません。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 空の構築・ムーブはO(1)、コピーはO(N)。入力からはO(N+E+B log(B+1))時間・O(N+B log(B+1))保持領域。

### build

```text
void build(const std::vector<std::vector<int>>& graph, int root = 0);
void build(int n, std::span<const std::pair<int,int>> edges, int root = 0);
```

隣接リストまたは無向辺列から表を構築します。構築済みの場合は表を作り直します。

**制約:** 入力グラフ・関数・比較器の前提は下記の共通条件に従います。

**計算量:** 空の構築はO(1)。その他はO(N+E+B log(B+1))時間、O(N+B log(B+1))保持領域です。

### size

```text
std::size_t size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

頂点数が0ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### component_count

```text
std::size_t component_count() const noexcept;
```

分解した連結成分の数を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### root

```text
int root() const noexcept;
```

根の頂点番号を返します。空なら-1です。

**制約:** なし。

**計算量:** O(1)。

### parent

```text
int parent(int v) const;
```

親の頂点番号を返します。根の親は-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### depth_of

```text
int depth_of(int v) const;
```

構築時の根からの深さを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### component_of

```text
int component_of(int v) const;
```

所属する木の番号を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### root_of

```text
int root_of(int v) const;
```

所属する木の根の頂点番号を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### same_component

```text
bool same_component(int u, int v) const;
```

2頂点が同じ木に属すならtrueを返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### is_ancestor

```text
bool is_ancestor(int u, int v) const;
```

uがvの祖先ならtrueを返します。uとvが同じ頂点ならtrue、異なる連結成分に属すならfalseです。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### get

```text
[[gnu::always_inline]] #endif int get(int u, int v) const;
[[gnu::always_inline]] #endif int get(int u, int v, int new_root) const;
```

最近共通祖先を返します。3引数版はnew_rootを根とした答えです。成分が揃わなければ-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### operator()

```text
int operator()(int u, int v) const;
```

2頂点の最近共通祖先を返します。異成分なら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### get_distance

```text
int get_distance(int u, int v) const;
```

パスの辺数を返します。2頂点が異なる連結成分に属すなら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### kth_ancestor

```text
int kth_ancestor(int v, std::uint64_t k) const;
```

k個上の祖先を返します。k=0ならv、根を越えるなら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log(H+2))。

### kth_on_path

```text
int kth_on_path(int u, int v, std::uint64_t k) const;
```

uからvへのパス上で0始まりのk番目の頂点を返します。2頂点が異なる連結成分に属すか、kが範囲外なら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(log(H+2))。

### order_index

```text
int order_index(int v) const;
```

森全体でのDFS順の添字を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### subtree_range

```text
std::pair<int, int> subtree_range(int v) const;
```

DFS順の部分木半開区間 / 部分木の頂点数。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### subtree_size

```text
int subtree_size(int v) const;
```

指定頂点を含む部分木の頂点数を返します。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** O(N+B log(B+1))。

### swap

```text
void swap(LCA& other) noexcept;
void swap(LCA& a, LCA& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


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


## 構築・状態と補助型

### operator=

```text
LCA& operator=(const LCA& other);
LCA& operator=(LCA&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
