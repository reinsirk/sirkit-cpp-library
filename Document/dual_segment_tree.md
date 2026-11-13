# 双対セグメント木

`DualSegmentTree<T,Action,Mapping,Composition>`は、区間をまとめて更新し、1要素の値を取得します。区間和などの集約が不要な場合に使います。添字は0始まり、区間は半開区間[l,r)です。[使用例](../Examples/dual_segment_tree_example.cpp)を参照してください。

Mappingは`mapping(f,x)`で更新後の値、Compositionは`composition(newer,older)`で「olderの後にnewer」を実行する更新を返す関数オブジェクトです。更新の合成は結合的で、Mappingと整合するように定義します。更新の順序は保持し、Actionの単位元・比較・既定構築は要求しません。

## API

| API | 動作 |
| --- | --- |
| `(values,mapping,composition)` | vector/spanの列で構築する。 |
| `(n,value,mapping,composition)` | 同じ初期値をn個並べて構築する。 |
| `apply(l,r,f)` / `apply(p,f)` | 区間 / 1要素へ更新fを適用する。空区間は何もしない。 |
| `get(p)` / `operator[](p)` | 現在の値をコピーして返す。 |
| `set(p,x)` / `transform(p,fn)` | 点をx / fn(現在値)へ置き換える。 |
| `values()` | 全要素をvectorで返す。O(N+1)。 |
| `size()` / `empty()` / `usable()` | 要素数 / 空か / 例外後も操作可能か。O(1)。 |
| `reset(values)` / `reset(n,value)` | 同じ設定で列を作り直す。O(旧N+新N+1)。 |
| `clear()` / `swap(other)` | 空にして配列領域を解放 / 内容をO(1)で交換する。 |

```cpp
struct Add {
    std::int64_t operator()(std::int64_t f,std::int64_t x) const { return x+f; }
};
DualSegmentTree<std::int64_t,std::int64_t,Add,std::plus<std::int64_t>> tree(
    std::vector<std::int64_t>{1,2,3},Add{},std::plus<std::int64_t>{});
tree.apply(0,2,4); // [5,6,3]。
auto value = tree.get(1); // 6。
```

## 前提・所有権・例外

演算・コピーをO(1)として、構築はO(N+1)時間・領域、更新・点操作はO(log(N+1))です。取得も保留中の更新を伝えるため非constです。T・Actionはコピー可能で、設定はconst呼出可能・move_constructibleにします。数値演算は途中計算も値域内に収め、コールバックから同じ木へ再入しないでください。

コピーの値・更新は独立し、演算設定はconstで共有します。移動はnoexcept・O(1)、移動元は同じ設定の空列として再利用できます。コピー代入・resetは確保やコピーの失敗時に元を保ちます。

操作中に値・更新・写像等が例外を投げると、`usable()==false`になります。以後はreset・clear・代入で復旧するまで操作がlogic_errorを投げます。値と更新には失敗後も安全に破棄できる型を使います。入力検査のエラーでは無効化せず、演算の外部状態は巻き戻しません。

点・区間の検査は既定で有効です。`SKIP_RUNTIME_CHECKS`は入力検査だけを除き、容量・添字の保護と例外後の無効化は維持します。

数学的構成と問題選定は[Luzhiledの双対セグメント木](https://ei1333.github.io/library/structure/segment-tree/dual-segment-tree.hpp.html)を参考にし、実装は独立に組み立てています。
