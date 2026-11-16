# DynamicBitset

`DynamicBitset`は、実行時に長さを決められるbit列です。多数の真偽値を64bitずつまとめて扱い、集合演算や部分和DPに使えます。bit番号0が最下位です。[使用例](../Examples/dynamic_bitset_example.cpp)を参照してください。

以下、Nはbit数です。referenceは1 bitへの代入用proxyで、spanやproxyはbitsetを所有しません。

## コンストラクタ

```text
explicit DynamicBitset(std::size_t n = 0, bool value = false);
explicit DynamicBitset(std::string_view text);
DynamicBitset(const DynamicBitset&) = default;
DynamicBitset(DynamicBitset&& other) noexcept;
```

n bitを同じvalueにするか、高位からの01文字列で構築します。先頭0も長さに含みます。

**制約:** 冒頭の値型・演算の前提を満たすこと。指定する列長は内部容量の上限内です。

**計算量:** N bitにO(1+ceil(N/64))時間・領域。文字列版はO(N)時間。 ムーブ構築はO(1)、コピーは保持領域に比例。

## test

```text
bool test(std::size_t pos) const;
```

指定bitをboolで返します。

**制約:** 位置を指定する形は0<=pos<size()。proxyの寿命は所有・参照の説明に従います。

**計算量:** O(1)。

## operator[]

```text
bool operator[](std::size_t pos) const;
reference operator[](std::size_t pos);
```

const版はbool、非const版は代入可能なreference proxyを返します。

**制約:** 位置を指定する形は0<=pos<size()。proxyの寿命は所有・参照の説明に従います。

**計算量:** O(1)。

## set

```text
DynamicBitset& set() noexcept;
DynamicBitset& set(std::size_t pos, bool value = true);
```

全bitを1、または指定bitをvalueへ設定し、自身への参照を返します。

**制約:** 位置を指定する形は0<=pos<size()。proxyの寿命は所有・参照の説明に従います。

**計算量:** 1 bit形はO(1)、全bit形はO(1+ceil(N/64))。

## reset

```text
DynamicBitset& reset() noexcept;
DynamicBitset& reset(std::size_t pos);
```

全bitまたは指定bitを0へ戻し、自身への参照を返します。

**制約:** 位置を指定する形は0<=pos<size()。proxyの寿命は所有・参照の説明に従います。

**計算量:** 1 bit形はO(1)、全bit形はO(1+ceil(N/64))。

## flip

```text
DynamicBitset& flip() noexcept;
DynamicBitset& flip(std::size_t pos);
```

全bitまたは指定bitを反転し、自身への参照を返します。

**制約:** 位置を指定する形は0<=pos<size()。proxyの寿命は所有・参照の説明に従います。

**計算量:** 1 bit形はO(1)、全bit形はO(1+ceil(N/64))。

## all

```text
bool all() const noexcept;
```

全bitが1ならtrueを返します。空列でもtrueです。

**制約:** 追加の入力制約はありません。

**計算量:** 最悪O(1+ceil(N/64))。

## any

```text
bool any() const noexcept;
```

1のbitがあればtrueを返します。

**制約:** 追加の入力制約はありません。

**計算量:** 最悪O(1+ceil(N/64))。

## none

```text
bool none() const noexcept;
```

1のbitがなければtrueを返します。空列でもtrueです。

**制約:** 追加の入力制約はありません。

**計算量:** 最悪O(1+ceil(N/64))。

## count

```text
std::size_t count() const noexcept;
std::size_t count(std::size_t l, std::size_t r) const;
```

全体または半開区間[l,r)にある1の数を返します。

**制約:** 区間版は0<=l<=r<=size()。

**計算量:** 全体はO(1+ceil(N/64))、区間版はO((r-l)/64+1)。

## intersection_count

```text
std::size_t intersection_count(const DynamicBitset& other) const;
```

同長のotherとのANDに含まれる1の数を返します。一時bitsetは作りません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## intersection_parity

```text
bool intersection_parity(const DynamicBitset& other) const;
```

