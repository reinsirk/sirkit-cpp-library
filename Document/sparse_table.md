# Sparse Table

`SparseTable<T,Op>`は、変更しない列の区間の最小値・最大値・gcdなどをO(1)で求めます。区間は0始まりの半開区間[l,r)です。[使用例](../Examples/sparse_table_example.cpp)を参照してください。

Opには結合的な演算を指定し、同じ値を重ねても変わらない`op(x,x)==x`が必要です。これをべき等性と呼びます。和はこの条件を満たさないため、[Disjoint Sparse Table](disjoint_sparse_table.md)を使います。左右の交換は要求せず、単位元identityは左右どちらに混ぜても結果を変えない値を指定します。

## API

| API | 動作 |
| --- | --- |
| `(values,identity=T{},op=Op{})` | vectorから構築する。既定構築は空列。 |
| `get(i)` | 元の要素のconst参照を返す。T=boolでは値。 |
| `prod(l,r)` / `all_prod()` | [l,r) / 全体を集約した値を返す。空はidentity。O(1)。 |
| `max_right(l,pred)` / `min_left(r,pred)` | 条件を満たす最大右端 / 最小左端を返す。O(log(N+1))。 |
| `size()` / `empty()` / `values()` | 要素数 / 空か / 元の列のコピー。列のコピーはO(N+1)。 |
| `assign(values)` / `clear()` / `swap(other)` | 再構築 / 空にする / 内容をO(1)で交換する。点更新は行わない。 |

```cpp
struct Minimum { int operator()(int a,int b) const { return std::min(a,b); } };
SparseTable<int,Minimum> table(
    std::vector<int>{4,2,7,1,8},std::numeric_limits<int>::max());
auto minimum = table.prod(1,4); // 1。
```

## 前提と計算量

演算・コピーをO(1)として、構築はO(N log(N+1)+1)時間・領域です。再構築は旧領域の破棄も含めO(旧N log(旧N+1)+新N log(新N+1)+1)です。

境界探索の述語は同じ値に同じ結果を返し、pred(identity)がtrueで、区間を伸ばしてfalseになった後はtrueへ戻らないことが前提です。単調性を確認する全要素走査は行いません。

Opはconst呼出可能な型を使います。添字・区間・述語の単位元条件は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。参照は再構築・clear・移動・交換後まで保持しないでください。コピーは独立、移動元は空として再利用できます。演算等の例外時に内容の強い保証は行いません。

32bit以下の整数型（bool以外）では、条件が合えば対応するGCC／Clang・CPUでAVX2を使って構築します。`SIRKIT_DISABLE_SIMD`で専用経路とCPU判定を除外できます。非対応環境やその他の値型は標準C++の経路で処理します。

数学的参考は[AtCoder Algorithm Lectures](https://info.atcoder.jp/entry/algorithm_lectures/sparse_table)です。実装は本ライブラリ用のCC0です。
