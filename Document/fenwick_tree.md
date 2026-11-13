# Fenwick Tree

`fenwick_tree<T>`は、点への加算と区間和を扱います。添字は0始まり、区間は左端を含み右端を含まない[l,r)です。公開名・基本操作はACL形式で、`atcoder::`は付けません。[使用例](../Examples/fenwick_tree_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `fenwick_tree()` / `(int n)` / `(vector<T>)` | 空列 / 0がn要素 / 指定した列で構築する。O(N+1)。 |
| `add(p,x)` | 添字pの要素にxを足す。 |
| `sum(l,r)` | [l,r)の和をTの値で返す。空区間はT{}。 |
| `get(p)` / `set(p,x)` | 点の値を取得 / xへ置き換える。 |
| `lower_bound(x)` / `upper_bound(x)` | 先頭からの和がx以上 / xより大きい最初の添字を返す。なければsize()。 |
| `size()` / `empty()` | 要素数int / 空かをO(1)で返す。 |
| `values()` | 列をO(N+1)時間・追加O(N)領域で復元する。 |
| `reset(n)` / `reset()` | 0がn要素 / 現在の長さで全要素0へ戻す。 |
| `clear()` / `swap(other)` | 容量を保って空にする / 内容をO(1)で交換する。 |

```cpp
fenwick_tree<std::int64_t> sums(std::vector<std::int64_t>{2,0,3,5});
sums.add(1,4);                // [2,4,3,5]。
sums.set(3,1);                // [2,4,3,1]。
auto total = sums.sum(1,4);   // 8。
auto index = sums.lower_bound(7); // 2。先頭からの和は2,6,9,10。
```

## 境界探索と数値の前提

点操作・区間和・境界探索はO(log(N+1))、領域はO(N)です。reset(n)は旧列の破棄を含めO(旧N+新N+1)、reset()はO(N+1)。演算とコピーをO(1)とします。先頭の和はsum(0,r)、全体はsum(0,size())で取得します。

lower_boundは非正の目標値なら0、upper_boundは負の目標値なら0です。探索中は全要素が非負で、途中の和がTに収まり、加算と順序が整合することが前提です。剰余で和が周回する場合は使えません。非負条件の検査だけで全列を走査しません。

bool以外の整数型は内部を同幅の符号なし型で保持し、型幅のmod 2^bitで計算します。符号付き型でも内部の符号付きオーバーフローを起こしません。利用可能な128ビット整数拡張も同様です。modintや独自型は、T{}を0とする可換な加減算を正しく定義してください。Mintは利用側で用意します。

列長はintの範囲と内部容量に収めます。不正な添字・区間は既定で例外により検査し、`SKIP_RUNTIME_CHECKS`で除きます。容量・添字の保護は維持します。sumはACLと同じ非constです。コピーは独立、移動元は空列として再利用できます。

[ACLのAPI仕様](https://atcoder.github.io/ac-library/production/document_ja/fenwicktree.html)と[Nyaanの解説](https://nyaannyaan.github.io/library/data-structure/binary-indexed-tree.hpp)を参照しています。実装は本ライブラリ用のCC0です。動的座標には[動的Fenwick Tree](dynamic_fenwick.md)を使います。