otherとのANDの1の数が奇数ならtrueを返します。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator&

```text
friend DynamicBitset operator&(DynamicBitset a, const DynamicBitset& b);
```

ANDを行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator|

```text
friend DynamicBitset operator|(DynamicBitset a, const DynamicBitset& b);
```

ORを行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator^

```text
friend DynamicBitset operator^(DynamicBitset a, const DynamicBitset& b);
```

XORを行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator~

```text
friend DynamicBitset operator~(DynamicBitset value);
```

全bit反転を行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## operator<<

```text
friend DynamicBitset operator<<(DynamicBitset value, std::size_t shift);
```

左への論理シフトを行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## operator>>

```text
friend DynamicBitset operator>>(DynamicBitset value, std::size_t shift);
```

右への論理シフトを行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## shift_or

```text
DynamicBitset& shift_or(std::size_t shift) noexcept;
```

元の各bitを1回だけ使い、`bits |= bits << shift`を適用します。shiftが0または長さ以上なら不変です。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## find_first

```text
std::size_t find_first() const noexcept;
```

最初の1の位置を返します。なければsize()です。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## find_last

```text
std::size_t find_last() const noexcept;
```

最後の1の位置を返します。なければsize()です。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## find_next

```text
std::size_t find_next(std::size_t pos) const noexcept;
```

posより後の最初の1の位置を返します。pos>=size()または不在ならsize()です。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## find_prev

```text
std::size_t find_prev(std::size_t pos) const noexcept;
```

pos未満の最後の1の位置を返します。pos>size()はsize()へ丸め、不在ならsize()です。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## resize

```text
void resize(std::size_t n, bool value = false);
```

長さを変更し、追加bitだけvalueで埋めます。縮小で消えたbitは復活しません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 再確保時は保持word数、新領域の初期化は追加word数に比例。

## reserve_bits

```text
void reserve_bits(std::size_t n);
```

n bit分を事前予約します。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 再確保時は保持word数、新領域の初期化は追加word数に比例。

## size

```text
std::size_t size() const noexcept;
```

要素数を返します。

**制約:** 追加の入力制約はありません。

**計算量:** O(1)。

## empty

```text
bool empty() const noexcept;
```

空ならtrueを返します。

**制約:** 追加の入力制約はありません。

**計算量:** O(1)。

## clear

```text
void clear() noexcept;
```

0 bitへ戻します。

**制約:** 追加の入力制約はありません。

**計算量:** 保持word数に比例する上界。

## swap

```text
void swap(DynamicBitset& other) noexcept;
friend void swap(DynamicBitset& a, DynamicBitset& b) noexcept;
```

内容と設定を交換します。

**制約:** 追加の入力制約はありません。

**計算量:** O(1)。値型・比較器のswapの費用は別です。

## to_string

```text
std::string to_string() const;
```

高位から並ぶ01文字列を返します。

**制約:** 追加の入力制約はありません。

**計算量:** O(N)時間・領域。

## words

```text
std::span<const std::uint64_t> words() const noexcept;
```

下位word順の読み取り専用spanを返します。

**制約:** 追加の入力制約はありません。spanの有効期間は所有・参照の説明に従います。

**計算量:** O(1)。spanは配列を所有しません。

## operator&=

```text
DynamicBitset& operator&=(const DynamicBitset& other);
```

ANDを代入を行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator|=

```text
DynamicBitset& operator|=(const DynamicBitset& other);
```

ORを代入を行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator^=

```text
DynamicBitset& operator^=(const DynamicBitset& other);
```

XORを代入を行います。長さは変えません。

**制約:** otherと同じbit数であること。

**計算量:** 最悪O(1+ceil(N/64))。

## operator<<=

```text
DynamicBitset& operator<<=(std::size_t shift) noexcept;
```

左への論理シフトを代入を行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## operator>>=

```text
DynamicBitset& operator>>=(std::size_t shift) noexcept;
```

右への論理シフトを代入を行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## operator==

```text
bool operator==(const DynamicBitset& other) const noexcept;
```

