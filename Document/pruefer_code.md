# Prüfer符号と指定次数の木

`Graph/Pruefer_Code.cpp` は、頂点番号0..n−1の木をPrüfer符号へ変換し、符号から木を復元します。最小番号の葉を順に除き、その隣接頂点を記録する定義に従います。

```cpp
std::vector<int> code{3, 4, 3, 5, 4};
auto edges = pruefer_decode(7, code);
auto restored = pruefer_encode(7, edges); // codeと一致
auto degrees = pruefer_degrees(7, code); // 1,1,1,3,3,2,1
```

`pruefer_encode(n,edges)` はn−1本の辺、`pruefer_decode(n,code)` と `pruefer_degrees(n,code)` は長さmax(0,n−2)の符号を受け取ります。いずれも入力を変更せず、結果の `vector` を返します。n>=1で、符号の各値は0..n−1です。符号化するグラフは単純無向木とします。

| 頂点数 | 符号 | 辺 | 次数 |
|---|---|---|---|
| 1 | 空 | 空 | `{0}` |
| 2 | 空 | `{0,1}` | `{1,1}` |

符号だけを渡す `pruefer_decode(code)` はn=code.size()+2を採用します。1頂点を扱う場合には頂点数を明示してください。

符号化では各頂点の残存次数と隣接頂点番号のXORを持ち、葉の隣接頂点を定数時間で取り出します。葉の探索位置は右へ単調に進め、新たな葉が探索位置より小さいときだけ、その頂点を次に処理します。各探索位置を高々一度通るため、符号化・復元ともに O(n) 時間・領域です。入力の辺順・向きには依存しません。最大番号n−1は最後の2頂点まで残ります。

`pruefer_from_degrees(degrees)` は指定次数を持つ木の符号を、`tree_from_degrees(degrees)` はその辺を返します。存在しない場合は `nullopt` です。n>=2なら、全次数が1以上で総和が2(n−1)であることが必要十分です。頂点vを次数−1回ずつ昇順に並べた符号を復元します。n=1では `{0}` だけを認め、空の次数列は認めません。計算量は O(n) です。

```cpp
auto tree = tree_from_degrees(std::vector<int>{1, 2, 1});
auto bad = tree_from_degrees(std::vector<int>{2, 2, 2}); // nullopt
```

`complete_tree_with_degrees(degrees,edges)` は、既存の森の辺を残して、指定次数の木になるような**追加辺だけ**を返します。実現できなければ `nullopt` です。既存辺による循環、負の残余次数、接続できない成分も `nullopt` になります。各成分の残余次数を新しい次数としてPrüfer復元を行い、各成分の頂点へ接続先を戻します。時間 O((n+m)α(n))、領域 O(n+m) です。

```cpp
std::vector<int> degrees{1, 2, 1, 2, 2, 2};
std::vector<std::pair<int,int>> forest{{1,2}, {0,3}};
auto added = complete_tree_with_degrees(degrees, forest);
```

既定では入力長・頂点番号・辺数を、通常の次数構築や葉除去に組み込んで検査します。木でない入力の検査のためだけの全辺走査は行いません。`SKIP_RUNTIME_CHECKS` はこれらの入力検査を除きます。次数列の実現可能性、森の完成が可能かの判定、件数の表現上限は省略しません。割当失敗時も入力は変更しません。

数学的参考は [AALのPrüfer code](https://info.atcoder.jp/entry/algorithm_lectures/prufer_code) です。記事の符号の定義・次数との対応に基づき、線形の探索とXORによる実装を独立に記述しています。
