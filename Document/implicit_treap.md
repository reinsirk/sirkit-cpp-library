# 暗黙Treap

`DataStructure/implicit_treap.cpp`の`lazy_implicit_treap`は、添字で操作する可変長列です。要素の位置を部分木の要素数で表し、乱数の優先度で木を構成します。ノードはvectorで所有し、削除したスロットはfree listで再利用します。

## 集約と遅延作用

`op`は結合的で`e()`を単位元とし、`mapping`は集約に分配することが前提です。`composition(f,g)`はgの後にfを適用します。各演算・要素のコピーや移動がO(1)なら、通常の位置操作・区間操作・境界探索は期待O(log N)です。挿入はノード配列の拡張を含めて償却期待O(log N)で、再確保時には保持スロット数に比例する処理が発生します。最悪の木の高さはO(N)になることがあります。

非可換の集約では順方向と逆方向の結果を保持し、区間反転で交換します。`Commutative=true`を指定する場合はopの可換性が必要です。既定の`RangeAddMin`と`RangeAddSum`は自動認識され、逆方向の重複した集約を省きます。

区間の集約は左右の境界を専用に下降し、区間内の部分木の集約を結合します。移動できる値型では、一時的な集約を移動して再利用します。

反転・回転は、区間を含む最小の部分木まで共通の経路を一度たどります。その根は最大優先度なので維持でき、左右の部分木を分割・結合して並べ替えます。全区間の反転は、根に反転を適用するだけでO(1)です。

## 挿入・削除と所有

一要素の挿入は添字に沿って下降し、新しいノードの優先度が部分木の根より大きくなった位置だけを分割します。一要素の削除は対象へ直接下降し、その左右の部分木だけを結合します。`pop_front()`・`pop_back()`・昇順列の`erase_max()`もこの経路を使います。区間削除は分割・結合後に対象ノードを破棄し、期待O(削除数 + log N)です。`erase_prod`は、Sのコピー・ムーブ構築が例外を投げない型では切り離した部分木の集約を使い、一要素の区間では対象まで直接下降します。コピー・ムーブが例外を投げ得る型は、削除前に集約を取得します。

vectorからの構築はO(N)、`to_vector()`は現在の要素数に比例します。使用領域は保持するスロット数に比例し、通常は過去の最大要素数に対応します。`reserve(n)`とノード確保にはintの範囲を超える容量の保護があり、検査を省く場合も維持します。

コピーではノード群を独立に所有します。コピー代入は失敗時に代入先を保ちます。ノードのコピー構築・代入が例外を投げず、既存容量に収まる場合はその容量を再利用し、それ以外はコピーの成功後に内容を交換します。ムーブ元は空列として再利用できます。

## 先頭・末尾と区間の移動

`front()`と`back()`は先頭・末尾の値を期待O(log N)で返します。空列には使えません。

`move_range(l,r,pos)`は、半開区間[l,r)を移動前の位置posの直前へ移します。posは0からsize()までで、size()は末尾です。移動先が区間内またはその端（l<=pos<=r）の場合、あるいは空区間の場合は何もしません。移動は期待O(log N)で、要素やIDの作り直し・削除は行いません。

```cpp
lazy_implicit_treap<> a(std::vector<std::int64_t>{10, 20, 30, 40, 50});
a.move_range(1, 3, 5); // [20,30]を元の末尾へ移す。[10,40,50,20,30]
a.move_range(3, 5, 0); // 末尾の[20,30]を先頭へ移す。[20,30,10,40,50]
// a.front() == 20、a.back() == 50。
```

## IDから現在位置を求める

`indexed_implicit_treap`は、要素に不変の整数IDを付けて操作するための別名です。`lazy_implicit_treap`と同じ先頭8テンプレート引数を受け取り、第9引数`TrackIds=true`を指定します。通常版は既定でIDを追跡せず、ID索引や親リンクの更新を持ちません。

IDは値と別に管理する非負の`int`で、生存する要素間で一意にします。同じ値の要素にも別のIDを付けられます。`set`や`apply`で値が変わってもIDは変わりません。既存の`insert_at`・`push_back`・`insert_sorted`やvectorからの構築は、IDなしの要素を作ります。

| API | 動作 |
| --- | --- |
| `insert_at_with_id(pos,id,value)` | 位置posの直前へID付きの値を挿入する。`0 <= pos <= size()` |
| `insert_after(id,new_id,value)` | 存在するIDの要素の直後へ、新しいID付きの値を挿入する |
| `insert_before(id,new_id,value)` | 存在するIDの要素の直前へ、新しいID付きの値を挿入する |
| `position_of(id)` | IDの現在位置を0始まりで返す。存在しなければ-1 |
| `contains_id(id)` | IDが存在すればtrue |
| `get_id(pos)` | 位置posのIDを返す。IDなしなら-1 |
| `get_by_id(id)` / `set_by_id(id,value)` | 存在するIDの値を取得・置換する。IDは保持する |
| `erase_id(id)` | IDの要素を削除してtrue。存在しなければfalse |
| `reserve_ids(n)` | ID 0からn-1までの索引を準備する。既存の登録は維持する |
| `erase_prod(l,r)` | 半開区間[l,r)の集約を返し、その区間を削除する。通常版でも利用可 |
| `prod_between(a,b)` | 2つのIDの間を、端点を含めずに列の順序で集約する |
| `erase_between(a,b)` | 2つのIDの間を集約して削除する。端点は残す |