長さと各bitの等値比較を行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

## operator!=

```text
bool operator!=(const DynamicBitset& other) const;
```

C++23のoperator==から得られる不等値比較です。

長さと各bitの不一致判定を行います。長さは変えません。

**制約:** bit番号・シフト量はstd::size_tです。各操作の境界動作は説明のとおりです。

**計算量:** 最悪O(1+ceil(N/64))。

```cpp
DynamicBitset reachable(10001);
reachable.set(0);
for (std::size_t score : {2,3,5}) reachable.shift_or(score);
auto ways = reachable.count(); // 7: 0,2,3,5,7,8,10。
```

shift_orは元の各bitを1回だけ利用するため、上の例では各点数を一度だけ選ぶ部分和になります。k=0や長さ以上なら不変です。通常のshiftはkが長さ以上なら全0になります。縮小で消えたbitは再拡大しても復活しません。==と!=は長さも含めて比較します。

## 前提・計算量

1 bit操作とsize・emptyはO(1)。全bit操作・集合演算・shift・探索・コピーは最悪O(1+ceil(N/64))、count(l,r)はO((r-l)/64+1)、文字列との変換はO(N)です。resize・reserveの再確保は保持word数に比例します。move・swapはO(1)です。

`auto bit=bits[pos]`は値ではなくproxyのコピーです。値を保存するなら`bool bit=bits[pos]`とします。proxyはbool変換・代入・flip・boolとの`&=`・`|=`・`^=`に対応します。サイズ変更・代入・所有権移動後は以前のproxyやword spanを使わないでください。末尾wordの未使用bitは常に0です。

find_next(pos>=size())はsize()、find_prev(pos>size())はposをsize()へ丸めて探索します。コピーは独立、コピー代入・resize・reserveの確保失敗時は旧内容を維持します。ムーブ元は空として再利用できます。

番号・区間・2項演算の同長・01文字の検査は既定で有効です。`SKIP_RUNTIME_CHECKS`でこれらを除きますが、容量保護と巨大shiftの処理は維持します。対応CPUではPOPCNT・AVX2を使い、`SIRKIT_DISABLE_SIMD`で追加の専用実装とCPU判定を除きます。

主Verifyは[Matrix Product (Mod 2)](https://judge.yosupo.jp/problem/matrix_product_mod_2)、AND個数は[ABC258 G](https://atcoder.jp/contests/abc258/tasks/abc258_g)、shift_orは[TDPC A](https://atcoder.jp/contests/tdpc/tasks/tdpc_contest)で補います。

## operator=

```text
DynamicBitset& operator=(const DynamicBitset& other);
DynamicBitset& operator=(DynamicBitset&& other) noexcept;
```

コピーまたはムーブで内容を置き換え、自身への参照を返します。コピーは独立した内容を持ちます。

**制約:** 型・所有・例外保証は共通の説明に従います。

**計算量:** 保持する配列・ノードのコピーと旧内容の破棄に比例。ムーブ構築とは異なり、代入では旧内容の解放費用も含みます。

## reference::operator bool

```text
operator bool() const noexcept;
```

参照するbitをboolで返します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::operator~

```text
bool operator~() const noexcept;
```

参照するbitの論理反転をboolで返します。bit自体は変えません。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::operator=

```text
reference& operator=(bool value) noexcept;
reference& operator=(const reference& other) noexcept;
```

参照するbitへbool、または別proxyの現在値を代入します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::flip

```text
reference& flip() noexcept;
```

参照するbitを反転し、自身への参照を返します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::operator&=

```text
reference& operator&=(bool value) noexcept;
```

参照するbitへvalueとのANDを代入します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::operator|=

```text
reference& operator|=(bool value) noexcept;
```

参照するbitへvalueとのORを代入します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。

## reference::operator^=

```text
reference& operator^=(bool value) noexcept;
```

参照するbitへvalueとのXORを代入します。

**制約:** 元のbitsetとbit位置が有効であること。サイズ変更・代入・所有権移動後のproxyは使えません。

**計算量:** O(1)。
