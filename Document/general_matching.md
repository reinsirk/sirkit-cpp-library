# 一般グラフの最大マッチング

`GeneralMatching`は、無向グラフから端点を共有しない辺を最大何本選べるか求めます。奇数閉路を含むグラフにも使えます。重みは扱いません。[本体](../Graph/General_Matching.cpp)・[使用例](../Examples/general_matching_example.cpp)を参照してください。

## API

### GeneralMatching

```text
explicit GeneralMatching(std::int32_t n = 0);
GeneralMatching(const GeneralMatching&);
GeneralMatching(GeneralMatching&& other) noexcept;
```

n頂点の空グラフを作ります。コピーは独立、ムーブ元は空です。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** O(N)時間・領域。コピーはO(N+M)、ムーブはO(1)。

### add_edge

```text
void add_edge(std::int32_t u, std::int32_t v);
```

無向辺を追加します。自己辺は無視し、多重辺は記録します。現在の対応は保ち、最大性を未確認にします。

**制約:** u,vは0..N-1。

**計算量:** 償却O(1)。再確保時は現在の記録辺数に比例します。

### solve

```text
std::int32_t solve();
```

現在の対応を最大化し、総本数を返す。辺追加後も継続できる。

**制約:** 構築済みの入力が共通の前提を満たすこと。

**計算量:** O(N³+NM)時間・O(N+M)追加領域。最大性が既知または完全対応ならO(1)。

### mate

```text
std::int32_t mate(std::int32_t vertex) const;
```

対応する相手の番号を返します。未対応なら-1です。

**制約:** 引数の頂点番号・添字は有効範囲内です。その他の前提は共通条件に従います。

**計算量:** O(1)。

### partners

```text
std::span<const std::int32_t> partners() const noexcept;
```

現在の全対応を読み取り専用spanで返します。

**制約:** なし。

**計算量:** O(1)。

### pairs

```text
std::vector<std::pair<std::int32_t, std::int32_t>> pairs() const;
```

u<vの対応辺をuの昇順でvectorにして返す。

**制約:** なし。

**計算量:** O(N)時間・O(対応本数)領域。

### matching_size

```text
std::int32_t matching_size() const noexcept;
```

現在のマッチングの本数を返します。

**制約:** なし。

**計算量:** O(1)。

### is_maximum

```text
bool is_maximum() const noexcept;
```

現在のマッチングが最大と確認済みならtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### is_perfect

```text
bool is_perfect() const noexcept;
```

全頂点に対応相手があればtrueを返します。

**制約:** なし。

**計算量:** O(1)。

### size

```text
std::int32_t size() const noexcept;
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

### edge_count

```text
std::size_t edge_count() const noexcept;
```

登録した元辺の数を返します。

**制約:** なし。

**計算量:** O(1)。

### edges

```text
std::span<const std::pair<std::int32_t, std::int32_t>> edges() const noexcept;
```

登録した元辺を追加順で返します。

**制約:** なし。

**計算量:** O(1)。

### reserve_edges

```text
void reserve_edges(std::size_t capacity);
```

指定件数を保持できる辺の記録容量を予約します。

**制約:** 指定件数は内部の容量上限以内です。

**計算量:** 再確保する場合は現在の保持要素数に比例します。

### clear_matching

```text
void clear_matching() noexcept;
```

辺を残してマッチングだけを消します。

**制約:** なし。

**計算量:** O(N)。

### clear_edges

```text
void clear_edges() noexcept;
```

頂点数と容量を残し、辺とマッチングを消します。

**制約:** なし。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧N+旧M+新N)。

### reset

```text
void reset(std::int32_t n = 0);
```

旧状態を破棄し、指定頂点数の空グラフへ戻します。

**制約:** 頂点数・入力構造・型の前提を満たすこと。空の構築も可能です。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧N+旧M+新N)。

### clear

```text
void clear();
```

領域を解放して空の状態へ戻します。

**制約:** なし。

**計算量:** 旧状態の破棄と新頂点の準備を含めO(旧N+旧M+新N)。

### swap

```text
void swap(GeneralMatching& other) noexcept;
void swap(GeneralMatching& a, GeneralMatching& b) noexcept;
```

保持するグラフ・結果・状態を交換します。

**制約:** なし。

**計算量:** O(1)。


```cpp
GeneralMatching graph(3);
graph.add_edge(0,1);
graph.add_edge(1,2);
graph.add_edge(2,0);
auto count = graph.solve(); // 1。
auto pairs = graph.pairs(); // 3辺のうちいずれか1辺。
```

## 対応と計算量

頂点番号は0..N-1で、左右への分割や端点の大小関係は不要です。多重辺は記録上は残しますが、対応では同じ頂点対を区別しません。辺を追加すると最大性は未確認になりますが、現在の対応は有効です。再solveで本数は減りませんが、相手が変わる場合があります。空グラフは最大かつ完全対応と扱います。

記録辺数MとしてsolveはO(N³+NM)時間・追加O(N+M)領域です。単純グラフではO(N³)。最大性が既知、または既に完全対応ならO(1)です。

件数・相手・span取得はO(1)、辺追加は償却O(1)、再確保はO(M)、pairsはO(N)時間・O(対応本数)領域。clear_matchingはO(N)、clear_edgesはO(N+M)。resetは旧データの破棄と新頂点の初期化を含み、所有領域とコピーはO(N+M)です。

## 所有・検査

コピーは独立、move元は0頂点・0辺・0対応、swapはO(1)です。辺追加・reset・コピー代入に失敗しても以前の内容を保ちます。solveは全作業領域を確保してから対応を変更するため、確保失敗時は辺・対応・最大性と既存spanを維持します。

partnersのspanはsolve・clear_matchingで値が更新され、reset・clear・代入・所有権移動で無効になります。edgesのspanは辺追加・予約・clear_edgesでも無効化し得ます。`SKIP_RUNTIME_CHECKS`で負の長さと頂点番号の検査を除き、内部の容量・添字保護は維持します。


## 構築・状態と補助型

### operator=

```text
GeneralMatching& operator=(const GeneralMatching& other);
GeneralMatching& operator=(GeneralMatching&& other) noexcept;
```

コピー／ムーブ代入し、自身への参照を返します。

**制約:** なし。

**計算量:** コピーは保持領域に比例します。旧状態の破棄費用を加えます。
