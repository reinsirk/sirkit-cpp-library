# DataStructure文書の詳細記録

公開説明を簡潔にする前の文書です。内部処理・測定・試験の詳細はこの記録と既存のAuditにあります。現在のAPI契約はDocumentとソースを優先してください。

## union-find

元文書: `Document/union_find.md`

# Union Find

頂点を互いに重ならない連結成分に分け、成分の結合と同一成分の照会を行います。[使用例](../../Examples/union_find_example.cpp)を参照してください。

各頂点に親の添字を持ち、根には成分サイズの負数を保存します。小さい成分を大きい成分へ接続するため、木の高さはO(log(n+1))以下です。代表探索では1つおきの頂点を祖父へつなぐpath halvingで経路を短くし、繰り返す操作の計算量を抑えます。

GCCの結合処理は、代表探索で読み出した根の負サイズを保持して再利用します。2本目の探索で経路を短くしても、最初の根とサイズは変わりません。第二引数がその根自身、または探索中の親がその根であれば、それ以上の書戻しは必要ないため結合不要として終了します。それ以外はpath halvingで探索し、最後にも同じ根かを確認します。同じ根のサイズを加算することはありません。Clangなどでは代表探索後に根のサイズを読み出します。

## API

| API | 動作 |
| --- | --- |
| `UnionFind(n=0)` | `n`頂点を別々の成分として構築する。`n<=INT_MAX`。 |
| `root(i)`、`leader(i)` | 頂点の代表を返す。 |
| `merge(a,b)` | 成分を結合し、新しく結合したときだけ`true`を返す。 |
| `unite(a,b)` | 同じ結合を行う。戻り値は`void`。 |
| `same(a,b)`、`issame(a,b)` | 同じ成分かを返す。 |
| `size(i)` | 指定頂点の成分サイズ。 |
| `size()`、`empty()`、`component_count()` | 全頂点数、空か、成分数。 |
| `groups()` | 成分ごとの頂点一覧。各一覧の頂点は昇順。 |
| `reset(n)`、`reset()` | 新しい`n`頂点、または現在の頂点数で全成分を独立にする。容量は再利用する。 |
| `clear()` | 空に戻し、確保済み容量を保持する。`noexcept`。 |
| `swap(other)`、`swap(left,right)` | 内容を交換する。`noexcept`。 |

頂点は`0<=i<size()`です。代表はその成分の最小頂点とは限りません。結合の順序に依存し、サイズが同じ成分同士では最初の引数が属する成分の代表を残します。`groups`の成分順は代表番号の昇順です。

```cpp
UnionFind uf(5);
uf.unite(0, 1);
assert(uf.merge(1, 2));
assert(!uf.merge(0, 2));
assert(uf.same(0, 2) && uf.size(1) == 3);
assert(uf.component_count() == 3);
auto copy = uf;
copy.merge(3, 4);
assert(!uf.same(3, 4));
uf.reset(3);
assert(uf.component_count() == 3 && !uf.same(0, 1));
uf.merge(0, 1);
uf.reset(); // 頂点数3のまま結合を消す。
assert(uf.component_count() == 3);
uf.clear();
assert(uf.empty());
swap(uf, copy);
assert(copy.empty() && uf.same(3, 4));
```

## 経路圧縮と計算量

代表を使う操作は内部の親のつながりを短くする場合があります。連結成分・代表・サイズは変わりません。`groups()`も内部の探索を行う場合がありますが、返した一覧は独立した値です。後からUnion Findを変更しても、既に返した一覧は変わりません。

頂点数を`n`、確保済み容量を`C`とします。構築とコピー構築はO(n+1)時間・領域、保持領域はO(C+1)です。`root`、`leader`、`merge`、`unite`、`same`、`issame`、`size(i)`は償却O(α(n+1))、単独の操作ではO(log(n+1))の上界です。αは逆Ackermann関数です。

`groups`は償却O(1+n α(n+1))時間、O(n+1)追加領域です。全頂点が別成分なら一覧を直接作り、全頂点が1成分なら代表を一度求め、親配列の連続書込みで全頂点をその直下へつなぎます。根の負サイズを保存・復元し、連番生成で結果を作ります。この2場合はO(n+1)時間です。その他は根に保持した負サイズから各成分の容量を確保し、代表から出力位置への作業配列1本で頂点を並べます。Clangでは代表・件数の2配列を使い、件数配列を出力位置へ再利用します。いずれも成分は代表頂点の昇順、成分内は頂点の昇順です。

`size()`、`empty`、`component_count`、`swap`とムーブ構築はO(1)です。`reset(m)`は旧長`n`も含めO(n+m+1)、`clear`とムーブ代入は旧状態の破棄を含めO(n+1)の上界です。コピー代入は旧長・新長を合わせた線形時間の上界です。

## 所有権と例外

親配列は`vector<int>`で所有します。コピーは独立し、一方の結合はもう一方へ影響しません。ムーブ後の移動元は空です。自己代入・自己ムーブでは内容を保ちます。

`reset`は先に必要な容量を確保し、成功後に内容を初期化します。確保に失敗しても、既存の連結状態と成分数は保たれます。`clear`は領域を解放せず、後の`reset`で再利用できます。コピー代入も、確保失敗時には代入先の連結状態を保ちます。

`groups`が結果の確保に失敗した場合も連結成分・代表・サイズは保たれますが、途中まで行った経路圧縮は残る場合があります。内部の親配列が一切変更されないという保証ではありません。

## 入力検査

既定では指定した頂点を検査し、範囲外なら`out_of_range`を送出します。検査のためだけに全頂点を走査しません。`SKIP_RUNTIME_CHECKS`を定義した場合は、利用側で頂点の前提を満たしてください。

頂点数が`int`で表せる上限を超える場合の`length_error`は、検査省略時も維持します。配列容量の確保にも標準コンテナの上限が適用されます。

[代表・サイズの独立モデル](../../Verify/current/tests/uf_merge_reference_test.cpp)でサイズ同値時の代表も照合します。[深い経路への失敗結合の試験](../../Verify/current/tests/uf_failed_merge_reference_test.cpp)では、先に照会で圧縮せず、同頂点・根自身・根直下を含む両方向の結合と`unite`を確認します。

Reputelessの省メモリ版から、負サイズを持つ1配列とサイズによる併合を引き継いでいます。[来歴とライセンスの監査](../Provenance/provenance-union-find-second-pass-2026-10-22.json)に原典とローカル改修の区別を記録しています。


## weighted-union-find

元文書: `Document/weighted_union_find.md`

# 重み付きUnion Find

頂点間の差を持つ連結成分を管理し、成分の結合・差の照会・制約の整合判定を行います。[使用例](../../Examples/union_find_w_example.cpp)を参照してください。

`merge(a,b,w)`の向きは`potential(b)-potential(a)=w`です。親の添字を持つ配列には根のサイズを負数で保存し、別の配列に親から各頂点への差を持ちます。小さい成分を大きい成分へ接続し、代表探索時には重みを累積して経路を短くします。

## API

| API | 動作 |
| --- | --- |
| `UnionFindW<T>(n=0)` | `n`頂点を別々の成分として構築する。`n<=INT_MAX`。 |
| `root(i)`、`leader(i)` | 代表頂点を返す。 |
| `same(a,b)`、`issame(a,b)` | 同じ成分かを返す。 |
| `size(i)` | 指定頂点の成分サイズ。 |
| `weight(i)` | 代表を0としたときの頂点`i`の重み。 |
| `diff(a,b)` | `potential(b)-potential(a)`を返す。同じ成分であること。 |
| `try_diff(a,b)` | 同じ差を`optional<T>`で返し、別成分なら`nullopt`。 |
| `merge(a,b,w)` | 未接続なら結合して`true`。既接続なら整合性にかかわらず`false`。 |
| `unite(a,b,w)` | 同じ結合を行う。戻り値は`void`、既接続の矛盾は無視する。 |
| `constrain(a,b,w)` | 未接続なら結合して`true`、既接続なら差が`w`と一致するとき`true`。 |
| `size()`、`empty()`、`component_count()` | 全頂点数、空か、成分数。 |
| `groups()` | 根の番号順の成分一覧。各一覧の頂点は昇順。 |
| `reset(n)`、`reset()` | 新しい`n`頂点、または現在の頂点数で全成分を独立にする。 |
| `clear()` | 空へ戻し、確保済み容量を保持する。`noexcept`。 |
| `swap(other)`、`swap(left,right)` | 重みと内容を交換する。`noexcept`。 |

頂点は`0<=i<size()`です。代表は成分の最小番号とは限りません。同じサイズの成分を結合するときは、最初の引数が属する成分の代表を残します。矛盾した制約を`constrain`へ渡しても、成分へ矛盾フラグを残しません。

```cpp
using UF = UnionFindW<std::int64_t>;
UF uf(5);
uf.unite(0, 1, 10);
assert(uf.merge(1, 2, -3));
assert(!uf.merge(0, 2, 99)); // 既接続なので追加しない。
assert(uf.constrain(0, 2, 7));
assert(!uf.constrain(0, 2, 8));
assert(uf.diff(0, 2) == 7 && !uf.try_diff(0, 3));
auto copy = uf;
copy.unite(3, 4, 2);
assert(!uf.same(3, 4));
uf.reset(3);
assert(uf.component_count() == 3 && !uf.same(0, 1));
uf.merge(0, 1, 4);
uf.reset(); // 頂点数3を維持して差と結合を消す。
assert(uf.component_count() == 3 && uf.weight(1) == 0);
uf.clear();
assert(uf.empty());
swap(uf, copy);
assert(copy.empty() && uf.diff(3, 4) == 2);
```

## 重みの型と演算範囲

`T{}`が零になる、コピー・ムーブ可能な可換加法群を指定します。`+`、`-`、単項`-`、`+=`と、整合判定に使う`==`が必要です。利用側で用意したmodintも使えます。浮動小数点に対する許容誤差の比較はありません。

符号付き整数では、保存する差と計算途中の演算結果が表現範囲内に収まることが前提です。根からの重みの累積、根同士を結ぶ差の補正、サイズ併合で向きを反転する符号反転、差の照会もこの前提に含まれます。最終的な頂点間の差だけが収まればよいという契約ではありません。範囲外を検出して例外にする機能や、広い累積値で相殺を保つ機能はありません。`SKIP_RUNTIME_CHECKS`の有無でこの前提は変わりません。

符号なし整数は型の幅の剰余として扱います。小さい符号なし型では加減算の整数昇格に注意が必要で、整合判定では差を`T`へ戻して比較します。たとえば`uint8_t`で根からの差が250と5なら、差は型の幅で11になります。

重みを更新する通常操作では、`T`の算術・複合代入・代入が例外を送出しないことを前提とします。値を変更してから例外を送出する独自型に対し、失敗後の連結状態の維持は保証しません。構築・コピー・`reset`の確保や値の構築に関する保証は、後述の所有権の説明を参照してください。

## 計算量と所有権

以下は`T`の演算・コピー・破棄をO(1)として数えます。頂点数を`n`、親と重みの確保済み容量の合計を`C`とすると、構築とコピー構築はO(n+1)時間・領域、保持領域はO(C+1)です。

1頂点の代表探索と、それを定数回行う操作は償却O(α(n+1))で、単独の操作ではO(log(n+1))の上界です。αは逆Ackermann関数です。代表を返すだけの照会や`groups()`でも、重みを保ちながら経路を短くする場合があります。連結関係・代表・頂点間の差は変わりません。

GCCの一般照会では、根までの経路を固定長の整数配列に記録してから、根側から順に圧縮します。サイズ併合による深さの上限を使うため、経路配列の容量は`numeric_limits<int>::digits`で足ります。重みは従来の再帰の戻りと同じ順・同じ左右の値で加算し、符号付き整数の途中結果に関する前提を変えません。Clangなどでは従来の再帰探索を使います。成分列挙はGCCでも従来の探索を使い、未圧縮の木を順番に列挙するときの費用を抑えます。

`groups`は償却O(1+n α(n+1))時間、O(n+1)追加領域で、返した一覧は独立した値です。すべて独立なら一覧を直接作り、複数頂点からなる1成分なら各頂点の代表探索を行ってから番号を並べます。その他の場合は、根に保存した負サイズから各成分の容量を予約し、代表番号から出力位置への作業配列1本で頂点を並べます。代表・件数の2配列を使う場合に比べ、作業領域を`sizeof(int)*n`バイト減らします。頂点の代表探索と重みの圧縮は昇順に行います。GCCでは一成分の走査を独立した関数に置きます。`size()`、`empty`、`component_count`、`swap`とムーブ構築はO(1)です。`reset(m)`とコピー代入は旧長`n`も含めO(n+m+1)の上界です。非送出コピー・破棄の型では既存容量を再利用し、十分な容量があれば内部配列を新たに確保しません。`T`のコピー代入も非送出なら、`reset`は重みの既存要素だけを0へ代入し、拡大で増える要素は0から一度だけ構築します。代入が例外を送出し得る型は、破棄してコピー構築し直します。`T`自体の構築・コピーによる確保は、その型に依存します。それ以外の型は新旧両方の領域を使います。`clear`とムーブ代入は旧状態の破棄を含めO(n+1)の上界です。

