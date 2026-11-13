# Link Cut Tree

`LinkCutTree<T,Op>`は、辺を追加・削除する森で頂点の値とパスの和などを管理します。頂点番号は0始まりで、番号の大小に親子関係の意味はありません。構築直後は各頂点が孤立し、辺はlinkで追加します。[基本例](../Examples/link_cut_tree_example.cpp)・[非可換合成](../Examples/link_cut_tree_composite_example.cpp)・[遅延作用](../Examples/link_cut_tree_lazy_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `(values,identity=T{},op=Op{},...)` | vector/spanの各値を持つ孤立頂点を構築する。既定の演算は加算。 |
| `link(u,v)` / `cut(u,v)` | 辺を追加 / 削除してtrue。閉路・自己辺 / 不存在の辺ならfalse。 |
| `connected(u,v)` | 同じ成分ならtrue。 |
| `get(v)` / `set(v,x)` / `transform(v,fn)` | 値をコピー / xへ置換 / fn(現在値)へ置換する。 |
| `path_prod(u,v)` | uからvへ端点を含めて順に集約したoptional<T>。未接続ならnullopt。 |
| `path_size(u,v)` / `path_length(u,v)` | 頂点数（未接続は0）/ 辺数のoptional（未接続はnullopt）。 |
| `kth_on_path(u,v,k)` | u側から0始まりk番目の頂点番号。未接続・範囲外はnullopt。 |
| `make_root(v)` / `find_root(v)` | 成分の根をvへ変更 / 現在の根を返す。 |
| `lca(root,u,v)` | rootを根にしたLCA。3頂点が同じ成分でなければnullopt。 |
| `apply(v,f)` / `apply(u,v,f)` | 点 / パスへ更新を適用する。更新ありの型だけで使える。パス版は未接続なら変更せずfalse。 |
| `size()` / `empty()` / `usable()` | 頂点数 / 空か / 例外後も森操作できるか。O(1)。 |
| `reset(values)` / `clear()` / `swap(other)` | 孤立頂点へ再構築 / 領域を解放して空にする / O(1)で交換する。 |

```cpp
LinkCutTree<std::int64_t> tree(std::vector<std::int64_t>{2,4,1,7});
tree.link(0,1); tree.link(1,2); tree.link(1,3);
auto sum = tree.path_prod(2,3); // optionalに12が入る。1+4+7。
tree.cut(1,3);
auto missing = tree.path_prod(2,3); // nullopt。
```

同じ頂点へのパスは、その頂点の値・頂点数1・辺数0です。パス操作・link・cutは根の向きを変える場合があります。根が重要ならmake_rootまたはlcaの引数で明示します。取得も補助木を変更するため非constです。

## 演算とパス更新

Opはconst呼出可能な結合演算、identityは左右の単位元です。左右の順序を保ち、写像合成などの非可換な集約も扱えます。T・Actionはcopyable、設定はconst呼出可能・move_constructibleで、破棄が例外を投げない型を使います。

パス更新には`LinkCutTree<T,Op,Action,Mapping,Composition>`を指定します。Mappingは`mapping(f,集約値,頂点数)`で更新後の集約を返し、Compositionは`composition(newer,older)`でolderの後にnewerを適用する更新を返します。更新は集約へ分配でき、反転と可換であることが前提です。作用なしの型は更新用の状態を持ちません。

構築はO(N+1)時間・領域、森操作は償却O(log(N+1))、1回は最悪O(N+1)です。値・更新のコピーや演算をO(1)とします。数値は途中計算も値域内に収め、コールバックから同じ木へ再入しないでください。

## 所有権と検査

コピーは頂点・辺・遅延の領域が独立し、演算設定はconstで共有します。移動はnoexcept・O(1)、移動元は同じ設定の空の森として再利用できます。コピー代入・resetは確保やコピーの失敗時に元を保ちます。resetはO(旧N+新N+1)、clearはO(N+1)です。

森操作中に値・演算・更新等が例外を投げると、`usable()==false`になります。reset・clear・代入で復旧するまで森操作はlogic_errorを投げます。失敗状態をコピー・移動した先も復旧が必要です。演算の外部状態は巻き戻しません。

頂点番号は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。N<=INT_MAXの容量保護、接続・辺・順位による戻り値、例外後の無効化は常に維持します。番号の入力検査エラーでは無効化しません。

数学的参考は[Nyaan](https://nyaannyaan.github.io/library/lct/link-cut-base.hpp)と[Luzhiled](https://ei1333.github.io/library/structure/dynamic-tree/lazy-link-cut-tree.hpp)です。実装は独立に組み立て、内部配置と検証結果は[Audit](../Audit/)に置きます。
