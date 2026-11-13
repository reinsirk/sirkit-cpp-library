# Segment Tree Beats

`SegmentTreeBeats`は、整数列への区間chmin・chmax・加算と、区間和・最小値・最大値を扱います。chminは各要素を上限以下へ、chmaxは下限以上へそろえる操作です。添字は0始まり、区間は半開区間[l,r)です。[使用例](../Examples/segment_tree_beats_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `SegmentTreeBeats()` / `(vector<int64_t>)` | 空列 / 指定した列で構築する。 |
| `chmin(l,r,x)` / `chmax(l,r,x)` | 各要素をmin(現在値,x) / max(現在値,x)へ更新する。 |
| `add(l,r,x)` | 区間の各要素へxを足す。 |
| `set(i,x)` / `get(i)` | 点代入 / 点の値を返す。 |
| `sum(l,r)` | 区間和をint64_tで返す。空区間は0。 |
| `min(l,r)` / `max(l,r)` | 最小値 / 最大値をoptional<int64_t>で返す。空区間はnullopt。 |
| `values()` | 全要素をvectorにコピーする。O(N+1)。 |
| `size()` / `empty()` / `reset(values)` | 要素数 / 空か / 再構築する。 |
| `clear()` / `swap(other)` | 空にする / O(1)で交換する。 |

```cpp
SegmentTreeBeats tree(std::vector<std::int64_t>{1,5,-2,8});
tree.chmin(0,4,4); // [1,4,-2,4]。
tree.chmax(1,3,0); // [1,4,0,4]。
tree.add(0,2,3);   // [4,7,0,4]。
auto sum = tree.sum(0,4); // 15。
```

## 前提と計算量

最大値・2番目の最大値・個数などを持ち、影響する要素だけをまとめて更新できる区間では子へ降りません。構築はO(N+1)時間・領域。N要素にQ回の範囲更新を行う全体の償却計算量はO((N+Q) log²(N+1))、1回の範囲更新は最悪O(N+1)です。set・get・sum・min・maxはO(log(N+1))です。

要素にはint64_tの全値域を使えます。add後の実際の要素が範囲外ならoverflow_errorを投げて列を保ちます。内部の遅延配置は変わる場合があります。sumの戻り値が範囲外の場合もoverflow_errorです。和だけが大きい列は保持・更新でき、極値や別区間を取得できます。空区間の更新は何もしません。

内部の和と遅延はGCC/Clangの128ビット整数拡張、または標準C++の2ワード演算を使います。`SIRKIT_FORCE_STANDARD_ARITHMETIC`で標準経路へ切り替えます。標準経路はsize_tが64ビット以下の環境が対象です。中間の子の値だけが64ビット範囲外でも扱えるよう計算し、容量・桁あふれの保護は検査省略時も維持します。数値設計の詳細は[Audit](../Audit/)に置きます。

取得も保留中の更新を子へ伝えるため非constです。更新・区間取得は動的確保を行わず、valuesは戻す配列を確保します。reset・コピー代入は失敗時に元を保ち、コピーは独立、移動元は空として再利用できます。resetはO(旧N+新N+1)、clearはO(N+1)で、確保容量を保持する場合があります。

添字・区間は既定で検査し、`SKIP_RUNTIME_CHECKS`で除きます。構成と問題選定は[NyaanのSegment Tree Beats](https://nyaannyaan.github.io/library/segment-tree/segment-tree-beats.hpp)を参考にし、実装は独立に組み立てています。