親と重みは標準コンテナで所有します。コピーは独立し、一方の結合は他方へ影響しません。ムーブ後の移動元は空です。自己代入・自己ムーブでは内容を保ちます。

コピー構築と破棄がともに非送出の型では、コピー代入と`reset`は必要な容量を先に予約してから置き換えます。`reset`に使う零の構築も、既存状態を消す前に行います。それ以外の型は従来どおり新しい状態を構築してから交換します。どちらも、確保や値のコピー・構築に失敗しても、連結状態・成分数・差を保ちます。ただし予約済み容量は変わる場合があります。非送出性の判定には標準の型特性を使い、独自型の単なる実行実績からは推測しません。

`groups`の結果確保に失敗した場合、途中まで行った経路圧縮が残ることがあります。非送出の値演算という前提の下で、成分・代表・差は保たれますが、内部の親配列が一切変わらないとは保証しません。

## SIMDと標準経路

x86のGCC/Clangでは、256頂点以上かつCPUがAVX2に対応するとき、親配列の連続8要素の符号ビットから根の位置を列挙します。根の負サイズによる予約だけをSIMD化し、重みの演算や圧縮順は変えません。通常のコンパイル設定と、既にAVX2が有効な設定の両方で使用できます。

`SIRKIT_DISABLE_SIMD`を定義すると、追加ヘッダー・AVX2専用関数・CPU判定を除外し、標準C++の走査を使います。コンパイラ・CPUが対応しない場合と小規模入力も同じ標準経路です。

## 入力検査

既定では指定した頂点を検査し、範囲外なら`out_of_range`を送出します。別成分に対する`diff`は`invalid_argument`です。`SKIP_RUNTIME_CHECKS`を定義した場合は、利用側でこれらの前提を満たしてください。

頂点数が`int`で表せる上限を超える場合の`length_error`は、検査省略時も維持します。検査のためだけに全頂点や全辺を走査する処理はありません。

[未圧縮の任意頂点順の列挙](../../Verify/current/tests/wuf_groups_reference_test.cpp)、[確保失敗時の状態](../../Verify/current/tests/wuf_groups_allocation_test.cpp)、[重みの加算順と整数境界](../../Verify/current/tests/wuf_order_reference_test.cpp)を独立モデルで照合します。

Reputelessの省メモリ版から、負サイズの親配列と親からの差を累積する方式を引き継いでいます。[来歴とライセンスの監査](../Provenance/provenance-weighted-union-find-second-pass-2026-10-22.json)に、原典・履歴・ローカル改修を区別して記録しています。


## rollback-union-find

元文書: `Document/rollback_union_find.md`

# 巻き戻せるUnion Find

結合操作の履歴を逆順に戻せるUnion Findです。保存した位置まで戻って別の結合を試す探索や、オフライン動的連結性で使えます。[使用例](../../Examples/rollback_union_find_example.cpp)を参照してください。

各頂点に親、根には成分サイズの負数を持ちます。小さい成分を大きい成分へ接続し、経路圧縮は行いません。変更情報を履歴へ残すため、`undo`は直前の`unite`呼出しを1回戻せます。公開`undo()`では、既に同じ成分だった履歴に対する頂点配列の書き戻しを省きます。

## API

| API | 動作 |
| --- | --- |
| `RollbackUnionFind(n=0)` | `n`頂点を別々の成分として構築する。`n<=INT_MAX`。 |
| `find(i)`、`root(i)`、`leader(i)` | 代表頂点を返す。 |
| `same(a,b)`、`issame(a,b)` | 同じ成分かを返す。 |
| `size(i)` | 指定頂点の成分サイズ。 |
| `unite(a,b)`、`merge(a,b)` | 成分を結合し、新しく結合したときだけ`true`を返す。 |
| `undo()` | 直前の結合呼出しを1回戻す。履歴が空でないこと。 |
| `get_state()` | 現在残っている結合呼出し数を`int`で返す。 |
| `snapshot()` | 現在の履歴位置を1個だけ保存する。 |
| `rollback(state=-1)` | 指定位置まで戻す。`-1`なら保存した位置まで戻す。 |
| `clear_history()` | 現在の連結状態を保ち、履歴と保存位置を0にする。 |
| `reserve_history(k)` | `k`回分の履歴容量を確保する。 |
| `size()`、`empty()`、`component_count()` | 全頂点数、空か、成分数。 |
| `groups()` | 成分ごとの頂点一覧。各一覧の頂点は昇順。 |
| `reset(n)`、`reset()` | 指定した頂点数、または現在の頂点数で全成分を独立にし、履歴と保存位置を消す。 |
| `clear()` | 頂点と履歴を空にし、確保済み容量を保持する。 |
| `swap(other)`、`swap(a,b)` | 履歴と保存位置を含めて内容を交換する。 |

頂点は`0<=i<n`、指定位置は`0<=state<=get_state()`です。履歴の回数も`INT_MAX`以下です。既に同じ成分だった場合も`unite`は履歴を1回進め、`undo`の対象になります。

```cpp
RollbackUnionFind uf(4);
const auto state = uf.get_state();
assert(uf.unite(0, 1));
assert(!uf.unite(0, 1));
uf.undo();
assert(uf.same(0, 1));
uf.rollback(state);
assert(!uf.same(0, 1));
```

## 保存位置と履歴の消去

`get_state()`は現在の履歴上の位置です。完全永続Union Findの版ハンドルとは異なり、巻き戻しで捨てた枝を再び選ぶことはできません。整数が範囲内でも、別の枝や別インスタンスで得た位置を同じ状態として扱うことはできません。

`snapshot`の保存枠は1個です。保存位置より手前へ`undo`や`rollback`した場合、保存位置も到達位置へ切り詰めます。

`clear_history()`は成分と代表を保ったまま、その状態を新しい履歴の開始点にします。例外を送出しません。保存済みの位置は無効になり、`get_state()`とsnapshot位置は0になります。履歴の確保済み容量は保持し、その後の結合へ再利用します。

```cpp
uf.unite(0, 1);
uf.clear_history();
assert(uf.get_state() == 0 && uf.same(0, 1));
uf.unite(1, 2);
uf.rollback(0);
assert(uf.same(0, 1) && !uf.same(0, 2));
```

`reset(n)`は頂点・履歴に必要な容量を先に確保し、全頂点を別々の成分へ戻します。引数なし`reset()`は頂点数を保ちます。`clear()`は頂点数も0にします。いずれも履歴と保存位置を0にし、以前の状態位置は無効になります。`clear_history()`と異なり、以前の連結関係は残りません。確保済み容量は減らしません。

## 計算量と所有権

頂点数を`n`、現在の履歴長を`h`、確保済み履歴容量を`H`とします。構築はO(n+1)時間、領域はO(n+H+1)です。巻き戻しや`clear_history`では履歴容量を減らしません。代表探索・同一成分判定・成分サイズはO(log(n+1))、結合は履歴確保を含めて償却O(log(n+1))時間です。履歴の再確保が起きた1回だけを見ると、既存履歴の移動費用もかかります。

`undo`はO(1)、`rollback`は戻す呼出し数を`k`としてO(1+k)です。Clangの`rollback`は末尾に連続する不成功結合の履歴を、頂点配列への書戻しを行わずに取り除きます。残りの履歴は保存した値を逆順に復元します。履歴位置と`undo`の回数は変わりません。`clear_history`は標準コンテナの保証に基づく上界でO(h+1)時間です。引数なし`size`、`empty`、`component_count`、`snapshot`、`get_state`、`swap`はO(1)です。`groups`はO(1+n log(n+1))時間、O(n+1)追加領域です。全頂点が独立または全頂点が1成分なら代表探索を省いてO(n+1)時間で一覧を作ります。その他は根の負サイズから各成分の容量を確保し、代表から出力位置への作業配列1本で頂点を並べます。ClangのAVX2対応CPUでは256頂点以上の代表探索を8頂点ずつ行い、代表・成分位置の2配列を使います。出力領域の確保より前に代表を求め、根や探索済みのlaneは読み直しません。Clangの標準経路は代表・件数・成分位置の3配列です。成分は代表頂点の昇順、成分内は頂点の昇順です。`SIRKIT_DISABLE_SIMD`で追加のAVX2実装とCPU判定を除きます。`reserve_history`は再確保する場合O(h+1)時間、しない場合O(1)です。

コピー構築は履歴と保存位置も含めて独立し、O(n+h+1)時間・領域を使います。コピー代入は必要な頂点・履歴容量を先に確保し、代入先の領域を再利用します。代入先の旧頂点数・履歴長も含めた時間の上界はO(旧n+旧h+n+h+1)です。`reset(m)`の時間上界はO(n+h+m+1)、`clear()`はO(n+h+1)で、`clear()`は例外を送出しません。一方の変更はもう一方へ影響しません。`groups`の戻り値も自身で値を所有します。別インスタンスへ移動すると移動元は空になります。自己代入・自己移動では内容を保ちます。

結合時の履歴確保、コピー、`reset`が例外で失敗しても、既存インスタンスの連結状態・履歴・保存位置は保たれます。コピー代入と`reset`では先行した予約によって容量が増える場合があります。所有には標準コンテナを使い、破棄で領域を解放します。

## 入力検査

既定では指定した頂点、空履歴への`undo`、不正な巻き戻し位置を拒否します。検査のためだけに配列や全履歴を走査しません。

`SKIP_RUNTIME_CHECKS`を定義すると入力検査を除外します。頂点や履歴位置の前提は利用側で満たしてください。頂点数と履歴回数の内部上限は検査省略時も維持します。

Nyaanの実装から、負サイズ・サイズによる結合・不成功結合も履歴へ残す方式を引き継いでいます。[来歴とライセンスの監査](../Provenance/provenance-rollback-union-find-second-pass-2026-10-22.json)に、原典と改修部分の根拠を記録しています。


## rollback-weighted-union-find

元文書: `Document/rollback_weighted_union_find.md`

# 巻き戻せる重み付きUnion Find

頂点間の差を持つ連結成分を管理し、追加した制約を逆順に取り消します。`unite(x,y,w)`の向きは`potential(y)-potential(x)=w`です。小さい成分を大きい成分へ接続し、経路圧縮は行いません。[実装](../../DataStructure/rollback-union-findw.cpp)・[使用例](../../Examples/rollback_weighted_union_find_example.cpp)

## API

nは頂点数、hは現在の履歴長です。以下はTの演算・コピー・破棄をO(1)とした費用です。

| 操作 | 動作・時間 |
| --- | --- |
| `RollbackWeightedUnionFind<T>(n=0)` | n頂点を独立にする。O(n+1) |
| `find(v)` / `same(x,y)` / `size(v)` | 代表 / 同じ成分か / 成分サイズ。O(log(n+1)) |
| `root_with_weight(v)` / `get_weight(v)` | `{代表,代表からvへの差}` / その差。O(log(n+1)) |
| `diff(x,y)` / `try_diff(x,y)` | y−xの差 / `optional<T>`。O(log(n+1))。diffは同じ成分、try_diffは別成分ならnullopt |
| `consistent(x,y,w)` | 別成分ならtrue、同じ成分なら差がwか。O(log(n+1)) |
| `unite(x,y,w)` | 新しく結合した場合だけtrue。既結合時は差を検査しない。償却O(log(n+1)) |
| `unite_checked(x,y,w)` | `Merged` / `AlreadyConsistent` / `Contradiction`。償却O(log(n+1)) |
| `get_state()` / `snapshot()` | 履歴位置を返す / 位置を1個だけ保存。O(1) |
| `undo()` / `rollback(state=-1)` | 追加1回を戻す / 指定位置へ戻す。O(1) / 戻す回数kとしてO(k+1)。−1は保存位置 |
| `clear_history()` | 成分と差を保ち、履歴・保存位置を0へ。容量を保持。O(h+1)の上界 |
| `reserve_history(k)` | k回分を予約。再確保時O(h+1)、予約済みならO(1) |
| `size()` / `empty()` / `component_count()` | 全頂点数 / 空か / 成分数。O(1) |
| `groups()` | 成分ごとに頂点を昇順で列挙。O(1+n log(n+1))時間、O(n+1)追加領域 |
| `reset(n)` / `reset()` | 指定頂点数 / 現頂点数で独立な状態へ。新頂点数mとしてO(n+h+m+1) |
| `clear()` | 空へ戻し配列容量を保持。O(n+h+1)の上界 |
| `swap(other)` / `swap(a,b)` | 履歴・保存位置も含め交換。O(1) |

冗長・矛盾の追加も、例外が起きなければ履歴を1回進めます。矛盾した追加は成分と差を変えず、矛盾フラグを残すAPIではありません。全成分が独立、または1成分の場合の`groups`はO(n+1)です。

```cpp
using UF = RollbackWeightedUnionFind<std::int64_t>;
UF uf(3);
uf.unite(0, 1, 4);
const int state = uf.get_state();
uf.unite(1, 2, -7);
assert(uf.diff(0, 2) == -3);
assert(uf.unite_checked(0, 2, 9) == UF::MergeResult::Contradiction);
uf.undo();                 // 矛盾した追加だけを戻す。
uf.rollback(state);        // 1と2の結合も戻す。
assert(!uf.same(0, 2));
uf.clear_history();        // 差4を新しい開始状態として残す。
```

