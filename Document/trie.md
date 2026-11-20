# バイト列の多重集合

`Trie`は文字列の重複個数、接頭辞、辞書順の照会を扱います。空文字列とNULを含む全256バイトに対応し、辞書順は符号なしバイトの順です。


以下、Lは入力語長、Kは返す語長、Nは保持ノード枠数、Tは保持索引枠数です。

## 登録・検索

### Trie

```text
Trie();
Trie(const Trie&);
Trie(Trie&& other) noexcept;
```

引数を省略すると空のTrieを作ります。コピーしたTrieは元のTrieから独立し、ムーブでは所有権を移します。

**制約:** 追加の制約はありません。

**計算量:** 空・moveはO(1)、コピーはO(N+256T)。

### operator=

```text
Trie& operator=(const Trie& other);
Trie& operator=(Trie&& other) noexcept;
```

独立コピー／所有権移動で代入し、自身への参照を返します。

**制約:** 同じ型の個体。

**計算量:** コピーは新旧の保持量に比例、move代入は旧状態の解放も含みます。

### insert

```text
std::size_t insert(std::string_view word, std::size_t copies = 1);
```

copies個追加してその語の個数を返します。copies=0なら照会のみ。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 償却O(16L+256)、再確保時は保持ノード・索引枠のコピーも加算。

### erase

```text
bool erase(std::string_view word, std::size_t copies = 1) noexcept;
```

指定個数を削除できればtrueを返します。個数が足りなければ何も変更せずfalseを返し、指定個数が0ならtrueを返します。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** O(16L+256)。

### erase_all

```text
std::size_t erase_all(std::string_view word) noexcept;
```

その語を全て削除し、削除した個数を返します。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** O(16L+256)。

### count

```text
std::size_t count(std::string_view word) const noexcept;
```

その語の個数を返します。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 語長LにO(16L+1)。

### contains

```text
bool contains(std::string_view word) const noexcept;
```

その語が1個以上あればtrueです。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 語長LにO(16L+1)。

### prefix_count

```text
std::size_t prefix_count(std::string_view prefix) const noexcept;
```

接頭辞を持つ語の総個数を返します。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 語長LにO(16L+1)。

### starts_with

```text
bool starts_with(std::string_view prefix) const noexcept;
```

接頭辞を持つ語が存在すればtrueです。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 語長LにO(16L+1)。

### longest_prefix

```text
std::optional<std::size_t> longest_prefix(std::string_view text) const noexcept;
```

textの接頭辞に当たる登録語のうち、最長のものの長さを返します。該当する登録語がなければnulloptです。

**制約:** 長さ付きバイト列を渡します。空語・NUL・全256バイトを扱います。

**計算量:** 語長LにO(16L+1)。


語長Lに対し通常の検索はO(16L)、登録は償却O(16L+256)、削除はO(16L+256)です。登録の再確保には保持ノード枠・索引枠のコピーも加わります。個数不足の削除はfalseで内容不変です。

## 辞書順

### count_less

```text
std::size_t count_less(std::string_view word) const noexcept;
```

word未満の語の総個数を重複込みで返します。

**制約:** 符号なしバイトの辞書順を使います。

**計算量:** O(256L)。

### kth

```text
std::optional<std::string> kth(std::size_t k) const;
```

重複込み・0始まりk番目の語を返し、範囲外ならnulloptです。

**制約:** 追加の制約はありません。

**計算量:** 返す語の長さKにO(256K+1)。

### lower_bound

```text
std::optional<std::string> lower_bound(std::string_view word) const;
```

word以上の最小登録語を返し、なければnulloptです。

**制約:** 符号なしバイトの辞書順を使います。

**計算量:** O(256L+K)、空wordではO(K+1)。

### items

```text
std::vector<std::pair<std::string, std::size_t>> items( std::string_view prefix = {}, std::size_t limit = std::numeric_limits<std::size_t>::max()) const;
```

異なる語と個数を辞書順に最大limit件返します。

**制約:** prefixは長さ付きバイト列。

**計算量:** 接頭辞探索＋訪問ノード数＋返却文字列総長。


Kは返す語の長さです。空のlower_boundはO(K)です。

## 管理・所有

### size

```text
std::size_t size() const noexcept;
```

重複込みの総個数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### distinct_size

```text
std::size_t distinct_size() const noexcept;
```

異なる語数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### empty

```text
bool empty() const noexcept;
```

空ならtrueです。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### node_count

```text
std::size_t node_count() const noexcept;
```

根を含む使用ノード数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### capacity

```text
std::size_t capacity() const noexcept;
```

確保済みノード枠数を返します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### clear

```text
void clear() noexcept;
```

全削除し、確保枠を保持します。

**制約:** 追加の制約はありません。

**計算量:** O(N+T)。

### reserve_nodes

```text
void reserve_nodes(std::size_t n);
```

少なくともnノード分の枠を予約します。

**制約:** ノード容量の上限以下。

**計算量:** 最悪O(N)。

### swap

```text
void swap(Trie& other) noexcept;
void swap(Trie& a, Trie& b) noexcept;
```

内容と確保枠を交換します。

**制約:** 追加の制約はありません。

**計算量:** O(1)。

### TrieNode::TrieNode

```text
TrieNode();
explicit TrieNode(std::unordered_map<char, std::shared_ptr<TrieNode>> values);
TrieNode(const TrieNode&);
TrieNode(TrieNode&&) noexcept;
```

既定／子map／子の所有権を共有するコピー／所有権移動で構築します。

**制約:** 別対象を所有するaliasingポインタ・循環所有は対象外。

**計算量:** 子mapのコピーに比例。既定とmoveはO(1)。

### TrieNode::operator=

```text
TrieNode& operator=(const TrieNode&);
TrieNode& operator=(TrieNode&&) noexcept;
```

子mapをコピー／移動して代入します。

**制約:** 別対象を所有するaliasingポインタ・循環所有は対象外。

**計算量:** 子mapのコピーや旧所有状態の解放に比例。


保持ノード枠数N・索引枠数Tとして、コピーはO(N+256T)、破棄・clearはO(N+T)、reserve_nodesは最悪O(N)、swapはO(1)です。保持領域はO(N+256T)。コピーは独立し、移動元は空で再利用できます。

登録・コピー代入の確保失敗では論理的内容を保持しますが、容量は増える場合があります。個数と容量の上限保護は`SKIP_RUNTIME_CHECKS`でも残ります。

別の公開型`TrieNode`は、既定構築または子mapから構築でき、`children`のコピーは子の所有権を共有します。別対象を所有するaliasingポインタ・循環所有は対象外です。深い木の破棄も反復処理です。

[完全な使用例](../Examples/trie_example.cpp) · [実装・検証の記録](../Audit/Changes/document-detail-archive-algorithm-string-2026-11-15.json)
