# FunctionalGraph

各頂点から出る辺が1本の有向グラフを分解します。頂点は0..N-1です。自己辺、複数の周期、周期へ合流する木を扱います。各弱連結成分は有向周期を1つ持ちます。

## FunctionalGraph

```text
FunctionalGraph();
explicit FunctionalGraph(std::span<const int> successors);
FunctionalGraph(const FunctionalGraph&);
FunctionalGraph(FunctionalGraph&& other) noexcept;
```

引数を省略すると空のグラフを作り、行先の配列を渡すとその配列から構築します。行先はグラフがコピーして所有するため、構築後に入力は参照しません。

**制約:** 全ての行先は0..N-1。空列も可。

**計算量:** 空の構築O(1)、列からの構築・コピーO(N log(N+1))時間・領域、ムーブO(1)。

## build

```text
void build(std::span<const int> successors);
```

行先配列から再構築します。自身のsuccessors()を入力にしても構いません。

**制約:** 全ての行先は0..N-1。

**計算量:** O(N log(N+1))時間・領域。旧状態の破棄費用も加わります。

## kth_successor

```text
int kth_successor(int v, std::uint64_t k) const;
```

vからk本の辺を進んだ頂点を返します。k=0はvです。

**制約:** 0<=v<N。kはstd::uint64_t全域を扱います。

**計算量:** O(log(N+1))。

## distance

```text
std::int64_t distance(int from, int to) const;
```

fromからtoへ初めて到達する辺数を返します。到達不能は-1、自身へは0です。

**制約:** from,toは有効な頂点番号。

**計算量:** O(log(N+1))。

## reachable

```text
bool reachable(int from, int to) const;
```

fromからtoへ到達可能ならtrueを返します。同じ弱連結成分でも到達不能な場合があります。

**制約:** from,toは有効な頂点番号。

**計算量:** O(log(N+1))。

## info

```text
CycleInfo info(int v) const;
```

CycleInfo{id,entry,depth,position,length}を返します。idは到達する周期の番号、entryはその周期で最初に到達する頂点、depthはそこまでの辺数です。positionは周期内でのentryの添字、lengthは周期の長さです。

**制約:** 0<=v<N。

**計算量:** O(1)。

## cycle

```text
std::span<const int> cycle(int id) const;
```

successor方向に並ぶ周期頂点への読み取り専用spanを返します。周期の始点やID順は未規定です。

**制約:** 0<=id<cycle_count()。

**計算量:** O(1)。

## successor

```text
int successor(int v) const;
```

vの行先を返します。

**制約:** 0<=v<N。

**計算量:** O(1)。

## successors

```text
std::span<const int> successors() const noexcept;
```

全頂点の行先への読み取り専用spanを返します。

**制約:** なし。

**計算量:** O(1)。

## on_cycle

```text
bool on_cycle(int v) const;
```

vが周期上ならtrueを返します。周期上ではinfo(v).depth=0、entry=vです。

**制約:** 0<=v<N。

**計算量:** O(1)。

## cycle_count

```text
int cycle_count() const noexcept;
```

周期の個数を返します。空グラフでは0です。

**制約:** なし。

**計算量:** O(1)。

## component_size

```text
int component_size(int v) const;
```

vと同じ弱連結成分全体の頂点数を返します。周期長とは異なります。

**制約:** 0<=v<N。

**計算量:** O(1)。

## size

```text
int size() const noexcept;
```

頂点数を返します。

**制約:** なし。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

0頂点ならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

## clear

```text
void clear() noexcept;
```

空へ戻します。

**制約:** なし。

**計算量:** 保持領域の解放に比例します。

## swap

```text
void swap(FunctionalGraph& other) noexcept;
void swap(FunctionalGraph& a, FunctionalGraph& b) noexcept;
```

状態を交換します。

**制約:** なし。

**計算量:** O(1)。

## operator=

```text
FunctionalGraph& operator=(const FunctionalGraph& other);
FunctionalGraph& operator=(FunctionalGraph&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーはO(N log(N+1))、旧状態の破棄費用を加えます。

## 所有・例外

コピーは元のグラフから独立し、move元は空になります。buildやコピー代入で領域を確保できなかった場合も、以前の状態とviewは有効です。viewは再構築・clear・代入・所有権の移動が成功するまで使えます。

頂点・周期ID・入力の行先を既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。容量保護は維持します。

```cpp
std::vector<int> next{2, 0, 3, 2};
FunctionalGraph graph(next);
int v = graph.kth_successor(1, 1000000000000000000ULL);
auto distance = graph.distance(1, 3); // 3
```

[完全な使用例](../Examples/functional_graph_example.cpp)・[AtCoder ABC367 E](https://atcoder.jp/contests/abc367/tasks/abc367_e)
