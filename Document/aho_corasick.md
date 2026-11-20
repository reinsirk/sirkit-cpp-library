# Aho–Corasick法

[AhoCorasick](../String/Aho_Corasick.cpp)は登録した複数の語を1本の文字列から同時に検索します。`AhoCorasick<>`は英小文字、`ByteAhoCorasick`は全256種類のバイト値、`AhoCorasick<Alphabet,First>`は連続したバイト範囲を扱います。[使用例](../Examples/aho_corasick_example.cpp)を参照してください。


以下、nは検索文の長さ、mは語長、Sは状態数、Pは登録語数、Rは一致数です。

## API

### AhoCorasick

```text
AhoCorasick();
AhoCorasick(const AhoCorasick&);
AhoCorasick(AhoCorasick&& other) noexcept;
```

空の未構築状態を作るか、コピーまたは所有権の移動で構築します。コピーは元の状態から独立します。

**制約:** 1<=Alphabet<=256、First+Alphabet<=256。

**計算量:** 空とmoveはO(1)、コピーは保持領域に比例。

### operator=

```text
AhoCorasick& operator=(AhoCorasick other) noexcept;
```

内容をコピー／移動して代入し、自身への参照を返します。

**制約:** 同じ型の個体。

**計算量:** 引数の構築と旧状態の破棄に比例。

### add

```text
int add(std::string_view pattern);
```

語を登録して追加順の0始まりIDを返します。空語・重複も別IDです。

**制約:** build前、語のバイトは指定アルファベット内。

**計算量:** 語長mと新状態数aにO(m+Alphabet*a)。

### build

```text
void build();
```

遷移・failure・出力リンクを確定します。再度呼び出しても状態は変わりません。

**制約:** build後はclearまでaddしません。

**計算量:** 状態数SにO(Alphabet*S)時間・O(S)作業領域、再呼出しO(1)。

### step

```text
int step(int state, unsigned char c) const;
```

1文字進んだ状態を返します。

**制約:** 0<=state<state_count()。 build済み、cは指定範囲内。

**計算量:** O(1)。

### failure

```text
int failure(int state) const;
```

最長の真の接尾辞に対応する状態を返します。

**制約:** 0<=state<state_count()。 build済み。

**計算量:** O(1)。

### output_count

```text
std::uint64_t output_count(int state) const;
```

その状態で一致する登録語数を返します。

**制約:** 0<=state<state_count()。 build済み。

**計算量:** O(1)。

### parent

```text
int parent(int state) const;
```

元Trieの親状態を返します。根の親は0で、build前も使えます。

**制約:** 0<=state<state_count()。

**計算量:** O(1)。

### count_total

```text
std::uint64_t count_total(std::string_view text) const;
```

重なり・重複を含む総一致数を返します。空語は全境界に一致します。

**制約:** build済みで、textのバイトは指定アルファベット内。 総数がuint64_tを超えると常に例外。

**計算量:** text長nにO(n+1)。

### match_counts

```text
std::vector<std::uint64_t> match_counts(std::string_view text) const;
```

各登録IDの一致数を返します。

**制約:** build済みで、textのバイトは指定アルファベット内。

**計算量:** text長n・状態数S・語数PにO(n+S+P+1)時間・O(S+P+1)領域。

### for_each_match

```text
template <class Callback> void for_each_match(std::string_view text, Callback&& callback) const;
```

一致ごとにcallback(id,begin,end)を呼びます。区間は半開区間です。

**制約:** build済みで、textのバイトは指定アルファベット内。 callbackから構造を変更しないこと。

**計算量:** 一致数RにO(n+R+1)とcallbackの費用。

### find_all

```text
std::vector<AhoMatch> find_all(std::string_view text) const;
```

AhoMatch{pattern,begin,end}のvectorを返します。

**制約:** build済みで、textのバイトは指定アルファベット内。

**計算量:** O(n+R+1)時間・O(R)返却領域。

### size

```text
std::size_t size() const noexcept;
```

登録語数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

登録語がなければtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### state_count

```text
int state_count() const noexcept;
```

根を含む状態数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### node_capacity

```text
std::size_t node_capacity() const noexcept;
```

状態用の確保枠数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### pattern_length

```text
std::size_t pattern_length(int id) const;
```

登録語の長さを返します。

**制約:** 0<=id<size()。

**計算量:** O(1)。

### pattern_state

```text
int pattern_state(int id) const;
```

登録語の終端状態を返します。

**制約:** 0<=id<size()。

**計算量:** O(1)。

### reserve_nodes

```text
void reserve_nodes(std::size_t n);
```

状態の確保枠を予約します。

**制約:** 状態番号・容量の上限以下。

**計算量:** 再確保時は状態数SにO(Alphabet*S)、確保済みならO(1)。

### reserve_patterns

```text
void reserve_patterns(std::size_t n);
```

登録語記録の確保枠を予約します。

**制約:** 語ID・容量の上限以下。

**計算量:** 再確保時O(P)、確保済みならO(1)。

### clear

```text
void clear() noexcept;
```

空の未構築状態へ戻し、確保枠を保ちます。

**制約:** 取得済みID・状態番号を再利用しないこと。

**計算量:** 保持する状態・語・構築用配列の長さに比例する上界。

### swap

```text
void swap(AhoCorasick& other) noexcept;
```

全状態と確保枠を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### AhoMatch::operator==

```text
bool operator==(const AhoMatch&, const AhoMatch&);
```

pattern・begin・endが全て同じならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。


```cpp
AhoCorasick<> automaton;
automaton.add("he");
automaton.add("she");
automaton.build();
auto total = automaton.count_total("shehe"); // 3。
auto counts = automaton.match_counts("shehe"); // {2,1}。
```

全遷移を状態ごとに持つため、構築時間・領域は状態数×Alphabetに比例します。for_each_matchとfind_allの時間はtext長と一致数に比例します。終了位置の昇順で通知し、同じ位置内のID順は保証しません。空語は開始前も含む全境界に一致します。

コピーは独立し、move元は空の未構築状態になります。clearは確保枠を保ちます。addで領域の確保に失敗したときや不正文字が見つかったときは、未登録の接頭ノードが残る場合がありますが、登録済みの語は保ちます。buildで領域の確保に失敗した場合は未構築のままで、再試行できます。

`SKIP_RUNTIME_CHECKS`は添字・文字範囲・構築状態の検査を除き、容量と総一致数の上限は維持します。Verifyは[Library Checker](https://judge.yosupo.jp/problem/aho_corasick)です。
