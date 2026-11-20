# PersistentUnionFindのroot・版・issame（2026-11-16）

通常UnionFindと同様、`root(i)`は代表頂点の番号を`int`で返す。保存用の版のハンドルは`version()`で取得し、型は`Version`とした。
`issame(a,b)`と`issame(a,b,version)`も追加し、既存の`same`は維持した。

```cpp
PersistentUnionFind uf(4);
auto initial = uf.version();
uf.unite(0, 1);
int representative = uf.root(1);
bool current = uf.issame(0, 1);          // true
bool previous = uf.issame(0, 1, initial); // false
```

旧`get_root()`は`version()`、`root_of(i)`は`root(i)`へ移行。UFの公開`root`ポインタは非公開状態へ移した。
`Version`は領域を所有しない版のハンドルで、実体は`Node*`。領域の所有は従来どおりshared_ptrとvectorで管理し、PersistentArrayの公開rootは変更していない。
型と寿命の説明・全APIは[Document](../../Document/persistent_union_find.md)、[完全な使用例](../../Examples/persistent_union_find_example.cpp)を参照。

## Verifyの役割と提出ファイル

| 提出ファイル | 問題 | 検証する内容 | 現在の状態 |
|---|---|---|---|
| [persistent_union_find.cpp](../current/submissions/persistent_union_find.cpp) | [LC Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) | 任意の過去版からの分岐・版の保存・issame | 最適化版[AC・96ms](https://judge.yosupo.jp/submission/417443)・現行本文一致 |
| [persistent_union_find_union_path_query.cpp](../current/submissions/persistent_union_find_union_path_query.cpp) | [yukicoder No.2295](https://yukicoder.me/problems/no/2295) | 版別連結の二分探索・root/sizeによる成分距離和・オンラインX | 最適化版[AC・1212ms](https://yukicoder.me/submissions/1199606)・現行本文一致 |

2295では辺重みが非減少のため、2頂点が初めて連結する版の重みが最小最大辺距離になる。
重みwで大きさA/Bの成分を結ぶと、新たなA*B個の頂点対の距離がwとなるため成分和を更新できる。
総和は各乗算の間で法998244353を取り、w*A*Bがint64を超える場合にも対応する。
この問題は時系列の版のみで、任意の保存版からの分岐はLCで補完する。重み付きUFのポテンシャル差の検証ではない。
時間は種別1/3がO(log²(N+1))、種別2がO(log³(N+1))、種別4がO(1)。領域はO(Nlog(N+1))ノード。

## 検証（API変更時点）

- 新APIと既存モデル：40ビルド・32実行、8構成すべて成功。返り値型、過去版、復元、copy/move、共有寿命、確保失敗も確認。
- 使用例：GCC/Clangの検査あり/なし4構成で成功。
- 2295：公式3例＋独立Floyd minimax参照84例を8構成で照合し、696実行成功。
- 最大入力：N=200000,Q=300000、深い平衡結合、1e19となる最終結合、約17万回の二分探索を含む4パターンを3構成で確認。最大約0.28秒・約229MiB（ローカル、オンラインACではない）。
- 派生版：実際に生成した14条件の入力例・使用例照合に成功。通常284本、派生1030本を同期。
- Python検査497件成功（意図した除外2件）。Codex後にFableも両ソースを確認し、追加修正不要と判断。

[最終監査](../../Audit/Changes/persistent-uf-api-final-2026-11-16.json)にSHA・採否・実行記録を保存。旧AC417250は新ソースへ引き継がず、新API版は417442でACを確認した。コミットなし。
[先の29提出](submissions29-results-2026-11-16.md)は引き続きすべて現行本文一致AC。

## 再提出結果（2026-11-16）

LCは公式API、yukicoderは公開判定とraw sourceを取得し、提出本文を照合した。生成冒頭の提出注記と改行形式のみを正規化し、実装・main・入出力は一致している。
LCの[Fastest](https://judge.yosupo.jp/submission/409831)は15msで、今回の91msとの差は76ms、約6.07倍。別提出の測定値なので、本体の変更効果を示す比較ではない。
yukicoderは全56ケースAC、メモリ225408KB。

先に提示された[417441](https://judge.yosupo.jp/submission/417441)は永続Segment TreeのPoint Add Range Sumで、こちらも現行本文一致・AC447msとして別に反映した。
取得記録とソースのSHAは[再提出監査](../../Audit/Submissions/persistent-uf-resubmissions-2026-11-16.json)を参照。

## その後の最適化

同日の[速度差調査・最適化](persistent-uf-optimization-2026-11-16.md)で本体を変更し、417443・1199606で現行本文一致のACを確認した。417442・1199601のACは最適化前ソースの記録として保持する。
