# Lazy Segment Tree

`lazy_segtree<S,op,e,F,mapping,composition,id>`は、配列の区間をまとめて更新し、区間和・最小値などを求めます。添字は0始まり、区間は左端を含み右端を含まない`[l,r)`です。[加算・区間和の使用例](../Examples/lazy_segment_tree_add_sum_example.cpp)、[代入も扱う使用例](../Examples/lazy_segment_tree_example.cpp)を参照してください。

演算とS・FのコピーをO(1)として、構築はO(N+1)時間・領域、更新と区間照会はO(log(N+1))です。更新の子への反映は必要になるまで保留し、複数の更新をcompositionでまとめます。

## 型と関数の設定

| 設定 | 役割 | 区間加算・区間和の場合 |
| --- | --- | --- |
| `S` | 区間の答えを表す型 | 合計と要素数の組 |
| `op(left,right)` | 左右の区間の答えを結合する | 合計と要素数をそれぞれ足す |
| `e()` | 空区間の答え。opに混ぜても結果を変えない | 合計0・要素数0 |
| `F` | 1回の更新を表す型 | 各要素に足す量 |
| `mapping(f,x)` | 更新fを区間の答えxへ反映する | 合計に「加算量×要素数」を足す |
| `composition(newer,older)` | olderの後にnewerを適用する更新を作る | 加算量を足す |
| `id()` | 何も変更しない更新 | 加算量0 |

opは括り方で結果が変わらないことが前提です。左右の順序は保持します。mappingは「結合してから更新」と「更新してから結合」で同じ結果になるよう定義します。**e()は空区間、id()は何もしない更新**です。

## API

| API | 動作 |
| --- | --- |
| `lazy_segtree()` / `(int n)` / `(vector<S>)` | 空列 / e()がn要素 / 指定した列で構築する。 |
| `set(p,x)` / `get(p)` | 添字pの値をSで置き換える / 現在のSを返す。 |
| `apply(p,f)` / `apply(l,r,f)` | 1要素 / [l,r)へ更新fを適用する。 |
| `prod(l,r)` / `all_prod()` | [l,r) / 全体をopでまとめたSを返す。全体はO(1)。空はe()。 |
| `max_right(l,pred)` / `min_left(r,pred)` | 条件を満たす最大の右端 / 最小の左端をintで返す。O(log(N+1))。 |
| `size()` / `empty()` | 要素数 / 空かをO(1)で返す。 |
| `values()` | 現在の全要素をvector<S>にコピーする。O(N+1)。 |
| `assign(values)` / `reset(n)` / `reset()` | 列で作り直す / e()がn要素 / 現在の長さで全要素をe()に戻す。 |
| `clear()` / `swap(other)` | 容量を保って空にする / 内容を交換する。O(旧N+1) / O(1)。 |

`prod`はopで決めた集約で、掛け算とは限りません。applyは設定した更新、setは直接の置換です。assign・reset(n)はO(旧N+新N+1)、reset()はO(N+1)です。

## 区間加算・区間和の例

```cpp
struct Sum { std::int64_t value; int length; };
using F = std::int64_t;
Sum op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(F add, Sum x) { return {x.value + add * x.length, x.length}; }
F composition(F newer, F older) { return newer + older; }
F id() { return 0; }
using Tree = lazy_segtree<Sum, op, e, F, mapping, composition, id>;
// 以下はmain内。
Tree tree(std::vector<Sum>{{1,1}, {2,1}, {3,1}});
tree.apply(0, 2, 3);                 // 列は[4,5,3]。
auto sum = tree.prod(0, 3).value;     // 12。
tree.set(1, Sum{10,1});              // 列は[4,10,3]。
```

lengthは、区間への加算量を「add×要素数」で求めるために持ちます。**1要素は{値,1}、空区間は{0,0}**です。Tree(n)やresetではlengthも0になるため、値0の配列は`Tree tree(std::vector<Sum>(n,Sum{0,1}))`で作ります。

代入・乗算も必要なら、Fを`{a,b}`で表し、各要素を`x -> a*x+b`で更新します。`{1,b}`は加算、`{0,b}`は代入、id()は`{1,0}`です。合成は`{newer.a*older.a, newer.a*older.b+newer.b}`。例えば「2*x+3の後に5*x+7」は10*x+22になります。

## 境界探索と利用上の注意

max_rightは左端を固定して右へ、min_leftは右端を固定して左へ区間を伸ばします。例えば[1,2,3,4,5]で「合計が5以下」なら、`max_right(1,pred)`は3を返します。[1,3)は合計5、[1,4)は9です。関数bool pred(S)を定義し、`max_right<pred>(l)`・`min_left<pred>(r)`とも書けます。

述語は同じ値に同じ結果を返し、`pred(e())`がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。区間和の上限判定では、要素が非負なら使えます。負数が混じる場合、この単調性は保証できません。

本ライブラリ独自の拡張として`mapping(F,S,std::size_t length)`も使えます。区間長を受け取れるため、上の例のlengthをSから省けます。2引数形も呼べる場合はそちらを優先します。更新によって区間長を変えてはいけません。

列長・添字・区間端はintで、`0<=p<N`・`0<=l<=r<=N`が前提です。NはINT_MAX以下かつ内部容量に収めます。利用側の演算は途中計算も含めて型の範囲内に収め、Mintが必要なら利用側で用意します。

不正な添字・区間・述語の単位元条件は既定で例外により検査します。`SKIP_RUNTIME_CHECKS`で入力検査を除き、容量・内部添字の保護は維持します。照会も遅延伝播で内部状態を変更するため、get・prod・境界探索・valuesは非constです。コピーは独立、ムーブ元は空列として再利用できます。演算やコピーが例外を投げる場合、内容の強い保証は行いません。

型引数と基本操作は[ACLのAPI](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html)に合わせています。数学的参考は[モノイド設計](https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design)、実装は本ライブラリ用のCC0です。検証記録は[Audit](../Audit/)に置きます。
