# 重軽分解

`with_values<Range>(values)`は、木の分解と選んだBIT／Segtreeを所有する`HeavyLightValues`を返します。初期値は**values[v]が頂点vの値**になる順で渡し、操作も頂点番号を使います。並べ替え・添字変換は内部で行います。[本体](../Graph/Heavy_Light_Decomposition.cpp)・[完全な使用例](../Examples/heavy_light_decomposition_example.cpp)。

```cpp
Graph tree{{1}, {0,2}, {1}}; // 0―1―2、根は既定で0。
std::vector<std::int64_t> value{10,20,30};
auto hld = HeavyLightDecomposition(tree)
    .with_values<fenwick_tree<std::int64_t>>(value);
hld.add(1, 5);                       // 頂点1を20→25にする。
auto sum = hld.path_prod(0, 2);       // 0→1→2の和、65。
auto subtree = hld.subtree_prod(1);  // 頂点1とその子孫の和、55。
auto x = hld.get(1);                 // 25。value[1]自体は20のまま。
```

## 集計器と演算の選び方

| Rangeに指定する型 | 向いている操作 |
| --- | --- |
| `fenwick_tree<T>` | 頂点への加算と経路・部分木の和。`get/set/add`が使える。 |
| `segtree<S,op,e>` | 頂点の代入と経路・部分木の最小値・最大値・積など。`get/set`が使える。 |
| `lazy_segtree<S,op,e,F,mapping,composition,id>` | 点・経路・部分木の一括更新も必要な場合。`get/set/apply`が使える。 |

Tは値、Sは各頂点に置く値と集計結果の型で、初期値はそれぞれvector<T>／vector<S>です。op(a,b)は2つの結果を結合し、e()は空区間の結果（左右の単位元）を返します。opには結合則が必要です。Fは更新内容の型、mapping(f,x)は更新後の集計値、composition(newer,older)は古い更新の後に新しい更新を行う合成、id()は何もしない更新を返します。[型引数の詳細](lazy_segment_tree.md)と完全例に範囲加算の定義があります。使う集計器も貼り付けます。実ACLも使用でき、Mintは利用側で用意します。

## 操作と経路の向き

以下、Nは頂点数、Mは辺数、Vは`HeavyLightValues<Range,Ordered>`、value_typeはBITならT、Segtree／LazyならSです。頂点・添字はint、祖先・jumpのkはstd::int64_t。通常の集計器で、値の演算・コピーをO(1)とした計算量です。

| VのAPI | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `value_type` | 保持する値・集計結果の型。 | — |
| `V()` | 値と分解が空の状態を作る。集計器は未構築。 | O(1) |
| `V(g,values,root=0)` | Graphと頂点順のvectorから構築する。 | 分解＋集計器構築 |
| `V(const HeavyLightDecomposition& layout,values)` / `V(HeavyLightDecomposition&& layout,values)` | 既存の分解をコピー／移動し、頂点順の値を取り込む。 | 分解のコピー／移動＋並べ替え＋集計器構築 |
| `V(const V&)` / `V(V&&)` / `operator=(V other)` | 独立コピー／移動構築／コピー・移動代入。代入は自身へのV&を返す。 | コピーは保持領域に比例、移動構築O(1) |
| `build(g,values,root=0)` | 分解と値を一緒に再構築。戻り値void。 | 分解＋集計器構築 |
| `clear()` / `swap(V& other)` | 空にする／同じ型と交換。戻り値void。 | 保持領域の解放／O(1) |
| `get(v)` / `set(v,x)` | value_typeで頂点vの値を返す／xを代入しvoid。 | getはSegtreeでO(1)、他はO(log N)。setはO(log N) |
| `add(v,x)` | 頂点vへ加算しvoid。addを持つBIT用。 | O(log N) |
| `apply(v,f)` | 頂点vへ作用しvoid。点applyを持つ集計器用。 | O(log N) |
| `path_prod(u,v,edges=false)` / `subtree_prod(v,edges=false)` | 両端を含む経路／vを含む部分木の結果をvalue_typeで返す。prodは和・最小値など、Rangeで決めた集計。 | O(log²N)／O(log N) |
| `path_apply(u,v,f,edges=false)` / `subtree_apply(v,f,edges=false)` | 経路／部分木へ作用しvoid。区間applyを持つ集計器用。 | O(log²N)／O(log N) |
| `values()` | 全値を頂点番号順のvector<value_type>で返す。 | Range.valuesの費用＋O(N)、なければN回get |

get・集計・valuesを含む値の操作は非constです。以下の「分解結果を取り出すAPI」はVからも同名で呼べます。Vにはwith_valuesや外部集計器を渡すオーバーロードはありません。

**既定のOrdered=falseは、順序を入れ替えても結果が同じ演算用**です。和・最小値・最大値はこのまま使えます。文字列連結・関数合成などはtrueにし、u→vの順に集計します。trueでは集計器2本分の領域・更新を使います。値の順を反転し、個々の辺の値を逆元にする機能ではありません。concatを文字列連結、empty_wordを空文字列を返す関数とすると、上の木で次のように使えます。

