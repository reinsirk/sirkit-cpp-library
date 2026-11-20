# Mo / MoWithUpdates

[本体](../Algorithm/Mo.cpp)は、区間の問い合わせを並べ替えて、要素の追加・削除が少なくなる順に処理します。Moは変更のない配列、MoWithUpdatesは点変更を含む配列を扱います。値と集計状態は利用側で持ちます。[使用例](../Examples/mo_example.cpp)を参照してください。


以下、Nは配列長、Qは照会数、Uは変更数、Bは使用ブロック幅、K=1+floor(N/B)です。

## API

### Mo

```text
explicit Mo(std::int32_t n = 0);
```

長さnの静的な列への問い合わせを管理します。

**制約:** n>=0。

**計算量:** O(1)。

### MoWithUpdates

```text
explicit MoWithUpdates(std::int32_t n = 0);
```

点変更を含む長さnの列への問い合わせを管理します。

**制約:** n>=0。

**計算量:** O(1)。

### add_query

```text
std::size_t add_query(std::int32_t l, std::int32_t r);
// MoWithUpdatesのみ
std::size_t add_query(std::int32_t l, std::int32_t r, std::size_t time);
```

半開区間[l,r)を登録し、追加順の要求IDを返します。更新版の2引数形は現在の変更数を時刻にします。

**制約:** 0<=l<=r<=length()、指定時刻は記録済みの変更数以下。

**計算量:** 償却O(1)。

### add_update

```text
std::size_t add_update(std::int32_t pos); // MoWithUpdatesのみ
```

変更位置を登録して変更IDを返します。旧値・新値は利用側で保存します。

**制約:** 0<=pos<length()。

**計算量:** 償却O(1)。

### run（Mo）

```text
template<class Add, class Erase, class Answer>
Window run(Add&& add, Erase&& erase, Answer&& answer, std::int32_t b = 0) const;
template<class AddLeft, class AddRight, class EraseLeft, class EraseRight, class Answer>
Window run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left,
           EraseRight&& erase_right, Answer&& answer, std::int32_t b = 0) const;
```

区間を伸縮してanswer(要求ID)を呼び、最後のWindow{left,right}を返します。左右共通／別々のcallbackを選べます。

**制約:** 集計は空から開始し、callbackは後述の契約を満たすこと。b>=0、0は自動選択。

**計算量:** 並べ替えにO(Q log(Q+1)+log(N+Q+1))時間・O(Q)領域。区間移動O(QB+NK+Q)回とcallbackの費用。

### run（MoWithUpdates）

```text
template<class Add, class Erase, class Apply, class Undo, class Answer>
State run(Add&& add, Erase&& erase, Apply&& apply, Undo&& undo,
          Answer&& answer, std::int32_t b = 0) const;
template<class AddLeft, class AddRight, class EraseLeft, class EraseRight,
         class Apply, class Undo, class Answer>
State run(AddLeft&& add_left, AddRight&& add_right, EraseLeft&& erase_left,
          EraseRight&& erase_right, Apply&& apply, Undo&& undo, Answer&& answer,
          std::int32_t b = 0) const;
```

区間と時刻を動かし、最後のState{left,right,time}を返します。

**制約:** 集計は空、外部の値は時刻0。callbackは後述の契約を満たすこと。b>=0。

**計算量:** 並べ替えは静的版と同じ。移動O(QB+NK+UK²+Q)回とcallbackの費用。

### order

```text
std::vector<std::size_t> order(std::int32_t b = 0) const;
```

処理順の要求IDを返します。要求ID自体は変えません。

**制約:** b>=0。

**計算量:** O(Q log(Q+1)+log(N+Q+1))時間・O(Q)領域。

### block_size

```text
std::int32_t block_size(std::int32_t b = 0) const;
```

使う正のブロック幅を返します。0は自動選択です。

**制約:** b>=0。

**計算量:** 静的版O(log(Q+1))、更新版O(log(N+1))。

### length

```text
std::int32_t length() const noexcept;
```

配列長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### query_count

```text
std::size_t query_count() const noexcept;
```

問い合わせ数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### update_count

```text
std::size_t update_count() const noexcept; // MoWithUpdatesのみ
```

変更数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

問い合わせがなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### query

```text
Query query(std::size_t id) const;
```

Query{left,right[,time]}を値で返します。

**制約:** id<query_count()。

**計算量:** O(1)。

### queries

```text
std::span<const Query> queries() const noexcept;
```

問い合わせ記録の読み取り専用viewを返します。

**制約:** viewの寿命は後述の所有契約に従います。

**計算量:** O(1)。

### update_position

```text
std::int32_t update_position(std::size_t id) const; // MoWithUpdatesのみ
```

変更IDの位置を返します。

**制約:** id<update_count()。

**計算量:** O(1)。

### updates

```text
std::span<const std::int32_t> updates() const noexcept; // MoWithUpdatesのみ
```

変更位置の読み取り専用viewを返します。

**制約:** viewの寿命は後述の所有契約に従います。

**計算量:** O(1)。

### reserve_queries

```text
void reserve_queries(std::size_t cap);
```

問い合わせ記録の容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再確保時O(Q)、確保済みならO(1)。

### reserve_updates

```text
void reserve_updates(std::size_t cap); // MoWithUpdatesのみ
```

変更記録の容量を予約します。

**制約:** 容量上限以下。

**計算量:** 再確保時O(U)、確保済みならO(1)。

### clear_requests

```text
void clear_requests() noexcept;
```

長さを保ち全記録を消します。

**制約:** 取得済みIDとviewは無効になります。

**計算量:** O(Q+U+1)の上界。

### reset

```text
void reset(std::int32_t n = 0);
```

指定した長さで全記録を消して再初期化します。

**制約:** n>=0。

**計算量:** 旧記録の破棄を含めO(Q+U+1)の上界。

### clear

```text
void clear() noexcept;
```

長さ0・記録なしに戻します。

**制約:** 取得済みIDとviewは無効になります。

**計算量:** O(Q+U+1)の上界。

### swap

```text
void Mo::swap(Mo& other) noexcept;
void MoWithUpdates::swap(MoWithUpdates& other) noexcept;
```

長さと全記録の所有権を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


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