## 型・範囲・履歴の前提

Tは0の構築、コピー・ムーブ・代入、加減算を備える可換加法群です。整合性の判定には`==`も必要です。modintを使え、符号なし整数は型の幅の剰余として扱います。浮動小数点の許容誤差判定はありません。

標準の符号付き整数では、途中和がTを超えても広い累積値で相殺を保ち、照会の最終値がTに収まらないと`overflow_error`です。結合では、サイズにより選ぶ根どうしの差をTに保存できる必要があります。`consistent`は論理的な整合性を調べるもので、この保存可能性を保証しません。128ビット整数拡張がなければ標準C++の2語整数で計算し、`SIRKIT_FORCE_STANDARD_ARITHMETIC`でこの代替経路を選べます。

頂点は`0<=v<size()`、履歴位置は`0<=state<=get_state()`、履歴長は`INT_MAX`以下です。位置は現在の枝に対するもので、捨てた枝や別インスタンスを選べる永続的な版番号ではありません。保存位置より前へ戻すと保存位置も切り詰めます。`reset`・`clear`・`clear_history`の前の位置は無効です。

## 所有・例外・検査

保持領域は確保済み履歴容量HとしてO(n+H+1)。コピーはO(n+h+1)で独立、ムーブ構築はO(1)で元は空です。ムーブ代入には代入先の旧状態を破棄する費用もかかります。`reset`は型の例外特性と容量に応じて領域再利用か一時構築を選び、後者では新旧両方の領域を一時的に使います。

`reset`で0の構築や確保に失敗しても成分・差・履歴を保ちますが、容量は増える場合があります。標準整数・通常のmodintでは、結合時の履歴確保失敗でも状態を保ちます。値を変えてから投げる独自型の代入など、全ての例外に対する状態維持は保証しません。有効な履歴のundo・rollbackは独自Tの算術・構築・代入を行いません。

既定では頂点、空履歴のundo、不正な位置、未連結のdiffを検査します。検査専用の全件走査はしません。`SKIP_RUNTIME_CHECKS`時は利用側で前提を満たし、容量・整数演算の保護は維持します。`SIRKIT_DISABLE_SIMD`で追加のSIMD実装とCPU判定を除けます。

## Verify