```cpp
using Words = segtree<std::string, concat, empty_word>;
std::vector<std::string> token{"a","b","c"};
auto words = HeavyLightDecomposition(tree).with_values<Words,true>(token);
auto forward = words.path_prod(0,2); // "abc"。
auto backward = words.path_prod(2,0); // "cba"。
```

**辺を扱うときは、親辺の値を子頂点の位置へ渡し、edges=trueにします。** 初期値はN個必要で、各根には0やe()を置きます。根0の0―1―2では、辺(0,1)をvalues[1]、辺(1,2)をvalues[2]へ置きます。`path_prod(0,2,true)`はこの2辺を集計し、`subtree_prod(1,true)`は部分木の内部の辺(1,2)だけを集計します。`set(2,x)`は親辺(1,2)の更新です。経路ではLCA、部分木では指定頂点の位置を除きます。`path_prod(v,v)`は頂点vの値、`path_prod(v,v,true)`は空経路の単位元です。葉の辺部分木も空で、空区間の更新は何もしません。非可換な部分木積はDFS添字順です。

## 分解の構築・所有

Hは`HeavyLightDecomposition`です。Graphは`std::vector<std::vector<int>>`の別名で、無向森の隣接リストを渡します。

| HのAPI | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `H()` / `H(g,root=0)` | 空／指定根からの分解を構築する。 | O(1)／O(N+M) |
| `H(const H&)` / `H(H&&)` / `operator=(H other)` | 独立コピー／移動構築／コピー・移動代入。代入は自身へのH&を返す。 | コピーO(N)、移動構築O(1) |
| `build(g,root=0)` / `clear()` / `swap(H& other)` | 再構築／空にする／交換。戻り値void。 | O(N+M)／旧領域の解放／O(1) |
| `with_values<Range,Ordered=false>(values) const &` | 長さNの頂点順vectorを受け、分解をコピーしてVを返す。 | コピー＋並べ替え＋集計器構築 |
| `with_values<Range,Ordered=false>(values) &&` | 同じ入力から、分解を移動してVを返す。 | 並べ替え＋集計器構築 |

コピー／moveの代入とbuildは、表の費用に旧状態の破棄費用も加わります。with_valuesは元の値配列の参照を保持しません。

## 分解結果を取り出すAPI（H・V共通）

この表の操作はconstから呼べます。区間のleft/rightは元の頂点番号ではなく、並べ替え後のDFS添字です。`H::Interval`は公開型`{int left; int right; bool reversed;}`で、Vに同名の型別名はありません。

| API | 戻り値・動作 | 計算量 |
| --- | --- | --- |
| `size()` / `empty()` | intの頂点数／boolの空判定。 | O(1) |
| `root()` / `component_count()` | intの指定根（空なら-1）／木の数。 | O(1) |
| `parent(v)` / `depth(v)` | intの親（各根で-1）／各根からの深さ（根で0）。 | O(1) |
| `component(v)` / `root_of(v)` | intの木の番号（0始まり）／所属する木の根。 | O(1) |
| `index(v)` / `vertex_at(i)` | intの頂点→DFS添字／添字→頂点。互いに逆変換。 | O(1) |
| `order()` | 添字iにvertex_at(i)を置くvector<int>のconst参照。 | O(1) |
| `reorder(values)` / `restore(ordered)` | 長さNのvector<T>をDFS添字順／頂点順へコピーして返す。Tはコピー可能なら既定構築不要。 | O(N)時間・追加領域 |
| `same_component(u,v)` / `is_ancestor(u,v)` | boolの同じ木か／uがvの祖先か。自身は祖先、別の木なら祖先ではない。 | O(1) |
| `subtree_range(v,edges=false)` | pair<int,int>の半開区間[l,r)。edges=trueはvの位置を除く。 | O(1) |
| `subtree_size(v)` | vを含む部分木の頂点数をintで返す。 | O(1) |
| `lca(u,v)` / `distance(u,v)` | intのLCA／経路の辺数。別の木なら-1。 | O(log N) |
| `kth_ancestor(v,k)` | k個上の頂点をintで返す。k=0はv、負数・根を超える場合は-1。 | O(log N) |
| `jump(u,v,k)` | u→vの0始まりk番目の頂点をintで返す。負数・範囲外・別の木は-1。 | O(log N) |
| `for_each_path(u,v,fn,edges=false)` | 通行順にfn(left,right,reversed)を呼び、接続ならtrue、別の木ならfalse。別の木では呼ばず、fnの戻り値は無視する。 | O(log N)回の呼出し、HLD側のヒープ確保なし |
| `path(u,v,edges=false)` | 同じ通行順のvector<H::Interval>を返す。別の木は空。 | O(log N)時間・領域 |