```cpp
using IdSumTreap = indexed_implicit_treap<
    RangeAddSum::S, RangeAddSum::op, RangeAddSum::e, RangeAddSum::F,
    RangeAddSum::mapping, RangeAddSum::composition, RangeAddSum::id>;

IdSumTreap a;
a.insert_at_with_id(0, 0, {0, 1});
a.insert_after(0, 1, {1, 1}); // [0,1]
a.insert_after(1, 2, {2, 1}); // [0,1,2]
a.insert_after(0, 3, {3, 1}); // [0,3,1,2]
int l = a.position_of(3);    // 1
int r = a.position_of(2);    // 3
auto removed = a.erase_prod(l + 1, r); // 端点を除く[2,3)を削除。
// removed.sum == 1、残る列は[0,3,2]。
// a.position_of(1) == -1、a.position_of(2) == 2。

a.insert_before(2, 4, {4, 1}); // [0,3,4,2]。
a.set_by_id(4, {40, 1});      // [0,3,40,2]。ID=4は変わらない。
auto value = a.get_by_id(4); // value.sum == 40。
auto between = a.prod_between(2, 0); // between.sum == 43。
auto removed_again = a.erase_between(0, 2); // removed_again.sum == 43、列は[0,2]。
bool erased = a.erase_id(4); // false。すでに削除されている。
```

`prod_between`・`erase_between`の端点IDは前後どちらの順でも渡せます。非可換の集約でも、実際の列の左から右へ集約します。同じIDや隣接する2要素を指定した場合は単位元を返し、列を変えません。両方のIDが存在することが前提です。

`position_of`・`get_id`は期待O(log N)、`contains_id`はO(1)です。挿入はノード配列の拡張を含めて償却期待O(log N)で、ID索引の拡張費用は別途かかります。`reverse`・`rotate`による並べ替えにも追従します。位置特定では親を根までたどり、祖先の遅延反転・作用を伝えてから左右関係を読みます。値の大小から位置を探す処理は使いません。

`get_by_id`・`set_by_id`・`erase_id`・`prod_between`は期待O(log N)、`erase_between`は期待O(削除数+log N)です。ABC421 Fのように端点を指定する問題では、位置を手動で並べ替えずに`erase_between(x,y).sum`で処理できます。

ID索引にはvectorを使うため、領域は保持ノード数に加えて、準備済みID枠数Mに比例します。`reserve_ids(n)`や新しい最大IDの挿入は、索引の追加枠の初期化と、再確保時の既存枠の移動を伴います。大きな飛び飛びのIDは利用側で圧縮してください。`reserve(n)`はノードだけを予約します。

削除したIDは再利用でき、ノードのスロット再利用でも以前のIDは復活しません。`clear`はID登録も消して容量を保持します。コピーは独立したID索引を持ち、O(ノード枠数+M)です。ムーブ・swapはID登録を列と共に移します。IDはそれぞれの木で管理し、コピー先とコピー元で同じIDを使えます。

`erase_prod`は期待O(削除数+log N)です。[ABC421 F - Erase between X and Y](https://atcoder.jp/contests/abc421/tasks/abc421_f)では値をIDとして使えます。各要素は一度しか削除されないため、全クエリで期待O(Q log Q)時間、O(Q)領域です。[提出形式の使用例](../Examples/implicit_treap_erase_between_example.cpp)では、IDから位置を求め、端点を除く区間を削除します。

## 入力の前提と検証

添字は0始まり、区間は半開区間です。既定では添字・区間などを低コストで検査します。`erase(l,r)`の不正区間は検査有効時に何もしません。`SKIP_RUNTIME_CHECKS`を定義するとこれらの入力検査を除き、正しい添字・区間が前提になります。

`erase_prod`・`move_range`の不正区間や移動先、`get_id`の不正位置、空列の`front`・`back`、存在するIDが必要な操作への不存在IDは検査時に`out_of_range`、負の挿入ID・重複IDは`invalid_argument`です。`position_of`・`contains_id`・`erase_id`は検査省略時も不存在を-1・false・falseとして扱います。ID索引の内部容量保護は検査省略時も維持します。

昇順用APIは列全体が昇順であること、境界探索は単位元で述語が真で区間伸長に単調であることが必要です。全列を走査してこれらの前提を検査することはしません。

使用例は[implicit_treap_example.cpp](../Examples/implicit_treap_example.cpp)です。提出用コードは次の3本です。

- [Dynamic Sequence Range Affine Range Sum](../Verify/current/submissions/implicit_treap_affine.cpp)：挿入削除・区間反転・アフィン作用・和。
- [Deque Operate All Composite](../Verify/current/submissions/implicit_treap_deque.cpp)：両端の追加削除・非可換な関数合成・全体集約。
- [Erase between X and Y](../Verify/current/submissions/implicit_treap_erase_between.cpp)：IDによる位置特定・直後への挿入・区間和と削除。オンラインの判定状況はVerifyのmanifestで確認してください。

[独立補足試験](../Verify/current/tests/implicit_treap_boundary_test.cpp)では、順序に敏感な境界探索の両形式、作用合成・反転・回転、比較順と等値判定の差、端点削除後のスロット再利用などを照合します。例外・コピー時の容量再利用は専用試験で補います。測定と検査の記録は`Audit/`を参照してください。

[IDの独立補足試験](../Verify/current/tests/implicit_treap_ids_test.cpp)では、vector参照と位置・値を照合し、遅延作用・反転・回転、ID付きと匿名の混在、削除後の再利用、コピー・ムーブ・swapを確認します。

[ID索引の確保失敗試験](../Verify/current/tests/implicit_treap_ids_allocation_test.cpp)では、ノード容量を再利用するコピーと再確保するコピー、挿入時のID索引・ノードの確保失敗後に、要素とID登録が保持されることを確認します。

[便利な操作の補足試験](../Verify/current/tests/implicit_treap_convenience_test.cpp)では、区間移動の全境界とvector参照、ID指定の操作、端点の前後・同一ID・隣接要素、非可換な集約を確認します。