| 問題 | 検証範囲 |
| --- | --- |
| [Unionfind with Potential](https://judge.yosupo.jp/problem/unionfind_with_potential) | 非零重み・矛盾・差。巻き戻しは対象外 |
| [無向辺2-SAT](https://yukicoder.me/problems/no/2293) | 重み0/1・矛盾・成分数と全条件の`rollback(0)` |
| [Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) | 重み0の版木・rollback。非零重みの復元は対象外 |
| [秘密の数列と分岐するノート](https://atcoder.jp/contests/awc0100/tasks/awc0100_m) | 任意の剰余重み・差・矛盾と、分岐する版のrollback |

新しい2本はオンライン未提出です。版木DFSや全消去を含む問題全体はO(N+Q log(N+1))時間・O(N+Q)領域です。整数端・所有権・その他のAPIは[独立参照試験](../../Verify/current/tests/rwuf_groups_reset_reference_test.cpp)等で補い、判定は[Verify一覧](../../Verify/current/README.md)を参照してください。[来歴の監査](../Provenance/provenance-rollback-weighted-union-find-second-pass-2026-10-22.json)


## persistent-union-find

元文書: `Document/persistent_union_find.md`

# 完全永続Union Find

保存した過去の版を参照し、その版から新しく結合できます。各版は非所有の`Node*`で表し、変更前の版を維持します。[使用例](../../Examples/persistent_union_find_example.cpp)を参照してください。

各頂点に親、根には成分サイズの負数を保持します。成分サイズの小さい根を大きい根へ接続し、経路圧縮は行いません。親とサイズは永続配列に保存し、更新経路だけをコピーします。結合時の2点更新は共通経路を一度だけ複製します。同一成分照会・結合では2頂点の親を並行して参照し、独立な経路の読み出しを重ねます。任意の保存版から分岐できる完全永続構造です。

## Union FindのAPI

| API | 動作 |
| --- | --- |
| `PersistentUnionFind(n=0)` | `n`頂点を別々の成分として構築する。`n<=INT_MAX`。 |
| `find(i[,version])`、`leader`、`root_of` | 指定版、または現在版での代表頂点。 |
| `find_with_size(i[,version])` | `{代表頂点,負の成分サイズ}`。 |
| `size(i[,version])` | 指定頂点の成分サイズ。 |
| `same(i,j[,version])` | 同じ成分か。 |
| `unite(i,j[,version])`、`merge` | 指定版から結合し、結果を現在版にする。新しく結合したときだけ`true`。 |
| `get_root()`、`restore(version)` | 現在版の取得、現在版の選択。 |
| `size()` | 全頂点数。 |
| `component_count([version])` | 成分数。 |
| `groups([version])` | 成分ごとの頂点一覧。各一覧の頂点は昇順。 |

`[,version]`は版を末尾に指定するオーバーロードがあることを表します。頂点は`0<=i<n`です。結合が不要な場合も、渡した版が現在版になります。

代表探索は親を最大O(log(n+1))回たどり、そのたびに永続配列を参照するため、`find`、`size(i)`、`same`はO(log²(n+1))時間、`unite`はO(log²(n+1))償却時間です。成功した結合はO(log(n+1))個のノードを追加します。`get_root`、`restore`、引数なし`size`はO(1)時間です。`component_count`はO(n log(n+1))、`groups`はO(n log²(n+1))時間です。

```cpp
PersistentUnionFind uf(4);
auto initial = uf.get_root();
uf.unite(0, 1);
auto left = uf.get_root();
uf.unite(2, 3, initial); // initialから別の枝へ
assert(!uf.same(0, 1));
assert(uf.same(0, 1, left));
uf.restore(left);
assert(uf.size(0) == 2);
```

## 成分列挙の参照

GCCの`groups`内部では、最初は疎な配列として参照し、親をたどるときは確保済みの経路を参照します。`unite`は親代表のサイズと子の親番号を同じ版へ記録するため、親リンク先にはその版のノードが必ず存在します。版の分岐・復元や頂点番号の付け替えでもこの性質を保ちます。列挙する添字は`[0,n)`にあり、版の検査は列挙前に一度行います。

## 永続配列のAPI

`PersistentArray<T,shift=4>`も単独で使えます。分岐数を`B=2^shift`とし、`1<=shift<=8`です。座標は`int64_t`で表す添字で、サイズは`0<=n<=INT64_MAX`です。値型にはコピー構築とコピー代入を要求します。既定引数の`T{}`を使う場合は、デフォルト構築も必要です。

| API | 動作 |
| --- | --- |
| `PersistentArray(n=0,ID=T{})` | 全要素の初期値が`ID`の疎な配列。 |
| `PersistentArray(vector,ID=T{})` | `vector`の値から構築する。 |
| `get(k)`、`get(version,k)` | 現在版、指定版の要素を値で返す。 |
| `get_two(a,b)`、`get_two(version,a,b)` | 2要素を `pair<T,T>` で返す。指定順を保ち、同じ添字も許す。 |
| `update(k,value)`、`update(version,k,value)` | 1点を変更して現在版にし、その`Node*`を返す。 |
| `update_two(a,x,b,y)`、`update_two(version,a,x,b,y)` | 2点を同時に変更し、結果を現在版にして`Node*`を返す。同じ添字なら`y`を優先する。 |
| `restore(version)`、`root` | 現在版の選択、現在版のハンドル。 |
| `size()` | 要素数。 |

配列の`nullptr`版は、全要素が`ID`の版です。Union Findの非空インスタンスでは、公開APIから得た非null版を使ってください。

`get_two`は2本の経路を同じ反復で参照します。片方が疎な未確保部分なら、その要素だけ初期値を返します。`get`と`get_two`はO(log_B(n+1))時間、`update`と`update_two`はO(B log_B(n+1))償却時間・償却追加領域です。保存ブロックを管理するvectorの拡張費用を含みます。定数配列の構築はO(B+log_B(n+1))時間で、全要素を確保しません。`vector`からの構築は葉ごとに値を続けて代入し、根から葉までの再走査を減らします。O(B+n+(n/B+1)log_B(n+1))時間、O(n+B)領域です。

```cpp
PersistentArray<int> a(4, 0);
auto initial = a.root;
a.update_two(0, 7, 3, 9);
assert(a.get(0) == 7 && a.get(3) == 9);
assert(a.get(initial, 0) == 0);
auto pair = a.get_two(0, 3); // {7,9}
auto previous = a.get_two(initial, 0, 3); // {0,0}
a.update_two(initial, 1, 4, 1, 8);
assert(a.get(1) == 8 && a.get(0) == 0);
```

## 版と所有権

ノードは小さいブロックから段階的に確保します。各ブロックの容量内でだけ追加し、以後の更新で過去のノードのアドレスを変えません。

葉と枝は木の深さから決まります。GCCでは、自明にコピーできる値型のノードをタグのないunionで保持します。Clang・その他のコンパイラと、自明にコピーできない値型にはvariantを使います。どちらも同じAPIと版の所有権を保ちます。

コピーは保存領域を共有し、現在版のハンドルをそれぞれ保持します。一方のコピーで更新しても、もう一方の現在版や保存済みの版は変わりません。移動元は空になります。

`Node*`は非所有ハンドルです。同じ保存領域を所有するインスタンスが1つ以上残っている間だけ使えます。最後の所有者の破棄で全版をまとめて解放し、木の深さに応じた再帰解放はしません。別の保存領域のハンドル、寿命切れのハンドルは渡せません。並行更新には外部同期が必要です。

更新が例外で失敗しても、現在版と既存の版の内容は維持します。作成途中の未公開ノードを保存領域に保持する場合があり、失敗時に保存領域の容量まで巻き戻す保証はありません。

Union Findの`arr`と`root`は互換用の公開フィールドです。`arr`を直接書き換えると親とサイズの不変条件を壊せるため、通常はUnion FindのAPIを使います。

## 入力検査

既定では添字の範囲と版の所有元を、対象への操作中に確認します。Union Findの非空インスタンスにnull版を渡すと`invalid_argument`、範囲外の添字は`out_of_range`です。寿命切れポインタを検査できる機能ではありません。

`SKIP_RUNTIME_CHECKS`を定義すると入力検査と検査専用の所有元情報を除外します。サイズ・ノード容量の内部上限は維持します。検査省略時も、添字と版の前提を満たす必要があります。

出典とライセンスの根拠は[来歴監査](../Provenance/provenance-persistent-union-find-second-pass-2026-10-22.json)を参照してください。

[成分列挙の独立参照試験](../../Verify/current/tests/puf_groups_reference_test.cpp)では、保存版ごとのラベルモデルと親リンク先の確保を照合します。


## dynamic-fenwick

元文書: `Document/dynamic_fenwick.md`

# 動的Fenwick Tree

[実装](../../DataStructure/dynamic-binary-indexed-tree.cpp)・[使用例](../../Examples/dynamic_fenwick_example.cpp)

## 対象と前提

`DynamicFenwickTree<S,T>`は、長さだけを指定した零配列への単点加算・区間和を扱います。更新位置の事前登録は不要です。
`S`は`bool`以外の64bit以下の整数型です。添字は0始まり、区間は半開区間で、加減算の結果は`T`の表現範囲内とします。
`lower_bound`と`upper_bound`は全要素が非負であることを前提にします。検査のための全要素走査は行いません。

## 保持方法

整数の値型（`bool`を除く）と長さ`n<=INT32_MAX`では、更新点数とセル密度に応じて保持方法を選びます。

| 条件 | 保持方法 |
| --- | --- |
| `n<=64` | 初回更新で直接配列を確保 |
| その他で異なる更新位置が32個以下 | 点列。4点まではオブジェクト内に保持 |
| `n<8192`で点列から移行 | 更新経路のFenwickセルをハッシュ表で保持 |
| `n<8192`で登録セル数が`ceil(n/8)`以上 | 直接配列へ展開 |
| `n>=8192`で33〜128点 | 列・値の配列と位置索引 |
| 129点以上 | 部分木の累積値を持つB+木 |
| B+木で`n<=100万`かつ登録セル数が`ceil(n/5)`以上 | 直接配列へ展開 |

先頭から連続して追加される場合は、B+木の登録セル数が`ceil(n/32)`以上で配列化を早めます。
配列化後の連続追加では、その位置のFenwickセルを完成させ、和を親1個だけへ伝えます。
親より上のセルには、その親が完成した時に伝えます。連続追加1回あたりの計算は償却O(1)です。
末尾到達や任意更新へ移る際には、登録ビットと上位セルをまとめて反映します。
途中の和・単点取得・探索は構築済みの範囲から答えるため、読み取りで状態変更や確保は行いません。
順番が変わる更新の前に上位セルを補い、末尾まで追加した場合も通常の配列へ戻ります。
配列用の確保に失敗した場合は、それまでの保持方法を維持します。

配列の更新では、登録済みセルの祖先もすべて登録済みである性質を使います。
更新位置のセルが登録済みなら、各祖先で登録状態を再判定せずに値だけを更新します。

大きな添字領域、浮動型・独自の値型は、更新したFenwickセルをハッシュ表に持ちます。
条件に合う値型と寸法では、密になったとき16MiB以内の直接配列へ移ります。
浮動型・独自型の和は、従来のFenwickセルの順序で計算します。

## 計算量と読み取り

構築はO(1)、通常の更新・和・探索は期待償却O(log n)です。
少数点の点列・固定配列には、それぞれ最大32点・128点の処理があります。
直接配列への変換はO(n)、表や木の容量拡張は保持量に比例する費用がかかります。

区間和は共通のFenwick祖先より先を省ける形式では打ち切ります。
累積和と単点取得には専用の経路を使います。密な整数配列の区間和は、左右の累積和を分け、2段ずつ交互に読みます。
終端に使う0番セルは常に0です。左端が0なら右側だけを計算します。GCCのx86系でAVX2を既定のコンパイル対象に含めない場合、両端の上位ビットが一致する64幅の区間は下位6bitだけを辿ります。共通の上位セルは相殺し、局所添字0を全体の0番セルへ写すため、左右の長さが違っても追加の分岐なしで集計できます。直接配列が全て零初期化されている間は、読み取り時の占有判定を省きます。
`clear`後に古い値を残す形式では占有情報を参照し、未更新セルを零として扱います。
読み取り・探索は領域を確保せず、登録セル数を増やしません。

1次元の整数配列には、探索用の番兵を実際の寸法より後に置きます。物理容量は原則として次の2冪までで、更新経路・区間和・登録件数には番兵を含めません。
これにより探索ループの端点判定を省きます。寸法が2冪なら根で上限を判定し、それより小さい段から探索します。

`stored_nodes()`は通常のFenwick更新経路の和集合の大きさです。実際の点数やB+木ノード数ではありません。
零の加算・相殺された値も登録済みとして数えます。`reserve`もこの件数を変えません。
連続追加の途中は上位祖先を数えるためO(log n)、通常の状態ではO(1)です。

## API

| 操作 | 意味 |
| --- | --- |
| `add(k,x)` / `set(k,x)` | 単点の加算／置換 |
| `sum(k)` / `sum(l,r)` / `sum_all()` | 前方累積和／区間和／全体和 |
| `get(k)` / `operator[](k)` | 単点取得 |
| `lower_bound(w)` / `upper_bound(w)` | 累積和が`w`以上／より大きくなる最初の位置。なければ`size()` |
| `size()` / `empty()` | 長さ／長さが零か |
| `clear()` / `reset(n)` | 値を零へ戻す／寸法も変更。最悪O(C)、Cは保持する全配列・表・木の総容量 |
| `reserve(nodes)` | セル数を目安に予約。再配置は期待O(C+C'+log(n+1))、C'は予約後の総容量 |
| `stored_nodes()` | 登録済みFenwickセル数。通常O(1)、連続追加の途中はO(log n) |

コピーは独立した値を持ちます。move後の元オブジェクトは長さ零として再利用できます。
`clear`は容量を再利用し、`reset`では保持形式と寸法に応じて再利用できる領域を引き継ぎます。
入力検査は既定で有効で、`SKIP_RUNTIME_CHECKS`を定義すると検査を除きます。

整数の専用経路のコピー代入は、コピーを作ってから置き換えます。その他の値型では、代入に失敗した場合は元の長さの零状態へ戻します。


## dynamic-fenwick-2d

元文書: `Document/dynamic_fenwick_2d.md`

# 動的2次元Fenwick Tree

[実装](../../DataStructure/dynamic-binary-indexed-tree-2d.cpp)・[使用例](../../Examples/dynamic_fenwick_2d_example.cpp)

## 対象と前提

`DynamicFenwickTree2D<T>`は零配列への単点加算と矩形和を扱います。更新位置の事前登録は不要です。
寸法は非負の`int`、添字は0始まり、区間は半開区間です。`T`は既定構築と必要な加減算に対応し、演算結果が表現範囲に収まる必要があります。
入力検査は既定で有効です。`SKIP_RUNTIME_CHECKS`を定義する場合も、有効な寸法・添字・区間を渡してください。

## 集計の仕組み

外側のFenwick Treeが行方向を集計し、各行ノードが列区間の和を担当します。
行ノードは必要時に作り、行数100万以下では添字配列、それを超える場合はハッシュ表で参照します。
列方向の保持方法は次のとおりです。整数型には`bool`を含めません。

| 条件 | 保持方法 |
| --- | --- |
| `m<=64`、または`m<=2048`かつ`n*m<=2^22` | 必要になった行だけ直接配列を用意 |
| その他の整数行で、昇格前の更新列が32種類以下 | 点列。最初の4列は行内、5〜32列には列索引も使用 |
| `m<8192`の整数行で33種類目を追加 | 更新したFenwickセルだけをHashへ保持 |
| `m>=8192`の整数行で33〜128種類の列を保持 | Flat：列・値の配列と256スロットの列索引 |
| Flatへ129種類目を追加 | B+木：葉32点・内部16子、元の列と区間集計値を保持 |
| B+木で`m<=1,000,000`かつ仮想セル数が`ceil(m/5)`以上 | Dense：列方向のFenwickセルを直接配列へ展開 |
| その他の値型 | 更新したFenwickセルだけをHashへ保持 |

Flatの単点取得・既存列の加算は列索引を使い、通常の区間和は128要素の配列を走査します。未使用列は和に含めません。
B+木は元の列を順序付きで持ち、部分木の累積値から区間和を求めます。Denseでは占有bitで未更新セルを区別し、全セル占有時にはその確認を省きます。
Denseへの変換では値配列を線形構築します。占有bitは64列内をシフト・ORで集約し、64の倍数のセルだけブロック間で伝播します。
整数の点列・Flat・B+木では型のビット幅に対応する符号なし集計を使います。浮動型・独自型には、この集計順変更を適用しません。

## 計算量と領域の準備

各行が保持する異なる更新列の数を`k`とします。通常の列方向操作は次のとおりで、外側ではO(log n)個の行ノードをたどります。

| 保持方法 | 加算 | 列区間和／単点取得 |
| --- | --- | --- |
| 点列 | 既存列は期待O(1)、新規列はO(32) | O(32)／期待O(1) |
| Flat | 既存列は期待O(1)、新規列はO(128) | O(128)／期待O(1) |
| Hash | 期待償却O(log m) | 期待O(log m) |
| B+木 | 償却O(log k) | O(log k) |
| 直接配列・Dense | O(log m) | O(log m) |

構築はO(1)です。行数100万以下では初回更新の索引準備にO(n)、直接配列を使う行の初期化にO(m)が必要です。
33点目のHash移行は期待O(32 log m)、Flat移行は128点分の初期化と32点のコピーが必要です。129点目では一時B+木へ128点と新しい点を追加するため、Dense化を除きO(129 log 129)の構築費用が発生します。
B+木からDenseへの変換はO(m)です。Hashや木の配列再確保にも保持量に比例する費用があり、通常操作の計算量とは区別します。Dense用の確保に失敗した場合はB+木を使い続けます。
メモリは行索引・行プールと各行の保持領域に依存します。Dense行はO(m)、Flat行は128点分、HashとB+木は保持したセル／点に応じた領域を使います。読み取りではノードや領域を追加しません。

`stored_nodes()`は仮想的な2次元Fenwickセルの件数です。点列・Flat・B+木でも更新経路の和集合を数え、実際の保持点数・木のノード数とは区別します。0の加算や相殺された値も登録済みとして数えます。

## API

| 操作 | 意味 |
| --- | --- |
| `add(i,j,x)` | `(i,j)`へ`x`を加算 |
| `sum(n,m)` | `[0,n)×[0,m)`の和 |
| `sum(nl,ml,nr,mr)` | `[nl,nr)×[ml,mr)`の和 |
| `get(i,j)` / `set(i,j,x)` | 単点の取得／変更 |
| `sum_all()` | 配列全体の和 |
| `rows()` / `cols()` / `empty()` | 寸法／いずれかの寸法が0か |
| `clear()` | 寸法を保ち零へ戻す。O(I+H)、I=行索引の長さ、H=疎行索引の表容量 |
| `reset(n,m)` | 寸法と値を設定し直す。O(I+H)。行の値配列は即座に走査しない |
| `reserve(nodes)` | 行プール・疎索引の予約。再配置は期待O(C+C')、C/C'は予約前後の全保持容量 |
| `stored_nodes()` | 仮想Fenwickセルの件数。O(1) |
| `swap(other)` | 寸法・値・確保領域を交換 |

コピーは独立した値を持ち、move・swapは所有領域を移します。`clear`後は保持した形式と領域を再利用します。`reset`では列幅や保持方法に応じて必要な領域を作り直します。

コピー・move・swap・clear/reset後の再利用は、[公開APIの所有権試験](../../Verify/current/tests/dynamic_fenwick_2d_ownership_behavior_test.cpp)で点辞書と更新セル集合に照合します。コピー代入中の確保失敗も確認し、保持形式や確保回数を固定しません。


## heap

元文書: `Document/heap.md`

# 最小ヒープ

`DataStructure/heap.cpp`の`Heap`は、`int`を所有する最小ヒープです。外部ライブラリに依存せず貼り付けられます。使用例は[heap_example.cpp](../../Examples/heap_example.cpp)です。

## API

| 操作 | 意味・前提 | 計算量 |
| --- | --- | --- |
| `Heap()` | 空で構築 | O(1) |
| `Heap(vector<int>)` | 配列を受け取って構築。ムーブで所有権を渡せる | O(N) |
| `Heap({値...})`, `Heap(first,last)` | 値をコピーして構築 | O(N) |
| `empty()`, `size()` | 空判定・要素数 | O(1) |
| `top()`, `top_or(fallback)` | 最小値。空ならそれぞれ`-1`・`fallback` | O(1) |
| `push(value)` | 値を追加 | 償却O(log N) |
| `pop()` | 最小値を削除。空なら何もしない | O(log N) |
| `pop_value()` | 最小値を削除して返す | O(log N) |
| `replace_top(value)` | 最小値を置換し、旧値を返す | O(log N) |
| `push_pop(value)` | 追加後の最小値を削除して返す。空なら`value`を返す | O(log N) |
| `clear()` | 空にする。確保済み容量は保持 | O(1) |
| `reserve(n)` | `n`要素分の容量を確保 | 再確保時O(N) |

`pop_value()`と`replace_top()`は、空なら`std::out_of_range`を送出します。`SKIP_RUNTIME_CHECKS`を定義するとこの検査を除き、非空が前提となります。`top()`の`-1`は空の目印と値の区別がつかないため、必要なら`empty()`を併用します。

コピーは独立した配列を持ち、ムーブは配列の所有権を移します。負値・重複値・`int`の両端値も格納できます。

## 構造

要素は一つの`std::vector<int>`が所有します。GCCのx86で`__AVX2__`が未定義の場合は、根の子を2個、それ以降の子を4個にしたヒープを使います。添字`i > 0`の親は`(i+1)/4`、根以外の頂点`i`の子は`4*i-1`から`4*i+2`です。根の修復では最初に2個の子を調べるため、根付近で終わる置換も短い処理で済みます。

追加は最初の親を確認し、上昇が不要なら挿入済みの位置で処理を終えます。上昇する場合は親を下へ移し、挿入位置へ値を置きます。配列からの構築は末尾側の親から修復し、O(N)時間です。

それ以外のコンパイル条件では二分ヒープを使い、添字`i`の子は`2*i+1`と`2*i+2`です。この場合の構築は`std::make_heap`です。どちらの構造も標準C++の整数演算で動作し、要素の所有方法と公開APIは共通です。

## Verify

[ABC234 D - Prefix K-th Max](https://atcoder.jp/contests/abc234/tasks/abc234_d)では、先頭から追加し、要素数がKを超えたら最小値を削除します。[提出用コード](../../Verify/current/submissions/heap_priority_queue.cpp)には共通の高速入出力と検査省略の設定を含めています。

[ローカル検査](../../Verify/current/tests/heap_test.cpp)では、オンライン問題に含まれない置換・重複・負値・整数端点・コピー/ムーブ・空の扱いを`std::multiset`と照合します。判定・測定の証拠は`Audit/`に置きます。


## node

元文書: `Document/node.md`

# 単方向ノードと連結列

`Node`は値を持たず、`std::shared_ptr<Node> n`で次のノードを所有します。[API一覧](../../DataStructure/Node.cpp)と[使用例](../../Examples/node_example.cpp)を参照してください。

コピーは後続ノードを共有します。共有されていない長い後続列は反復処理で解放し、連結に沿った深い再帰を避けます。循環参照の自動回収は行いません。

| 操作 | 戻り値・前提 |
| --- | --- |
| `make_node(next={})` | `next`を後続に持つ新しいノード。O(1)時間・1回確保。 |
| `make_chain(count, tail={})` | `tail`の前に`count`個追加した先頭。O(count)時間・count回確保。 |
| `chain_size(head)` / `chain_tail(head)` | ノード数／末尾。空の末尾はnullptr。循環なしでO(k)時間・O(1)追加領域。 |
| `chain_nth(head, index)` | 0始まりのindex番目、不足ならnullptr。循環も可。O(index+1)時間上界。 |
| `push_front_node(head)` / `pop_front_node(head)` | 先頭の追加／削除。削除は空ならfalse。O(1)時間。 |
| `reverse_chain(head)` | 循環のない列を反転して新しい先頭を返す。O(k)時間。 |
| `clone_chain(head)` | 循環のない列の独立した複製。O(k)時間・k回確保。 |
| `cycle_entry(head)` / `has_cycle(head)` | 循環入口の非所有ポインタ／循環の有無。O(k)時間・O(1)追加領域。 |
| `break_cycle(head)` | 循環を切断したらtrue。O(k)時間・O(1)追加領域。 |
| `clear_chain(head)` | 先頭を空にし、非共有の後続を反復解放。循環回収は先にbreak_cycle。 |

`chain_size`、`chain_tail`、`clone_chain`は既定で循環を検出し、`invalid_argument`を送出します。`SKIP_RUNTIME_CHECKS`時は循環のない入力が前提です。

`reverse_chain`と`break_cycle`による接続変更は、同じノードを共有する側にも見えます。`pop_front_node`は共有された元先頭の`n`を切断しません。

`push_front_node`は新ノードの確保に成功してから接続を移すため、確保に失敗しても元の先頭と接続を保持します。

`cycle_entry`はFloyd法で入口を求め、`has_cycle`は合流の検出で終了します。`break_cycle`は速い側のポインタの直前も保持し、合流後に入口と周期末尾を同期して探します。先頭から循環する場合は合流時点で末尾も分かるため、周期をもう一周しません。切断直前に先頭の所有権を保持するため、切断する`n`自身を引数にしても切断中のノードは生存します。

`SKIP_RUNTIME_CHECKS`時の`chain_size`は2ノードずつ数えます。個数の表現上限に近づいた場合は1ノードずつ進め、検査を省略してもオーバーフローを防ぎます。

GCCでは`chain_nth`が空列を先に返し、2ノードずつ進めます。残り1個は通常の走査で処理し、不足した位置の所有権も保持して返します。循環上でも指定した回数だけ進みます。

オンラインVerifyは[ABC228 B - Takahashi's Secret](https://atcoder.jp/contests/abc228/tasks/abc228_b)です。各人をノード、秘密を伝える相手を次ノードとして接続し、始点からの循環を`break_cycle`で切断して`chain_size`で到達人数を数えます。始点から到達しない循環も、全ノードの接続を解除して回収します。共有された末尾、複製、逆転、確保失敗、長い列の反復破棄は独立したローカル検査で補います。


## erasable-priority-queue

元文書: `Document/erasable_priority_queue.md`

# 削除可能な優先度付きキュー

`ErasablePriorityQueue<T, Minimize>`はC++20以降で利用でき、追加・先頭削除に加えて、指定した値を1個削除できます。`Minimize=true`なら最小値、`false`なら最大値を先頭にします。

## APIと前提

- `ErasablePriorityQueue()`：空で構築します。`ErasablePriorityQueue(values)`はvectorを受け取り、線形時間で一括構築します。
- `push(value)` / `push(T&&)` / `emplace(args...)`：1要素を追加します。
- `top()` / `pop()`：先頭の値を返す／先頭を1個削除します。空でないことが前提です。
- `erase(value)`：存在する値を1個削除します。重複があれば1個分だけです。存在と個数の検査は行いません。
- `size()` / `empty()`：有効な要素数／空かどうかを返します。遅延削除の回収は行いません。
- `try_top()`：空なら`nullopt`、そうでなければ先頭の値を返します。
- `extract_top()`：空なら`nullopt`、そうでなければ先頭を1個削除して値を返します。
- `swap(other)`：有効な要素と遅延削除を定数時間で交換します。
- `clear()`：要素と遅延削除を破棄し、容量を解放します。

`T`は比較・コピーが可能な型を使います。比較は厳密弱順序を満たし、`==`はその同値関係と一致する必要があります。浮動小数点のNaNはこの条件を満たしません。move元は空になり、再利用できます。

既定では空の`top()`・`pop()`・`erase()`を例外で検出します。`SKIP_RUNTIME_CHECKS`を定義すると、この検査を除きます。空でない場合も、存在しない値の`erase()`は利用側の前提違反です。

## 方式と計算量

値を保持するヒープと、削除する値を保持するヒープを使います。先頭の取得・削除時に両方の先頭が一致していれば、両方から取り除きます。削除の記録は、対象が先頭に来るまで残ります。

GCC（Clangを除く）の64bit整数型では、1つの親に最大4個の子を持つ4分岐ヒープを使います。子の比較を明示し、木の段数と依存する読み取りを減らします。一括構築では葉に近い親から順に処理し、線形時間でヒープを作ります。

先頭削除では穴を葉まで下げ、末尾の値を親と比較しながら戻します。相殺する2つのヒープは交互に降り、各配列を独立に復元します。子が存在する親かを確認してから子の添字を計算し、容量や添字のオーバーフローを避けます。

GCCの32bit以下の整数型と、Clangなどの整数型では二分ヒープを使い、相殺時だけ2つの配列を交互に降ります。GCCでは通常の削除も配列上で下降・復元し、Clangなどは`std::priority_queue`を使います。整数以外と`bool`は保持型も処理も`std::priority_queue`です。追加の所有領域やCPU固有の命令は使わず、いずれも標準C++の処理です。

GCCの整数型では、削除待ちがなければ先頭照会の正規化を省きます。4分岐版では相殺処理の展開位置を調整し、その経路の呼出し費用を抑えます。

未回収の削除済み要素を含む保存数を`H`とすると、追加と削除予約は`O(log(H+1))`です。先頭取得・削除は回収を含めて償却`O(log(H+1))`ですが、1回の操作で多数の削除を回収することがあります。`size()`・`empty()`は`O(1)`、一括構築は`O(n)`、使用領域は`O(H)`です。

要素型や比較演算が例外を投げる場合の保証は、内部の`std::priority_queue`の操作に依存します。

## 使用例とVerify

- [使用例](../../Examples/erasable_priority_queue_example.cpp)
- [Library Checker提出コード](../../Verify/current/submissions/erasable_priority_queue_double.cpp)：Double-Ended Priority Queue。最小値・最大値優先の2表を同期し、入力を逐次処理します。
- [ローカル補足](../../Verify/current/tests/erasable_priority_queue_test.cpp)：`std::multiset`との照合で、整数端・重複・補助API・コピーとmoveを確認します。
- [ヒープ境界の補足](../../Verify/current/tests/erasable_priority_queue_boundary_test.cpp)：整数各幅の小サイズと4冪前後で、一括構築・逐次追加・遅延削除・コピーを独立照合します。

A+Bへの提出は、このライブラリのオンラインVerifyには使用しません。原典のunit testは方式の参考とし、独立照合はローカルで実行します。


## dynamic-li-chao

元文書: `Document/dynamic_li_chao.md`

# 動的Li Chao Tree

整数座標の閉区間で、追加した直線・線分の最小値を取得します。座標圧縮や照会座標の事前登録は不要です。

## APIと前提

`DynamicLiChaoTree<T, kInf>(xmin, xmax, inf=kInf)`で定義域`[xmin,xmax]`と正の番兵infを指定します。Tは符号付き64bit以下の整数で、既定はint64_t。kInfの既定は2^60で、Tで表せない場合はTの最大値です。テンプレート引数による指定も維持します。

- `add_line(a, b)` / `update(a, b)`：定義域全体に`y=a*x+b`を追加します。
- `add_segment(a, b, l, r)` / `update_line_segment(...)`：閉区間`[l,r]`に追加します。`l>r`なら無操作です。
- `query(x)`：最小値を返します。被覆する線がなければ指定した`inf`です。
- `query_line(x)`：最小値を与える`Line`を返します。同値時は任意、非被覆なら`nullopt`です。
- `infinity()`：設定したinfをO(1)で返します。
- `empty()` / `domain_min()` / `domain_max()`：追加の有無と定義域を取得します。
- `stored_nodes()`：実際の格納ノード数です。照会では増えません。
- `Line(a=0,b=kInf)`：直線の係数。省略時はテンプレート引数側のkInfを使い、木ごとのinfとは連動しません。
- `clear()`：全ての線を消し、確保済み容量を再利用します。
- `reserve(n)`：ノード用の容量を確保します。直線本数とは異なります。

各直線・線分の**対象区間内**で、数学的な`a*x+b`が`T`に収まり、指定した`inf`未満である必要があります。中間の積だけが範囲を超え、切片との相殺で最終値が収まる場合も扱えます。線分の対象区間外の値は前提に含めません。

既定では座標と両端の値を検査します。線形関数なので、両端の確認で対象区間内の値域を判定できます。`SKIP_RUNTIME_CHECKS`で入力検査を除いても、内部のノード数上限は維持します。

## 方式と計算量

各ノードに区間の中央付近で有利な直線を保持し、競合する直線だけを子へ送ります。必要なノードを連続した配列へ追加し、子は添字で参照します。GCCでは、満杯の配列を`realloc`で拡張してコピーを減らします。所有権は`unique_ptr`と専用の解放処理で管理し、Clangなどでは`vector`を使います。

線分が区間の片側に収まる場合は反復で降下し、両側へまたがる場合だけ分岐します。既存直線との差は線形なので、重なる区間で差が最小になる端点を比較し、全域で勝てない線分の探索を省きます。この比較は線分の対象区間内で行います。切片が指定した`inf`の場合は枝刈りを省略しますが、有効な直線なら通常通り格納・評価します。

中点は順序付きの符号なし差分で計算し、符号付き整数の全域を定義域にしてもオーバーフローさせません。値だけの照会と直線を返す照会は同じ探索を使い、値だけの場合は係数の保存を省きます。

座標数を`W`、格納ノード数を`M`とすると、直線追加・照会は`O(log W)`、線分追加は`O(log² W)`、使用メモリは`O(M)`です。全域の64ビット整数座標でも高さは最大64です。

ノードは自明にコピー・破棄できる型に限定します。予約容量に空きがある場合の拡張では、有効なノードだけを新しい領域へコピーします。

空の構築・状態取得はO(1)、clearはO(M)、reserveは再確保時O(M)です。コピーはO(M)で独立した木を作り、moveはO(1)です。コピー・move・clear後もinfの設定を保持します。コピー代入ではノードのコピーを先に完了させ、確保失敗時に元の定義域とノードを維持します。

## 使用例とVerify

- [使用例](../../Examples/dynamic_li_chao_example.cpp)
- [直線追加の提出コード](../../Verify/current/submissions/dynamic_lichao_line.cpp)：Library CheckerのLine Add Get Min。
- [線分追加の提出コード](../../Verify/current/submissions/dynamic_lichao_segment.cpp)：Segment Add Get Min。問題の半開区間を`[l,r-1]`へ変換します。

API・全整数幅・入力検査・所有権はローカルの総当たり参照でも検証します。測定条件と検証結果は`Audit/`に保存します。


## hash-set

元文書: `Document/hash_set.md`

# HashSet

`DataStructure/Hash_set.cpp`は、キーの値を所有する開番地法の集合です。`string_view`などの参照先は所有せず、登録中は参照先が有効で、キーとして不変である必要があります。順序は持ちません。`GenericHash`は整数・文字列・pair・tuple・列などに対応し、独自の同値関係には対応するHasherとKeyEqualを渡します。

同じ要素を持つ`HashSet`は、内部のハッシュ種・列挙順・再配置の違いによらず、外側の`HashMap`や`HashSet`のキーとして同じハッシュ値になります。要素に独自の同値関係を使う集合をキーにするときは、その同値関係に合う外側のHasherも指定してください。

## 基本操作

`insert(key)`は既存または追加した要素のiteratorを返します。`insert_checked(key)`と`emplace(args...)`は`{iterator, 追加の有無}`を返します。`contains(key)`と`count(key)`は追加を行いません。検索・追加は期待償却O(1)で、列キーのハッシュ計算には列長に比例する費用がかかります。衝突が多い入力では時間が増えます。

`erase(key, false)`は次の要素を探さず削除し、`end()`を返します。有効なiteratorを使う`erase(it, false)`はO(1)です。省略時の`erase(key)`や`erase(it)`は次のiteratorを返すため、最悪O(capacity())です。

```cpp
HashSet<int> values;
values.insert(7);
if (auto it = values.find(7); it != values.end()) values.erase(it, false);
for (auto it = values.begin(); it != values.end();) it = values.erase(it);
```

`erase_if(pred)`は条件に合う要素を削除し、その数を返します。空集合ではO(1)、それ以外ではO(capacity())です。述語から表を変更しないでください。

`CompactSlot`に整数キーを保持し、`std::equal_to<Key>`または`std::equal_to<>`で比較する場合は、削除したキーの値を再登録の探索にも利用します。同値なキーの墓石に着いた時点で、その後ろに有効な同値キーはないため、その位置へ再登録できます。それ以外の型・比較器では登録済みの位置だけを比較します。削除したキーの参照先を保持し続ける必要はありません。削除したiteratorや参照が再び有効になるという意味ではありません。

## 容量・所有・反復子

`reserve(n)`はn要素分を予約し、`rehash(n)`はnバケット以上かつ現登録数に足りる容量へ再配置します。旧・新容量をC・C'、登録数をNとすると、再配置は期待O(C+C')、衝突が集中すると最悪O(C+C'+N²)です。ハッシュ・同値判定・要素操作の費用は別です。予約済みのreserveはO(1)。削除では自動縮小せず、`rehash(0)`で縮められます。

全列挙と`clear()`はO(C)です。空集合の検索・`begin()`・集合比較は容量によらずO(1)です。`clear()`は容量を保ち、要素も墓石もなければ走査を省きます。コピーは独立した集合を作り、移動元は空になって再利用できます。

キーの書き換えは禁止です。再配置・clear・swap・ムーブでiteratorと要素参照が無効になり、eraseは対象だけを無効にします。`SKIP_RUNTIME_CHECKS`でiterator検査とその専用状態を除きます。検査でも、消して同じ位置へ再登録したiteratorなど全ての無効な使用は検出できません。容量オーバーフローの保護は維持します。

## 再配置の高速化

GCCのSSE2対応x86では、64バケット以上の表の再配置で登録状態を16個ずつ判定します。自明にコピー・破棄できるキーだけに適用し、ハッシュが例外を送出しても元の表を保持します。GCCのx86では新しい格納領域の一部を値の構築前に順に書き、最初のランダムな書き込みの負担も抑えます。

小さい表・非自明なキー・Clang・その他の対象環境では標準C++のループを使います。`SIRKIT_DISABLE_SIMD`でSIMD専用コードを除外できます。領域の事前準備は標準C++の処理として維持します。キーを探す探索手順は共通です。

## 反復子の状態走査

GCCのSSE2対応x86では、`optional`でキーを保持する場合、`begin()`・前後のiterator移動・次iteratorを返す削除で、先頭4バケットを逐次確認し、続く空き区間を16バケットずつ調べます。登録状態だけを読み、範囲外や破棄済みのキーには触れません。

`CompactSlot`、Clang、その他の対象環境、`SIRKIT_DISABLE_SIMD`指定時には通常の逐次走査を使います。iteratorや表の追加状態はありません。

## Verify

オンライン問題の入力に対する集合操作が、実際の出力に反映される問題を使います。

| 提出先 | 確認する操作 |
|---|---|
| [ABC344 C - A+B+C](https://atcoder.jp/contests/abc344/tasks/abc344_c) | 最大100万回の整数追加、重複、所属照会 |
| [ABC073 C - Write and Erase](https://atcoder.jp/contests/abc073/tasks/abc073_c) | 整数の追加・検索・削除・再登録・要素数 |
| [ABC164 C - gacha](https://atcoder.jp/contests/abc164/tasks/abc164_c) | 文字列の追加・重複・要素数 |

他のAPI・衝突・例外・コピー/移動・容量境界は、`Verify/current/tests/hash_set_test.cpp`と`hash_set_boundary_test.cpp`で独立参照に照合します。`hash_set_unit`は固定種のローカル検査です。速度比較は同じ問題・main・入出力・コンパイラ設定で行います。

使用例は`Examples/hash_set_example.cpp`、提出コードは`Verify/current/submissions/hash_set_membership.cpp`、`hash_set_toggle.cpp`、`hash_set_strings.cpp`です。


## implicit-treap

元文書: `Document/implicit_treap.md`

# 暗黙Treap

`DataStructure/implicit_treap.cpp`の`lazy_implicit_treap`は、挿入・削除・区間操作を行う可変長列です。公開APIは`namespace sirkit`内にあります。[使用例](../../Examples/implicit_treap_example.cpp)。

`S`は要素と集約の型、`op`は結合的な集約、`e()`は単位元です。`F`は区間への作用、`mapping`は作用の適用、`composition(f,g)`はgの後にfを適用し、`id()`は何もしない作用を返します。作用は集約に分配する必要があります。非可換な集約も扱えます。`Commutative=true`は可換性を指定し、既定の最小値・和では自動判定します。

以下、Nは現在の要素数、Pは保持するノード枠数、MはID索引の枠数、kは削除要素数です。S/Fのコピー・演算・述語をO(1)として記します。木の高さは期待O(log N)、最悪O(N)です。

## API

| 操作 | 動作・計算量 |
| --- | --- |
| `lazy_implicit_treap()` / `(identity)` | 空列をO(1)で作る。後者は単位元を指定する。 |
| `lazy_implicit_treap(values[,identity])` | vectorからO(N)時間・追加O(N)領域で構築。 |
| `identity()` / `size()` / `empty()` | 単位元・要素数・空判定。O(1)。 |
| `get(i)` / `set(i,x)` / `front()` / `back()` | 要素の取得・置換。期待O(log N)。 |
| `insert_at(i,x)` / `push_front(x)` / `push_back(x)` | 挿入。償却期待O(log N)、ノード再確保時O(P)。 |
| `erase_at(i)` / `pop_front()` / `pop_back()` | 一要素削除。期待O(log N)。popは空なら無操作。 |
| `erase(l,r)` / `erase_prod(l,r)` | 区間削除／集約を返して削除。期待O(k+log N)。 |
| `prod(l,r)` / `all_prod()` | 順序を保つ区間集約／全体集約。期待O(log N)／O(1)。空は単位元。 |
| `apply(l,r,f)` | 区間へ作用。期待O(log N)。 |
| `reverse(l,r)` / `rotate(l,m,r)` | 反転／区間[l,m),[m,r)の交換。期待O(log N)。全体反転はO(1)。 |
| `move_range(l,r,pos)` | 移動前の隙間posへ区間を移す。期待O(log N)。l<=pos<=rなら無操作。 |
| `to_vector()` | 現在の列を返す。O(N)時間・領域。 |
| `lower_bound(x)` / `upper_bound(x)` | 昇順列でx以上／x超の最初の添字。なければN。期待O(log N)。 |
| `get_rank(x)` / `count(x)` / `contains(x)` | 昇順列で小さい要素数／等価要素数／存在判定。期待O(log N)。 |
| `insert_sorted(x)` / `erase_val(x)` / `erase_max()` | 昇順を保つ挿入／一致する一要素削除／最大値削除。挿入は償却、他は期待O(log N)。 |
| `max_right(l,g)` / `min_left(r,g)` | 集約がgを満たす右端／左端。関数テンプレート形式もある。期待O(log N)。 |
| `reserve(n)` / `clear()` | ノード容量確保／全削除。再確保時O(P)／O(P+M)、容量は保持。 |
| コピー／move／`swap` | コピーO(P+M)、move・swapはO(1)。列とID索引を一緒に扱う。 |

添字は0始まり、区間は半開区間です。昇順用操作は列全体が`operator<`で昇順、境界探索は`g(identity())`が真で、区間を伸ばすと単調であることが前提です。これらを確認する全列走査はしません。`contains`・`erase_val`は`==`があればそれを使い、なければ`<`による同値を使います。

既定の`RangeAddMin`はint64_tの区間加算・最小値で、単位元は2^60です。`t(1000)`や`t(values,1000)`の丸括弧で変更します。従来の`t({7})`は1要素、`t{{}}`は空列です。空列は`t{}`と書くと明確です（`t({})`は従来から曖昧）。各要素と加算後の値は単位元以下にします。`RangeAddSum`は`S{sum,size}`の加算・和で、各要素に`size=1`を持たせます。全演算が型の範囲に収まることが前提です。

```cpp
using namespace sirkit;
lazy_implicit_treap<> a(std::vector<std::int64_t>{10, 20, 30}, 1000000);
a.insert_at(1, 5);           // [10,5,20,30]
a.apply(0, 2, 3);            // [13,8,20,30]
auto minimum = a.prod(0, 4); // 8
// 空区間では設定した1000000を返す。
a.move_range(1, 3, 4);       // [13,30,8,20]
```

## ID付き操作

`indexed_implicit_treap`は先頭8テンプレート引数を共有し、`TrackIds=true`にする別名です。IDは非負intで、生存要素間で一意とします。通常の挿入・vector構築はIDなしです。領域はO(P+M)で、大きく飛び飛びのIDは利用側で圧縮してください。

| 操作 | 動作・計算量 |
| --- | --- |
| `insert_at_with_id(pos,id,x)` / `insert_after(id,new_id,x)` / `insert_before(...)` | 位置／既存IDの直後・直前に挿入。償却期待O(log N)、ID索引拡張は別途必要。 |
| `position_of(id)` / `get_id(pos)` | IDの位置／位置のID。期待O(log N)。不在・匿名は-1。 |
| `contains_id(id)` | 存在判定。O(1)。 |
| `get_by_id(id)` / `set_by_id(id,x)` / `erase_id(id)` | 取得・置換・削除。期待O(log N)。eraseは不在ならfalse。 |
| `prod_between(a,b)` / `erase_between(a,b)` | 両IDを除く間を列順に集約／削除。期待O(log N)／O(k+log N)。 |
| `reserve_ids(n)` | ID索引を確保。追加枠数に比例し、再確保時はO(M)。 |

端点IDは逆順でも渡せ、同じID・隣接IDの間は単位元です。set・作用・並べ替えはIDを保持し、削除したIDは再利用できます。[ID付き使用例](../../Examples/implicit_treap_erase_between_example.cpp)では挿入と区間和・削除を示しています。

コピーは独立した列・ID索引を所有し、代入失敗時は元の列を保ちます。指定した単位元は不変値として保持し、コピー・move・clear後も設定を保ちます。move元は空列として再利用できます。ノードの空き枠は再利用し、clearも確保済み容量を保持します。

既定では添字・区間・必要なIDを低コストで検査します。`erase(l,r)`の不正区間は無操作、他の不正位置・空のfront/back・不存在IDへの参照は例外です。負の挿入ID・重複IDは拒否します。`SKIP_RUNTIME_CHECKS`時は有効な入力が前提ですが、不在へのposition_of/contains_id/erase_idの戻り値と容量保護は維持します。

Sのコピー・moveが例外を投げ得る場合、`erase_prod`は削除前に集約を取得します。投げない型では切り離した部分木の集約を使います。検証・詳しい測定は`Audit/`、独立参照は`Verify/current/tests/implicit_treap_*_test.cpp`に置いています。


## line-container

元文書: `Document/line_container.md`

# 直線の追加と最適値照会

`LineContainer<T, objective>`は直線`y=k*x+m`を追加し、指定した`x`での最小値または最大値を返します。傾きや照会順の制約はありません。`MinLineContainer<T>`と`MaxLineContainer<T>`を使うと目的を省略できます。[使用例](../../Examples/line_container_example.cpp)を参照してください。

直線を傾き順に保持し、最適になる区間の右端`p`を保存します。追加した直線と隣接する直線の交点を更新し、不要になった直線を除きます。整数で包絡が小さい場合は、隣接する直線から不要と判断できる追加の確保を省き、残す場合は計算した交点を再利用します。照会では`x<=p`となる最初の直線を探します。以下、hは包絡に保持する直線数です。領域はO(h)、追加は償却O(log h)、照会はO(log h)です。1回の追加で多数の直線を除く場合はO(h)かかります。

## API

| API | 動作 |
| --- | --- |
| `add(k,m)` | 直線を追加する。既存直線より常に劣るものや重複は包絡から除かれ得る。 |
| `query(x)` | 最適値をTで返す。非空が前提。 |
| `query_optional(x)` | 空ならnullopt、それ以外はqueryと同じ。 |
| `query_line(x)` | 選択した元係数の組`pair<T,T>{k,m}`を返す。非空が前提。値の評価は行わない。 |
| `query_line_optional(x)` | 空ならnullopt、それ以外はquery_lineと同じ。 |
| `query_with_line(x)` | 1回の探索で`Result{value,k,m}`を返す。非空が前提。 |
| `query_with_line_optional(x)` | 空ならnullopt、それ以外はquery_with_lineと同じ。 |
| `size()` / `empty()` | 保持する直線数・空判定。O(1)。追加回数とは異なる。 |
| `clear()` / `swap(other)` | 全削除 / 同じ型の包絡を交換する。O(h) / O(1)。 |
| `lc_div(a,b)` | 整数では床除算、浮動小数点では通常の除算。 |
| `lc_inf<T>()` | 整数ではTの最大値、浮動小数点では正の無限大。 |

同じ値を与える直線の選択は未規定です。`Result`は値と係数を所有し、コンテナ内の要素への参照を持ちません。値と直線が両方必要な場合に使います。

```cpp
MinLineContainer<std::int64_t> lines;
lines.add(2, 1);
lines.add(-1, 4);
auto result = lines.query_with_line(3);
// result.value == 1, result.k == -1, result.m == 4
```

## 小さい整数包絡の事前棄却

最小化でTが32bit以下、保持直線が256本以下なら、隣接直線の既存境界`p`を使って候補を先に比較します。候補が左の直線以上となるのを`p`で、右の直線より大きいのを`p+1`で確認できれば、整数の照会域全体で不要です。第2比較を厳密にすることで、同点の直線を保持する従来の状態も維持します。

係数差と境界の積はint64_tに収まり、交点の除算前に判定できます。この経路は標準C++のみで、128bit整数拡張の有無によらず使えます。64bit・最大化・浮動小数点では従来経路を使います。

64bit最小化では、包絡の傾きが降順なので、候補と隣接線の交点の分母は正になります。GCCでは小包絡の事前棄却にある2回の交点計算だけ、この順序を使って符号判定を省きます。差は符号なし整数で計算し、床除算・範囲制限・最小値境界の処理を維持します。大包絡・32bit・最大化・浮動小数点、および他のコンパイラでは従来の計算を使います。

## 数値と入力の前提

整数Tは64bit以下の符号付き型です。傾き・切片・問い合わせはTに収めます。交点は整数の床除算で求め、型の範囲へ制限した右端を保持します。係数差の符号付きオーバーフローを避けて計算し、型の最小値ちょうどで選択される直線も扱います。

整数の最適値は数学的な`k*x+m`を評価し、Tの範囲外なら`SKIP_RUNTIME_CHECKS`の有無によらず`overflow_error`を投げます。積だけがTを超えても、切片との相殺で最終値がTに収まれば返します。利用可能なGCC/Clangでは128bit整数拡張を使い、`SIRKIT_LINE_CONTAINER_FORCE_PORTABLE`で標準C++の代替計算へ切り替えられます。拡張型がない環境も代替経路を使います。`query_line`系は最適値の評価をしないため、この値域検査を行いません。

浮動Tでは有限の係数・問い合わせを渡します。交点や値には丸め誤差があり、厳密な実数の最適値を保証しません。値の表現範囲を超える計算は無限大やNaNになり得ます。整数版と異なり、浮動値の計算結果を範囲外例外へ変換しません。

検査有効時は非有限の浮動入力を`invalid_argument`、非optional照会の空表を`out_of_range`で拒否します。`SKIP_RUNTIME_CHECKS`はこれらを除外します。`lc_div`のゼロ除算は`domain_error`、整数の商がTに収まらない場合は`overflow_error`を常に投げます。

## 所有権と参照

包絡のノードは`std::multiset`が所有します。コピーは独立した包絡を持ち、ムーブ後の元も有効な状態で再利用できます。追加の確保失敗では元の包絡を保持します。コピー代入の例外保証は基底の`std::multiset`に従い、失敗時の元内容保持は保証しません。

公開の`begin()`・`end()`・`lower_bound(x)`は包絡の参照に使います。最小化でも`k,m`は元の符号で保存し、傾き順は最大化が昇順、最小化が降順です。整数の`p`は型の範囲へ制限されるため、真の交点そのものとは限りません。追加で削除された直線へのiteratorや参照は無効になります。

基底の`insert`・`erase`やmutable係数を直接変更すると、隣接交点との整合性が壊れます。更新には`add`と`clear`を使います。`insect(x,y)`は旧互換の内部交点更新で、利用者が通常操作として呼ぶ関数ではありません。

同梱の1D部品は2次元版と`SIRKIT_LINE_CONTAINER_DEFINED`を共有し、どちらを先に含めても重複定義しません。提出先は[Library Checker Line Add Get Min](https://judge.yosupo.jp/problem/line_add_get_min)です。最大化・浮動係数・追加API・整数端・所有権は独立ローカル検査で補います。出典とライセンスの調査根拠は[監査記録](../Provenance/provenance-line-container-min-2026-10-22.json)に置きます。

[独立補足試験](../../Verify/current/tests/line_container_test.cpp)では全登録直線の内積を参照に、全取得API・整数端・相殺・浮動値・所有権を照合します。


## line-container-2d

元文書: `Document/line_container_2d.md`

# 2次元の内積最適化

`LineContainer2D`は点を追加し、方向`(a,b)`に対する`a*x+b*y`の最大値・最小値と、その点を照会します。削除には対応しません。[使用例](../../Examples/line_container_2d_example.cpp)を参照してください。

点`(x,y)`を直線`y*t+x`として、最小・最大の2つの包絡に保存します。`a`が非零なら`t=b/a`から選択点を求め、元の係数で内積を評価します。軸方向は座標の極値を使います。包絡の選択点を固定容量でキャッシュし、範囲外では木の検索に戻ります。照会でキャッシュは変更しません。

以下、hは両包絡に保持する点数の合計です。保持領域はO(h)、追加は償却O(log h)、照会はO(log h)です。多数の点を包絡から除く1回の追加にはO(h)かかる場合があります。

## API

| API | 動作 |
| --- | --- |
| `LineContainer2D(inf=2^60)` / `infinity()` | 正のint64_tの制限値で空表を構築／設定値の取得。O(1)。 |
| `add(x,y)` | 有限の点を追加する。重複も登録件数に含める。 |
| `max_ld(a,b)` / `min_ld(a,b)` | 最大・最小の内積値を`long double`で返す。非空が前提。 |
| `max_ld(整数,整数)` / `min_ld(整数,整数)` | 型全体がint64_t範囲内の整数型では、型から保証できる有限性・比の検査を省く。計算は浮動小数点。 |
| `max_ll(a,b)` / `min_ll(a,b)` | int64_t係数の照会後、値を±infに制限し、0方向へ切り捨ててint64_tで返す。 |
| `max_rounded_ll(a,b)` / `min_rounded_ll(a,b)` | 同じ範囲制限後に最も近い整数へ丸める。半整数は0から遠い側。 |
| `argmax(a,b)` / `argmin(a,b)` | 選択した元の点を`pair<long double,long double>`で返す。非空が前提。 |
| `max_with_point(a,b)` / `min_with_point(a,b)` | 点を1回選択し、`Result{value,x,y}`を返す。係数はlong double、非空が前提。 |
| `max_optional(a,b)` / `min_optional(a,b)` | 空ならnullopt、それ以外は内積値を返す。 |
| `argmax_optional(a,b)` / `argmin_optional(a,b)` | 空ならnullopt、それ以外は選択点を返す。 |
| `size()` / `empty()` | 重複込みの追加点数・空判定。O(1)。有効な包絡点数とは異なる。 |
| `clear()` / `swap(other)` | 全削除して空にする / 全状態を交換する。O(h) / O(1)。 |

同点で選ばれる点は規定しません。`Result`は3つの値を所有し、表の要素への参照を持ちません。内積値と選択点が両方必要な場合に使います。
`value`は返した点の内積です。浮動計算の順序により、別に呼んだ`max_ld`・`min_ld`とビット単位で一致する保証はありません。

```cpp
LineContainer2D points(1000000); // ll版を±1000000に制限。
points.add(1, 2);
points.add(3, -1);
auto result = points.max_with_point(2, 1);
// result.value == 5, result.x == 3, result.y == -1
```

## 数値と入力の前提

座標と浮動係数は有限値が前提です。型本来の`long double`精度と既定の丸めモードで使います。内積・交点には丸め誤差があり、整数引数やll版でも厳密な整数内積を保証しません。丸め前に精度を失った値は、rounded版でも復元できません。ゼロの符号は規定しません。

非空の照会で、有限な非零`a`に対して`b/a`が表現範囲を超える場合は、`SKIP_RUNTIME_CHECKS`の有無によらず`overflow_error`を投げます。内積が浮動小数点範囲を超える場合は無限大になり得ます。巨大な有限積同士の相殺でNaNになった場合は、指数を調整して再評価します。ll版は内積を±infへ制限するため、範囲外の数学的な値をそのまま返すAPIではありません。

検査有効時は非有限の入力を拒否し、非optional照会の空表に`out_of_range`を投げます。`SKIP_RUNTIME_CHECKS`を定義すると、これらの入力検査を除外します。追加点数の上限、比の表現範囲、相殺の回復は維持します。`SIRKIT_LINE_CONTAINER_FORCE_PORTABLE`は同梱1D部品の整数演算の標準代替経路を指定するもので、2D演算を厳密整数へ切り替える設定ではありません。

## 所有権と公開メンバー

コピーはO(h)で独立した包絡を持ち、内部の選択点参照も組み直します。ムーブはO(1)で、元は空として再利用できます。コピー・move・clearでもinfを保持し、swapでは設定も交換します。自己代入は内容を保持します。追加途中の確保失敗では、両包絡と登録件数を空に戻して例外を伝えます。コピー代入は一時コピーが完成してから交換します。

公開の`minlc`・`maxlc`・`Xmin`・`Xmax`・`Ymin`・`Ymax`は読み取り用です。包絡の直線は元点の`k=y,m=x`を保持します。外から書き換えたり、包絡だけに追加・削除したりすると、件数・座標範囲・内部参照の不変条件が壊れます。

提出先は[Library Checker Line Add Get Min](https://judge.yosupo.jp/problem/line_add_get_min)です。直線`a*x+b`を点`(a,b)`、照会を方向`(x,1)`へ変換します。全方向・浮動係数・追加API・所有権はローカル検査で補います。出典とライセンスの調査根拠は[監査記録](../Provenance/provenance-line-container-2d-min-2026-10-22.json)に置きます。

[独立API試験](../../Verify/current/tests/line_container_2d_reuse_test.cpp)では全登録点の内積を参照とし、値・最適点・丸め・範囲制限・コピー・ムーブを照合します。[確保失敗試験](../../Verify/current/tests/line_container_2d_allocation_test.cpp)では、追加失敗時の全消去とコピー代入失敗時の保持を確認します。


## binary-trie

元文書: `Document/binary_trie.md`

# 整数の二分トライ

[BinaryTrie](../../DataStructure/Binary_Trie.cpp)と[CompressedBinaryTrie](../../DataStructure/Compressed_Binary_Trie.cpp)は、非負整数の多重集合を扱います。全要素へのXOR、順位、XOR値の最小・最大を照会できます。[通常版の使用例](../../Examples/binary_trie_example.cpp)と[圧縮版の使用例](../../Examples/compressed_binary_trie_example.cpp)を参照してください。

## 型と範囲

`T`は`bool`・`const`・`volatile`を除く整数型です。`MAX_LOG`は最高位ビット番号で、B=`MAX_LOG+1`とします。キーとXORマスクは`0 <= x < 2^B`の範囲とし、符号付きTでは非負の範囲に収めます。例えば`std::uint32_t,29`は30bit、`std::uint64_t,63`は64bitすべてを使えます。

`MAX_LOG`は省略でき、既定値は`std::numeric_limits<T>::digits - 1`です。両クラスとも、符号なし型では全ビット、符号付き型では符号ビットを除く非負の範囲を扱います。第2テンプレート引数による手動指定もできます。

```cpp
BinaryTrie<std::uint32_t> full;                  // 32bit（MAX_LOG=31）。
BinaryTrie<std::uint32_t, 29> limited;           // 手動で30bitに限定。
CompressedBinaryTrie<std::uint64_t> compressed; // 64bit（MAX_LOG=63）。
CompressedBinaryTrie<std::uint64_t, 29> small;   // 手動で30bitに限定。
```

通常版は最大2bitずつ木をたどります。値の範囲が分かっている場合は`MAX_LOG`を小さく指定すると、探索する段数と必要なノード数を抑えられます。

`NODES`は根を含む未圧縮の二分トライのノード上限です。圧縮版も同じ制限を使い、圧縮した実ノードの個数だけで判定するものではありません。どちらもコピー・ムーブはできません。ノードとID配列はライブラリが所有し、再確保や破棄に利用者の処理は不要です。

## 操作

以下は両方に共通します。`size_type`は`std::size_t`、ID一覧は`std::vector<int>`への読み取り専用参照です。

| API | 結果 |
| --- | --- |
| `size()` / `empty()` | 重複込みの要素数 / 空か |
| `add(x,id=-1)` | xを1個追加する |
| `insert_unique(x,id=-1)` | 未登録時だけ追加してtrue。登録済みなら個数・IDを変えずfalse |
| `del(x,id=-1)` | 1個削除してtrue。値や指定IDがなければfalse |
| `find(x)` | `{xの個数, xのID一覧}`。未登録なら`{0, 空配列}` |
| `count(x)` / `contains(x)` | xの個数 / 存在 |
| `min_element()` / `max_element()` | `{最小値/最大値, その値のID一覧}`。空でないこと |
| `get_kth(std::int64_t k)` | 重複込み・1始まりの`{k番目の値, ID一覧}`。`1 <= k <= size()` |
| `operate_xor(x)` | 全要素にXORを適用する。以後`add(y)`で追加する公開値はy |
| `count_less(upper)` | upper未満の要素数。整数型のupperが0以下なら0、キー範囲を超えればsize() |
| `min_xor(x)` / `max_xor(x)` | `{x ^ yの最小値/最大値, 対応するyのID一覧}`。空でないこと |
| `count_xor_less(x,upper)` | `x ^ y < upper`となる要素数。upperの扱いはcount_lessと同じ |

IDは重複でき、`-1`は匿名要素を表します。匿名要素はID一覧に含めません。ID省略の削除は匿名要素を優先し、匿名がなければ任意のID付き要素を1個削除します。一覧の順序は規定せず、返された参照は次の更新または破棄まで有効です。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)です。指定IDの削除には、その値のID数に比例する探索も加わります。通常版の追加は償却O(B)、圧縮版の追加は償却O(B)で、ノード配列の再配置時には保持ノード数に比例する処理も発生します。ID追加は配列の拡張を伴うことがあります。

## 記憶領域と圧縮

通常版の内部ノードは最大2bit・4分岐をまとめて保持します。Bが奇数なら最上段だけ1bitで、以降は2bitずつです。昇順・XOR順の照会は、各桁の遅延XORを反映した子の順序で進みます。順位と個数には各子の要素数を使い、重複・IDの扱いは変わりません。要素が1個だけなら最小値・最大値は同じなので、物理キーの経路を復元した後で照会マスクをXORします。

最初の最大2048ノードを構築時に確保し、その後は最大8192ノードずつ増やします。 新しい枝のノードは空きノードを先に再利用し、残りを未使用ブロックから1ノードずつ取得します。必要なノードとIDの確保が成功するまで既存の親へ接続せず、失敗時は取得した部分だけを戻します。削除で空いたノードは再利用し、ブロックの拡張でもノードとID枠のアドレスを保ちます。`NODES`は物理的な4分岐ノードの数ではなく、各桁を元の二分木へ展開したときの根込みノード数で制限します。同じ上位1bitを共有する2本の枝を数え分け、追加成功後にだけこの個数と要素数を更新します。

圧縮版は値が分かれるビットだけを内点に持ちます。非空でD種類の値を持つ場合、生存する実ノードは`2D-1`個です。確保済みの領域は削除で縮めず再利用するため、保持量は過去の確保量にも依存します。`reserve(n)`は異なる値n個分の圧縮ノード領域を上限内で予約します。再配置時は保持ノード数に比例し、未圧縮の`NODES`制限は緩和しません。

圧縮版の内点のキーは、分岐ビットより上の共通接頭辞を表します。保存時のキーは公開値と遅延XORのXORです。検索や削除では、まず根の共通接頭辞と比較し、不一致なら子をたどらず不存在を返します。代表として記録した値が削除されても、内点が残る限り、その上位接頭辞は残る全要素に共通です。葉の負の分岐番号ではシフトせず、64bit最上位の分岐でも型幅以上のシフトを使いません。

## 検査と失敗

既定では負のキー、キー範囲外、空集合の最小・最大照会、不正な順位で例外を投げます。`SKIP_RUNTIME_CHECKS`を定義するとこれらの入力検査を除き、利用者が前提を満たす必要があります。ノード容量・添字・要素数のオーバーフロー保護は維持します。

追加に失敗した場合も値・個数・IDを保持しますが、確保済み容量が増える場合はあります。存在しない値やIDの削除はfalseを返し、内容を変えません。

提出先は[Library Checker Set Xor-Min](https://judge.yosupo.jp/problem/set_xor_min)です。重複・ID・遅延XOR・順位・容量境界・確保失敗は[多重集合参照](../../Verify/current/tests/binary_trie_reference_test.cpp)、[所有権と確保失敗](../../Verify/current/tests/binary_trie_allocation_test.cpp)、[圧縮接頭辞の境界](../../Verify/current/tests/compressed_binary_trie_prefix_test.cpp)で補います。測定条件と検証・出典の記録は[Audit](../)に置きます。

通常版の[桁・容量境界の試験](../../Verify/current/tests/binary_trie_stride_boundary_test.cpp)は、小さいビット幅の全列挙、全XOR、64bit最上位、二分prefix集合による容量の独立計算、ID参照を照合します。[確保失敗の補足](../../Verify/current/tests/binary_trie_stride_exception_test.cpp)はノード・ID枠のブロック境界も含めて状態保持と再利用を確認します。[単一要素の試験](../../Verify/current/tests/binary_trie_sole_test.cpp)は、XORと重複・削除を挟んだ最小値・最大値を確認します。

[ブロック境界の失敗試験](../../Verify/current/tests/binary_trie_suffix_exception_test.cpp)は、空きノード・ブロック末尾・新規ブロックを一度の追加でまたぎ、最後のID確保が失敗する場合も含めて全公開操作の状態を照合します。


## lazy-segment-tree

元文書: `Document/lazy_segment_tree.md`

# Lazy Segment Tree

`lazy_segtree<S,op,e,F,mapping,composition,id>`は、配列の区間をまとめて更新し、区間和・最小値などを求めます。添字は0始まり、区間は左端を含み右端を含まない`[l,r)`です。[加算・区間和の使用例](../../Examples/lazy_segment_tree_add_sum_example.cpp)、[代入も扱う使用例](../../Examples/lazy_segment_tree_example.cpp)を参照してください。

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

型引数と基本操作は[ACLのAPI](https://atcoder.github.io/ac-library/production/document_ja/lazysegtree.html)に合わせています。数学的参考は[モノイド設計](https://info.atcoder.jp/entry/algorithm_lectures/range_query_monoid_design)、実装は本ライブラリ用のCC0です。検証記録は[Audit](../)に置きます。


## pages/binary-trie

元文書: `Document/pages/binary_trie.md`

# 整数の二分トライ

`Binary_Trie.cpp`は`BinaryTrie<T,MAX_LOG,NODES>`を提供します。非負整数の多重集合に、値の追加・削除、順位、全体XOR、XOR値の最小・最大を照会できます。このファイルには圧縮版の`CompressedBinaryTrie`は含まれません。

## 使用例

```cpp
BinaryTrie<std::uint32_t, 29> values;
values.add(7);
values.add(3);
values.add(7);
const auto count = values.count(7);      // 2。
const auto third = values.get_kth(3).first; // 7。順位は1始まり。
values.operate_xor(2);
const auto nearest = values.min_xor(0).first; // min(5,1,5)=1。
```

## 前提と計算量

`T`は`bool`・`const`・`volatile`を除く整数型です。`B=MAX_LOG+1`として、キーとXORマスクは`0 <= x < 2^B`、符号付き型では非負の範囲を扱います。`MAX_LOG`省略時は`std::numeric_limits<T>::digits-1`です。空集合の最小・最大照会はできません。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)、追加は償却O(B)です。指定IDを削除する場合は、その値のID数に比例する探索も加わります。`NODES`は内部でまとめた分岐数ではなく、二分木に展開した根込みのノード上限です。

値・ノード・ID一覧はライブラリが所有します。コピーとムーブはできません。IDを指定しない要素は匿名で、`find`等から返るID一覧に含まれません。返したID一覧への参照は次の更新または破棄まで有効です。通常版には圧縮版専用の`reserve`はありません。

既定の入力検査は`SKIP_RUNTIME_CHECKS`で除けますが、容量・添字・要素数の保護は残ります。追加失敗時は値・個数・IDを保ちます。全APIの型と戻り値はこのページのソース由来の一覧、詳しい共通契約と方式の比較は[二分トライの説明](../../binary_trie.md)、完全な例は[通常版の使用例](../../../Examples/binary_trie_example.cpp)を参照してください。


## pages/compressed-binary-trie

元文書: `Document/pages/compressed_binary_trie.md`

# 整数の圧縮二分トライ

`Compressed_Binary_Trie.cpp`は`CompressedBinaryTrie<T,MAX_LOG,NODES>`を提供します。値が分かれるビットだけを保持する圧縮木で、非負整数の多重集合・順位・全体XOR・XOR値の最小と最大を扱います。このファイルに通常版の`BinaryTrie`は含まれません。

## 使用例

```cpp
CompressedBinaryTrie<std::uint32_t, 29> values;
values.reserve(100); // 異なる値100個分の領域を予約。
values.add(7);
values.add(3);
values.add(7);
const auto count = values.count(7);        // 2。
const auto smallest = values.get_kth(1).first; // 3。
values.operate_xor(2);
const auto nearest = values.min_xor(0).first; // 1。
```

## 前提と計算量

`T`は`bool`・`const`・`volatile`を除く整数型です。`B=MAX_LOG+1`として、キーとXORマスクは`0 <= x < 2^B`、符号付き型では非負の範囲です。`MAX_LOG`省略時は`std::numeric_limits<T>::digits-1`です。順位は重複込み・1始まりで、空集合の最小・最大照会はできません。

`size`・`empty`・`operate_xor`はO(1)、検索・順位・XOR照会・削除はO(B)、追加は償却O(B)です。ノード配列の再配置時には保持ノード数に比例する処理が発生します。指定IDの削除は、その値のID数に比例する探索も必要です。

非空で異なる値がD個なら、生存する実ノードは`2D-1`個です。確保した領域は削除で縮めず再利用します。`reserve(n)`は異なる値n個分を予約し、再配置時は保持ノード数に比例します。`NODES`は圧縮後のノード数ではなく、未圧縮の二分木に展開した根込みのノード上限です。

領域とID一覧はライブラリが所有し、コピー・ムーブはできません。IDを省略した要素は匿名で、ID一覧には含めません。ID一覧への参照は次の更新または破棄まで有効です。追加に失敗しても値・個数・IDを保ちます。

既定の入力検査は`SKIP_RUNTIME_CHECKS`で除けますが、容量・オーバーフローの保護は残ります。全APIの型と戻り値はこのページのソース由来の一覧、共通契約と方式の比較は[二分トライの説明](../../binary_trie.md)、完全な例は[圧縮版の使用例](../../../Examples/compressed_binary_trie_example.cpp)を参照してください。


## dynamic-union-find

元文書: `Document/dynamic_union_find.md`

# 動的Union Find

`int`全域のキーを、使った時点で登録するUnion Findです。負の値や`INT_MIN`・`INT_MAX`も使えます。連続した添字への変換は利用側で用意する必要がありません。

## APIと前提

- `DynamicUnionFind()`：空で構築します。`DynamicUnionFind(n)`は空のままn要素分を予約します。
- `add(k)`：単独成分として登録し、新規登録ならtrueを返します。
- `find(k)`：代表キーを返します。`same(x, y)`は同じ成分かを返します。
- `unite(x, y)`：サイズで併合し、併合したとき1、既に同じ成分なら0を返します。
- `unite(x, y, f)`：併合したときだけ`f(新しい代表キー, 吸収した代表キー)`を呼びます。コールバックの例外でも併合は完了したままです。
- `size(k)`：所属成分の要素数を返します。
- `data(k)`：根なら負のサイズ、非根なら親のキーを返します。負の親キーと区別する判定には使えません。
- `contains(k)`：登録済みかを返します。要素を追加しません。
- `node_count()` / `component_count()` / `empty()`：登録数・成分数・空かどうかを返します。
- `groups()`：全成分のキー一覧を返します。成分間の順序は未規定です。
- `reserve(n)`：登録状態を変えずに容量を予約します。`clear()`は登録を消し、確保済み容量を再利用します。

`find`・`same`・`size`・`data`・`unite`は、未知のキーを単独成分として自動登録します。登録数は`INT_MAX`以下です。内部の容量上限は`SKIP_RUNTIME_CHECKS`でも維持します。

## 方式と計算量

ハッシュ表でキーを連続した内部添字へ変換し、キーと親・サイズを連続配列に保持します。親はキーではなく添字なので、負のキーを根のサイズと取り違えません。サイズによる併合と経路の短縮を使います。

GCCでは、未登録のときだけ値を生成する共通ハッシュ表の`find_or_emplace`を使い、登録時の探索の重複を減らします。成分の比較にも内部添字を使います。`groups()`の成分番号は内部添字と同じ幅で持ち、必ず代入してから読む領域のゼロ初期化を省きます。32要素以下では固定長の作業領域を使い、一時的なヒープ確保も省きます。Clangなどは従来の探索・作業領域を使います。どちらも標準C++23で実装しています。

`groups()`は、全要素が別成分なら各キーの単独一覧を直接作ります。全要素が1成分なら登録順のキー一覧を作り、戻り値の領域を全て予約してから代表を1回取得し、全ての非根を代表へ直結します。代表キーと成分サイズは保ちます。一般の成分構成では上記の作業領域を使う方式を維持します。

ハッシュ検索は期待`O(1)`、併合・探索は償却`O(α(N))`です。`contains`は期待`O(1)`、件数取得は`O(1)`、`groups`は`O(N α(N))`、全独立または全連結なら`O(N+1)`です。使用領域は登録数と予約容量に比例します。

コピーは独立した複製を作り、コピー代入の確保失敗時は元の内容を維持します。move元は空になり、再利用できます。1キーの登録に失敗したとき、そのキーは登録されず、既存の連結関係は保ちます。複数キーを扱う操作では、それまでに成功した登録は残ります。`groups`の確保失敗時も登録と連結関係は保ちます。成功した`groups`は親を短縮するため、`data(k)`の返す親キーが変わる場合があります。

## 使用例とVerify

- [使用例](../../Examples/dynamic_union_find_example.cpp)
- [Library Checker提出コード](../../Verify/current/submissions/dynamic_union_find.cpp)：Unionfind。通常のAPIを逐次呼び出します。

負のキー・整数端・各補助API・コールバック・所有権・確保失敗は、成分ラベルを全走査する独立参照でも照合します。[検証](../Verification/verify-dynamic-union-find-second-pass-2026-10-22.json)・[測定](../Performance/performance-dynamic-union-find-second-pass-2026-10-22.json)・[出典](../Provenance/provenance-dynamic-union-find-second-pass-2026-10-22.json)をそれぞれ記録しています。
