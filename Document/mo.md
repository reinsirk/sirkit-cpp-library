# Mo / MoWithUpdates

[本体](../Algorithm/Mo.cpp)は、区間の問い合わせを並べ替えて、要素の追加・削除が少なくなる順に処理します。Moは変更のない配列、MoWithUpdatesは点変更を含む配列を扱います。値と集計状態は利用側で持ちます。[使用例](../Examples/mo_example.cpp)を参照してください。

## API

| API | 動作 |
| --- | --- |
| `Mo(n)` / `MoWithUpdates(n)` | 長さnの配列への問い合わせを管理する。既定n=0。 |
| `add_query(l,r)` | 半開区間[l,r)を登録し、追加順の要求IDを返す。更新版では現在の時刻を使う。 |
| `add_update(pos)` | 更新版専用。変更位置を登録し変更IDを返す。旧値・新値は利用側で保存する。 |
| `add_query(l,r,time)` | 更新版専用。記録済みの指定時刻での照会を登録する。 |
| `run(add,erase,answer,b=0)` | 要素の追加・削除で区間を動かし、answer(要求ID)で答えを保存する。 |
| `run(add_left,add_right,erase_left,erase_right,answer,b=0)` | 追加・削除を左右で分ける形。順序に依存する集計に使う。 |
| `order(b=0)` / `block_size(b=0)` | 処理順のID列 / 使用する正のブロック幅を返す。0は自動選択。 |
| `length()` / `query_count()` / `empty()` | 配列長 / 要求数 / 要求がないか。更新版はupdate_count()も使える。 |
| `query(id)` / `queries()` | Query{left,right[,time]}の値 / 読み取り専用span。更新版はupdate_position(id)・updates()も使える。 |
| `reserve_queries(cap)` / `reserve_updates(cap)` | 記録の容量を予約する。後者は更新版専用。 |
| `clear_requests()` / `reset(n=0)` / `clear()` | 長さを保ち記録を消す / 再初期化 / 長さ0へ戻す。 |

```cpp
std::vector<std::int64_t> a{2,7,1,8};
Mo mo(4);
auto q = mo.add_query(1,4);
std::int64_t sum = 0;
std::vector<std::int64_t> answers(mo.query_count());
mo.run([&](std::int32_t i) { sum += a[i]; },
       [&](std::int32_t i) { sum -= a[i]; },
       [&](std::size_t id) { answers[id] = sum; });
// answers[q] == 16。処理順が変わっても、回答は追加順のIDへ保存する。
```

## 更新版とcallback

時刻tは最初のt変更を適用した状態で、0が変更前です。更新版のrunは、伸縮callbackの後に`apply(change_id,left,right)`・`undo(change_id,left,right)`・answerを渡します。左右共通の形は`run(add,erase,apply,undo,answer,b=0)`です。変更対象が現在の区間内なら、古い寄与を除き、値を変更して新しい寄与を加えます。区間外なら値だけを変更します。

実行開始時の集計は空、更新版の配列は時刻0に戻しておきます。runは静的版でWindow{left,right}、更新版でState{left,right,time}を返し、外部の配列・集計はその状態のまま残ります。要求なしなら全フィールド0で、更新callbackも呼びません。再実行前には利用側で初期状態へ戻してください。

callbackはlvalueとして呼べる必要があり、保持・コピーはしません。実行中に本体を追加・clear・移動しないでください。作業領域はcallbackを呼ぶ前に確保します。callback例外では本体記録を維持しますが、外部状態は巻き戻しません。

## 計算量と前提

Nは配列長、Qは照会数、Uは変更数、Bはブロック幅、K=1+floor(N/B)です。並べ替えはO(Q log(Q+1)+log(N+Q+1))時間・O(Q)領域。sort以外の区間移動はO(QB+NK+Q)回で、更新版は時刻移動O(UK²)回が加わります。各callbackの費用は別途必要です。

自動幅は静的版がmax(1,N/max(1,floor(sqrt(Q))))、更新版がmax(1,floor(cbrt(N²)))で、N=0では1です。静的版は概ねO(N sqrt(Q)+Q)、更新版はN・Q・Uが同程度なら概ねO(N^(5/3))の移動を狙います。自動幅が常に最適とは限りません。正のbで手動指定できます。

長さ・端点・変更位置・幅はint32_t、要求ID・変更ID・時刻はsize_tです。`0<=l<=r<=N`を守り、空区間も使えます。IDは並べ替えで変わらず、記録を消すと無効です。記録追加は償却O(1)、件数・記録取得はO(1)、reserveは再確保時O(Q)／O(U)です。

コピーはO(Q+U)で独立、代入は失敗時に元内容を保持します。move・swapはO(1)、move元は長さ0で再利用できます。spanは正常な追加・容量変更・clear・代入・所有権移動まで有効で、確保失敗時は維持します。`SKIP_RUNTIME_CHECKS`で入力検査を省き、容量・自動幅・0除算回避の保護は維持します。

主Verifyは[Static Range Count Distinct](https://judge.yosupo.jp/problem/static_range_count_distinct)、更新版は[Point Set Range Frequency](https://judge.yosupo.jp/problem/point_set_range_frequency)です。並べ替えや例外・所有の詳しい検証は[Audit](../Audit/)に置きます。
