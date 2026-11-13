# 隣接順を保つ全方位木DP

`RerootingDP<T>`は、森の各頂点を根にした場合のDPをまとめて求めます。例えば「各頂点から全頂点までの距離和」を全ての頂点について計算できます。[本体](../Graph/Rerooting_DP.cpp)・[使用例](../Examples/rerooting_dp_example.cpp)を参照してください。

## 計算の設定

`build(identity,merge,to_vertex,to_edge)`で、辺から受け取る値と頂点の答えを同じ型Tで定義します。

| 引数 | 役割 |
| --- | --- |
| `identity` | メッセージがない場合の値。mergeの左右どちらに渡しても結果を変えない。 |
| `merge(const T&,const T&)->T` | 複数の隣接先から受け取った値を結合する。括り方で結果が変わらないこと。 |
| `to_vertex(const T&,int v)->T` | 結合した値から、頂点vを根とする答えを作る。 |
| `to_edge(const T&,int from,int to,size_t edge_id)->T` | 頂点の値を、fromからtoへ送る値に変える。 |

uからvへ送る値は、v以外からuへ届いた値を結合し、to_vertex、to_edgeの順に変換したものです。頂点uの全体の答えは、全隣接先から届いた値を結合してto_vertexを適用します。結合順は各頂点での**辺の追加順**なので、文字列連結のように順番が重要な演算にも使えます。

```cpp
struct State { std::int64_t sum=0,count=0; };
RerootingDP<State> tree(3);
tree.add_edge(0,1);
tree.add_edge(1,2);
tree.build(State{},
    [](const State& a,const State& b){ return State{a.sum+b.sum,a.count+b.count}; },
    [](const State& a,int){ return State{a.sum,a.count+1}; },
    [](const State& a,int,int,std::size_t){ return State{a.sum+a.count,a.count}; });
// tree[0].sum==3、tree[1].sum==2、tree[2].sum==3。
```

countは頂点数、sumは距離和です。頂点を含めるとcountを1増やし、辺を渡ると各頂点への距離が1ずつ増えるのでsumにcountを加えます。

## API

| API | 動作 |
| --- | --- |
| `RerootingDP<T>(n=0)` / `add_edge(u,v)` | n頂点で構築／無向辺を追加してsize_tの辺IDを返す。 |
| `build(...)` / `values()` / `operator[](v)` | 全根結果のconst spanを返す／同じspan／頂点vの答えのconst参照。 |
| `message(id,reverse=false)` / `messages()` | 追加時u→vの値（trueなら逆向き）／全方向のconst span。 |
| `edge(id)` / `edges()` / `neighbors(v)` | 追加時の端点pair／辺表span／出るarc番号のspan。 |
| `size()` / `empty()` / `edge_count()` / `has_results()` | 頂点数／0頂点か／辺数／現在の結果があるか。各O(1)。 |
| `component_count()` | build後の森の連結成分数。 |
| `reserve_edges(m)` / `reserve_neighbors(v,d)` | 辺表／隣接列を予約する。 |
| `reset(n)` / `clear()` / `swap(other)` | 辺と結果を消してn頂点へ／0頂点へ／交換。 |

各辺idのarcは2*idが追加時のu→v、2*id+1がv→uです。結果取得はhas_results()==trueが前提で、辺を追加したら再buildします。空の森も扱い、孤立点vの答えはto_vertex(identity,v)です。

## 前提・所有と計算量

頂点は0..N-1で、自己辺・多重辺・閉路を認めません。Tはbool・cv修飾型以外でコピー・代入可能、破棄が例外を投げないことが必要です。デフォルト構築は不要です。関数はconst参照を通してbuild中だけ呼び、同じ入力に同じ結果を返すものを使います。実行中に同じグラフを変更しないでください。

値の演算・コピーをO(1)として、buildはO(N+M)時間・追加領域です。探索とDPは反復で行います。結果の参照取得はO(1)、辺追加は償却O(1)、再確保・コピーは保持量に比例します。コピーは独立、move元は空、swapはO(1)です。

build・コピー代入・resetの失敗時は旧内容を保ちます。build失敗時は旧結果の参照も維持します。辺追加に失敗した場合も辺・結果を保ちますが、辺表・隣接列の容量と以前のspanは復元しません。関数の外部状態は巻き戻しません。成功した再構築・reset・clear・代入や、対応配列の再確保で参照は無効化し得ます。

`SKIP_RUNTIME_CHECKS`で頂点・辺番号・結果の有無・森の検査を除き、内部件数・添字の上限は維持します。森の検査は通常の探索に組み込み、検査専用の全辺走査はしません。数学的参考は[Nyaan](https://nyaannyaan.github.io/library/tree/rerooting.hpp.html)、機能上の参考は[Luzhiled](https://ei1333.github.io/library/graph/tree/rerooting.hpp.html)。独立した反復実装を[Library Checker](https://judge.yosupo.jp/problem/tree_path_composite_sum)と各根の素朴な再計算で検証します。
