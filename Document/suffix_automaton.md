# Suffix Automaton

[SuffixAutomaton](../String/Suffix_Automaton.cpp)は文字列を末尾へ伸ばしながら、部分文字列の存在・出現回数・異なる個数・最長共通部分文字列を扱います。既定の`SuffixAutomaton<>`は英小文字、`SuffixAutomaton<256,0>`はNULを含む全バイトです。一般形は`<Alphabet,Base>`で、値を[Base,Base+Alphabet)に収めます。Unicodeの文字単位ではありません。[使用例](../Examples/suffix_automaton_example.cpp)を参照してください。


以下、nは現在の文字列長、mは追加列または照会列の長さ、statesは状態数です。

## API

### SuffixAutomaton

```text
SuffixAutomaton();
explicit SuffixAutomaton(std::string_view text);
SuffixAutomaton(const SuffixAutomaton&);
SuffixAutomaton(SuffixAutomaton&&other) noexcept;
```

空／指定文字列／独立コピー／所有権移動で構築します。

**制約:** 1<=Alphabet<=256、Base+Alphabet<=256、文字列は指定バイト範囲内。

**計算量:** 空とmoveはO(1)、文字列長mにO(Alphabet*m)、コピーは保持領域に比例。

### operator=

```text
SuffixAutomaton& operator=(const SuffixAutomaton&other);
SuffixAutomaton&operator=(SuffixAutomaton&&other)noexcept;
```

独立コピー／所有権移動で代入し、自身への参照を返します。

**制約:** 同じ型の個体。

**計算量:** コピーは新旧保持領域に比例。move代入は旧状態の破棄を含みます。

### append

```text
void append(std::uint8_t byte);
void append(std::string_view text);
```

末尾へバイト／列を追加します。

**制約:** 追加するバイトは[Base,Base+Alphabet)。番号・長さ・容量の上限以下。

**計算量:** 1バイトは償却O(Alphabet)、最悪O(n+Alphabet)。mバイトの追加はO(Alphabet*m)。

### size

```text
std::size_t size()const noexcept;
```

元の文字列長を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty()const noexcept;
```

空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### distinct_substring_count

```text
std::uint64_t distinct_substring_count()const noexcept;
```

空を除く異なる部分文字列の個数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### state_count

```text
std::size_t state_count()const noexcept;
```

保存する状態数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### contains

```text
bool contains(std::string_view pattern)const noexcept;
```

部分文字列として存在すればtrueです。空語も含みます。

**制約:** 範囲外バイトは不一致として扱います。

**計算量:** パターン長mにO(m+1)。

### occurrences

```text
std::uint64_t occurrences(std::string_view pattern)const;
```

重なりを含む出現回数を返します。空語はn+1です。

**制約:** 範囲外バイトは不一致として扱います。同じ個体の並行キャッシュ照会は外部同期が必要です。

**計算量:** append後の最初の一致する非空照会はO(n+states+m)、以後O(m+1)。

### longest_common_substring

```text
Match longest_common_substring(std::string_view text)const noexcept;
```

最長共通部分文字列のMatch{first_start,second_start,length}を返します。

**制約:** 引数の範囲外バイトは一致を区切ります。

**計算量:** 引数長mにO(m+1)。

### reserve

```text
void reserve(std::size_t text_capacity);
```

指定した文字列長で必要となる最大状態数と関連領域を予約します。

**制約:** 番号・容量の上限以下。

**計算量:** 再確保時は保持する状態・文字列・キャッシュ等の大きさに比例する上界。

### reset

```text
void reset(std::string_view text={});
```

指定した文字列へ置換します。省略すると空です。

**制約:** 追加するバイトは[Base,Base+Alphabet)。番号・長さ・容量の上限以下。

**計算量:** 旧状態の解放と新長mにO(Alphabet*m+旧保持量+1)の上界。

### clear

```text
void clear()noexcept;
```

空へ戻し、全領域を解放します。

**制約:** 既存の借用viewや状態番号を再利用しないこと。

**計算量:** O(n+1)。

### swap

```text
void swap(SuffixAutomaton&other)noexcept;
void swap(SuffixAutomaton&a,SuffixAutomaton&b)noexcept;
```

内容と所有領域を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


```cpp
SuffixAutomaton<> automaton("ababa");
auto distinct = automaton.distinct_substring_count(); // 9。
auto count = automaton.occurrences("aba"); // 2。
auto match = automaton.longest_common_substring("zzbabab");
// first_start=1、second_start=2、length=4（"baba"）。
```

最長共通部分文字列の位置は自分側と引数側の順です。同長なら引数で先に終わるものを選び、長さ0なら位置も0です。元の文字列自体は保持せず、状態から自分側の位置を復元します。範囲外バイトの検索は不一致、最長共通部分文字列ではその位置で一致を区切ります。

## 計算量・所有と検査

同じ終了位置を持つ部分文字列を状態へまとめ、必要な場合に状態を複製します。1バイト追加は償却O(Alphabet)、1回の最悪はO(n+Alphabet)です。固定Alphabetなら償却O(1)。状態数は高々2n+1、領域はO(Alphabet*states+n+1)で、空なら根も確保しません。

出現回数はappend後の最初の一致する非空照会でO(n+states+m)のキャッシュを作り、以後O(m)です。確保失敗では元を保ちます。const照会もキャッシュを更新するため、同じ個体への並行キャッシュ照会は外部で同期してください。

コピーは独立、コピー代入・resetは失敗時に元を保ちます。moveはnoexcept・O(1)、移動元は空で再利用可です。1バイト追加は強い例外保証、文字列追加も確保・容量超過では不変ですが、途中の不正バイトでは処理済みの接頭辞を保持します。検査だけの全走査はしません。

`SKIP_RUNTIME_CHECKS`は追加バイトの検査を除き、照会の不一致判定とint32_t状態番号・長さ・容量の保護は維持します。構成・機能の数学的参考は[NyaanのSuffix Automaton](https://nyaannyaan.github.io/library/string/suffix-automaton.hpp.html)で、固定幅配列と出現回数キャッシュは独立に実装しています。