for_each_pathはfnの実行費用も加わります。reversed=trueはright-1からleftへ読みます。辺モードでu=vならfor_each_pathは呼出しなしでtrue、pathは空です。区間から頂点を取り出す場合はvertex_atを使います。

```cpp
HeavyLightDecomposition layout(tree); // BIT／Segtreeなしで使える。
auto [l,r] = layout.subtree_range(1); // vertex_at(l)..vertex_at(r-1)が部分木。
auto parts = layout.path(2,0);
std::vector<int> route;
for (auto [left,right,reversed] : parts)
    for (int i=0; i<right-left; ++i)
        route.push_back(layout.vertex_at(reversed ? right-1-i : left+i));
// 上の0―1―2の木ではrouteは{2,1,0}。
```

## 外部集計器を使うAPI（Hのみ）

Hはconstでも呼べます。bit／seg／lazyにはDFS添字順・長さNの集計器を渡します。参照は保持しません。Qを区間照会1回の費用、Aを区間更新1回の費用とします。

| API | 戻り値・前提 | 計算量 |
| --- | --- | --- |
| `path_prod(u,v,bit,edges=false)` | sumを持つBITの和。型はsumの戻り値、単位元はT{}。 | O(log N)回の照会 |
| `path_prod(u,v,seg,edges=false)` | ACL形式segtreeのop/eを使った可換積。segはconst可、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,lazy,edges=false)` | ACL形式lazy_segtreeのop/eを使った可換積。lazyは非const、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,seg,reverse,edges=false)` | segの非可換積。reverseは集計値を正しい逆順集計値へ変換する。戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,lazy,reverse,edges=false)` | lazyの非可換積。reverseの条件は同じ。lazyは非const、戻り値S。 | O(log N)回の照会 |
| `path_prod(u,v,identity,query,op,edges=false)` | 単位元・照会・結合演算を指定。queryの戻り値型で結果を返す。 | O(log N)回の照会・結合 |
| `path_prod(u,v,identity,query,op,reverse,edges=false)` | 上と同じで、集計値の逆順変換を追加。 | O(log N)回の照会・結合 |
| `subtree_prod(v,query,edges=false)` | 部分木のDFS添字昇順の積。戻り値型はqueryから決まる。 | Q |
| `path_apply(u,v,range,f,edges=false)` / `subtree_apply(v,range,f,edges=false)` | range.apply(l,r,f)で更新。戻り値void。fは値で受け取る。 | O(log N)回の更新／A |

queryはprod/sumを持つ集計器か、(l,r)／(l,r,reversed)から値を返す関数です。2引数は可換専用、3引数は向きに合った結果を返します。reverseと3引数queryは併用しません（std::identityは除く）。空区間は単位元を返してください。reverseは要素順を逆にした集計値への変換です。単一の結果だけで逆順を復元できない場合は3引数queryを使います。opは結合的でidentityを左右の単位元とし、区間照会・値のコピー・結合の費用を表の回数に加えます。

## 前提・所有権・計算量

入力は頂点0..N-1の対称な隣接リストで表す無向森です。指定根の木から構築し、残りは最小未訪問頂点を根にします。空の入力にはroot=0。経路集計・更新の両端は同じ木が前提で、検査時は別の木を例外にします。頂点範囲・自己辺・閉路・多重辺は探索中に検査し、対称性確認だけの全辺走査はしません。`SKIP_RUNTIME_CHECKS`で入力検査と検査専用状態を除きます。BITの整数和は型幅で周回します。

コピーは独立、move元は空です。元のlayout・値配列とも更新を共有しません。木の形や根を変えるときはbuildへ値も渡し、辺の値は新しい親子対応で配置し直します。build・コピー代入の失敗時は元を保持しますが、移動を指定した分解からの構築失敗では移動元が空になる場合があります。更新中の例外では巻き戻さず、Ordered=trueはclear/buildで再構築してください。pathが返す配列は独立所有、orderは参照です。clear・再構築・代入・破棄後も並びが必要ならコピーを保持してください。get・集計・valuesも非constのHLDから呼びます（Lazyは照会時も内部を変更します）。

並べ替えはO(N)、その後に集計器を構築します。vector構築のない実ACL FenwickはN回のaddでO(N log N)。所有状態を固定1回確保し、破棄時には所有領域を解放します。表の計算量は演算・コピーをO(1)と仮定し、文字列などは長さに応じた費用が加わります。

[API設計の参考（Nyaan・CC0）](https://github.com/NyaanNyaan/library/blob/2e713880f0d5041d794e0f09c7f31c1826ba0ccf/tree/heavy-light-decomposition.hpp)・[理論参考](https://info.atcoder.jp/entry/algorithm_lectures/heavy_light_decomposition)・[分解の参照試験](../Verify/current/tests/new_hld_reference_test.cpp)・[所有版の参照試験](../Verify/current/tests/hld_owned_test.cpp)
