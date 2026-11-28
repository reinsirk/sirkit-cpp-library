# Sqrt Tree

`SqrtTree<T, Op>(values, identity, op)` は結合的な演算 `op` と単位元 `identity` を使って、固定した列の区間積を求めます。演算が非可換でも左から順に掛けます。

- `prod(l, r)` は半開区間 `[l,r)`、`all_prod()` は全列の積です。空区間の積は単位元になります。
- `get(i)` は元の要素を返します。`size()` は要素数、`empty()` は空かどうかを返します。
- $n$ 要素の構築は $O(n\log\log(n+2)+1)$ 時間・領域です。取得と区間積は $O(1)$ で、区間積に使う演算は高々2回です。

たとえば最小値の演算で `[8,3,6,1,5]` を構築すると、`prod(1,4)` は1です。

[使用例](../Examples/sqrt_tree_example.cpp) ／ [Verify](../Verify/current/submissions/sqrt_tree_static_rmq.cpp) ／ [AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/sqrt_tree)
