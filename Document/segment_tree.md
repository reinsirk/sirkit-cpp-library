# Segment Tree

`segtree<S,op,e>`は、点の値を置き換え、区間和・最小値などを求めます。添字は0始まり、区間は左端を含み右端を含まない[l,r)です。公開名・型引数・基本操作はACL形式で、`atcoder::`は付けません。[使用例](../Examples/segment_tree_example.cpp)を参照してください。

Sは値の型、opは左右の区間の答えを結合する関数、e()は空区間の答えです。opは括り方で結果が変わらず、e()を左右どちらに混ぜても結果を変えないことが必要です。左右の順序は保持するため、文字列連結や行列積にも使えます。

## API

| API | 動作 |
| --- | --- |
| `segtree()` / `(int n)` / `(vector<S>)` | 空列 / e()がn要素 / 指定した列で構築する。O(N+1)。 |
| `set(p,x)` / `transform(p,f)` | 添字pをx / f(現在値)へ置き換える。O(log(N+1))。 |
| `get(p)` / `all_prod()` | 点のS / 全体を集約したSをO(1)で返す。空列の全体積はe()。 |
| `prod(l,r)` | [l,r)を左からopでまとめたSを返す。O(log(N+1))。空区間はe()。 |
| `max_right(l,pred)` / `min_left(r,pred)` | 条件を満たす最大右端 / 最小左端をintで返す。O(log(N+1))。 |
| `size()` / `empty()` / `values()` | 要素数int / 空か / 列のコピー。列のコピーはO(N+1)。 |
| `assign(values)` / `reset(n)` / `reset()` | 列で再構築 / e()がn要素 / 現在の長さで全要素をe()へ戻す。 |
| `clear()` / `swap(other)` | 容量を保って空にする / 内容をO(1)で交換する。 |

```cpp
std::int64_t op(std::int64_t a,std::int64_t b) { return a+b; }
std::int64_t e() { return 0; }
// 以下はmain内。
segtree<std::int64_t,op,e> tree(std::vector<std::int64_t>{2,4,1});
tree.set(1,9);                // [2,9,1]。
auto sum = tree.prod(0,2);    // 11。
```

`prod`は掛け算専用ではなく、opで設定した集約です。上の例なら区間和になります。同じ呼出形式の関数オブジェクトも型引数に指定できます。

## 境界探索と利用上の注意

max_rightは左端lを固定して右へ、min_leftは右端rを固定して左へ区間を伸ばします。例えば[1,2,3,4]で「合計5以下」を条件にすると、max_right(1,pred)は3です。[1,3)の合計は5、[1,4)は9になります。関数bool pred(S)を定義し、`max_right<pred>(l)`・`min_left<pred>(r)`とも書けます。

述語は同じ値に同じ結果を返し、pred(e())がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。区間和の上限判定なら全要素が非負のときに使えます。単調性を確認する全要素走査は行いません。

演算・値のコピーをO(1)として、領域はO(N+1)です。assign・reset(n)は旧列の破棄を含めO(旧N+新N+1)、reset()はO(N+1)。列長・添字・区間端はintで、0<=p<N・0<=l<=r<=Nが前提です。値の演算は途中計算も型の範囲内に収めます。

不正な添字・区間・述語の単位元条件は既定で例外により検査し、`SKIP_RUNTIME_CHECKS`で除きます。容量・添字の保護は維持します。コピーは独立、移動元は空列として再利用できます。演算等の例外時に内容の強い保証は行いません。

公開APIは[ACLのsegtree](https://atcoder.github.io/ac-library/production/document_ja/segtree.html)、数学的参考は[モノイド設計](https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design)です。実装は本ライブラリ用のCC0です。内部最適化と測定は[Audit](../Audit/)に置きます。
